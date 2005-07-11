/*
 *  Lexical scanner for XML 1.0
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation. You find a copy of this
 *  license in the file COPYRIGHT in the root directory of this release.
 *
 *  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
 *  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
 *  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 *  Description:
 *
 *    This module implements a lexical scanner/parser for XML data.
 *    It works different for prolog and content: (prolog is meta-data
 *    such as the encoding specification or document type tag)
 *
 *    The prolog is processed by a yacc-generated parser.  In this phase
 *    the scanner holds only an approximate state (e.g. which kind of tag
 *    is currently parsed, or if we're inbetween quotes etc.) and breaks
 *    the input stream up into tokens which are returned to the parser.
 *
 *    The content however is parsed without external control.  The scanner
 *    only returns a single token to the yacc-parser (at the end-of-file or
 *    upon interruption by "processing instructions".)  The main reason
 *    for this distinction is that XML tags are easy to parse because
 *    there's little context, so a generated parser would provide little
 *    benefits.
 *
 *    The content is broken down into tag names, attribute names and
 *    values and content data inbetween tags.  This information is
 *    passed to xmltv_tags.c via a small set of callback functions.
 *    That module handles the semantic level of tags (e.g. checking if
 *    all opened tags are also closed <tag>...</tag>)
 *
 *
 *  Author: Tom Zoerner
 *
 *  $Id: xml_scan.lex,v 1.2 2005-07-11 14:56:06 laurentg Exp $
 */

%{
#define DEBUG_SWITCH DEBUG_SWITCH_XMLTV
#define DPRINTF_OFF

#include <string.h>
#include <stdlib.h>

#include "xmltv_types.h"
#include "xmltv_debug.h"

#include "xml_cdata.h"
#include "xml_hash.h"
#include "xml_prolog.tab.h"
#include "xmltv_db.h"
#include "xmltv_tags.h"


/* character encoding in the input stream: currently supported are the
** Latin family (ISO-8859-1..15) and UTF-8 which are passed through to
** the application as-is; also supported is UTF-16 (little or big endian)
** which is automatically converted to UTF-8 prior scanning */
static XML_ENCODING xml_scan_encoding;

/* function pointer used to transcode input character set to application
** requested character set; supported output encodings are UTF-8 and Latin-1;
** mapped to function which uses memcpy() if no transcoding is required */
static void (* XmlScan_CdataAppend)( XML_STR_BUF * pBuf, const char * pStr, uint len );

/* buffers used to assemble content and attribute values */
static XML_STR_BUF xml_attr;
static XML_STR_BUF xml_content;

/* stack used to parse entity replacement text
** note: fixed limit of nesting depth prevents direct or indirect
** self-reference (which is disallowed by the XML spec) */
#define MAX_ENTITY_REF_DEPTH 10
typedef struct
{
   YY_BUFFER_STATE      buffer;
   int                  prev_state;
   bool                 change_state;
} XML_ENTITY_STACK;
static XML_ENTITY_STACK xml_entity_stack[MAX_ENTITY_REF_DEPTH];
static int xml_entity_stack_depth;

/* hash array and temporary variables used to process entity definitions */
static XML_HASH_PTR pEntityHash;
static XML_STR_BUF xml_entity_new_name;

/* the payload of each hash array entry holds a pointer to a dup'ed string with the replacement text */
typedef struct
{
   char       * pValue;
} XML_ENTITY_HASH_VAL;

/* temporary flag which indicates the top-level tag was closed */
static int xml_scan_docend;
/* temporary cache for state while processing PI tag */
static int xml_scan_pre_pi_state;

/* forward declarations of internal operators
** (note: external operators are declard in xmltv_tag.h for lack of a scanner header file)
*/
static void XmlScan_PopEntityBuffer( void );
static int XmlScan_ParseEntity( const char * p_buffer, int scan_state, bool change_state );
static const char * XmlScan_QueryEntity( const char * pName );

/* ----------------------------------------------------------------------------
** Pass pre-processed content data to parser
** - note the content may be assembled from multiple chunks if there are CDATA
**   sections or parsed entities; this is transparent to the user-level
** - this function is called when beginning to scan a tag (either opening or
**   closing tag); hence it can be called more than once for elements with mixed
**   content data which is separated by child tags (i.e. the application has to
**   concatenate the chunks, if appropriate)
*/
#ifdef __GNUC__
__inline
#endif
static void XmlScan_ForwardContent( void )
{
   if (XML_STR_BUF_GET_LEN(xml_content) != 0)
   {
      XmltvTags_Data(&xml_content);
      XmlCdata_Reset(&xml_content);
   }
}

/* ----------------------------------------------------------------------------
** Add a character reference to a string buffer
** - note: no need to check for parse errors in the conversion, because the
**   scanner already made sure we only have decimal or hexadecimal digits;
** - the string is terminated by ';'
*/
static void XmlScan_AddCharRef( XML_STR_BUF * p_buf, char * p_text, int base )
{
   ulong lval;
   char cval[6];

   lval = strtol(p_text, NULL, base);
   if ( (lval < 32) &&
        ((lval != 9) && (lval != 10) && (lval != 13)) )
   {
      Xmltv_SyntaxError("Disallowed entity character reference", "&#0;");
   }
   else
   {
      if (xml_scan_encoding == XML_ENC_ISO8859)
      {
         if (lval <= 0xFF)
         {
            cval[0] = (char) lval;
            XmlScan_CdataAppend(p_buf, cval, 1);
         }
         else
            Xmltv_SyntaxError("Invalid code in entity character reference", p_text);
      }
      else
      {
         /* From the Unicode FAQ (http://www.cl.cam.ac.uk/~mgk25/unicode.html)
         ** U-00000000  U-0000007F:      0xxxxxxx
         ** U-00000080  U-000007FF:      110xxxxx 10xxxxxx
         ** U-00000800  U-0000FFFF:      1110xxxx 10xxxxxx 10xxxxxx
         ** U-00010000  U-001FFFFF:      11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
         ** U-00200000  U-03FFFFFF:      111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
         ** U-04000000  U-7FFFFFFF:      1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
         */
         if (lval <= 0x7F)
         {
            cval[0] = (char) lval;
            XmlScan_CdataAppend(p_buf, cval, 1);
         }
         else if (lval <= 0x7FF)
         {
            cval[0] = 0xC0 | (char) (lval >> 6);
            cval[1] = 0x80 | (char) (lval & 0x3F);
            XmlScan_CdataAppend(p_buf, cval, 2);
         }
         /* else if (lval <= 0xFFFF) */
         else if ((lval <= 0xD7FF) || ((lval >= 0xE000) && (lval <= 0xFFFD)))
         {
            cval[0] = 0xE0 | (char)  (lval >> (2*6));
            cval[1] = 0x80 | (char) ((lval >>    6)  & 0x3F);
            cval[2] = 0x80 | (char) ( lval           & 0x3F);
            XmlScan_CdataAppend(p_buf, cval, 3);
         }
         /* else if (lval <= 0x10FFFFL) */
         else if ((lval >= 0x10000) && (lval <= 0x10FFFFL))
         {
            cval[0] = 0xF0 | (char)  (lval >> (3*6));
            cval[1] = 0x80 | (char) ((lval >> (2*6)) & 0x3F);
            cval[2] = 0x80 | (char) ((lval >>    6)  & 0x3F);
            cval[3] = 0x80 | (char) ( lval           & 0x3F);
            XmlScan_CdataAppend(p_buf, cval, 4);
         }
         else
         {  /* 0x110000... are disallowed in XML */
            Xmltv_SyntaxError("Invalid code in entity character reference", p_text);
         }
      }
   }
}

/* ----------------------------------------------------------------------------
** Fill the scanner's input buffer while transcoding from UTF-16 to UTF-8
** - the XML standard requires to support UTF-16, however parsing UTF-16 would
**   require to write a completely new scanner; so this function is a compromise,
**   i.e. it allows to use the scanner in UTF-8 mode for UTF-16 input
** - the function works by reading the input word for word, decoding UTF-16,
**   re-encoding in UTF-8 and appending the result to the internal input buffer
*/
static size_t XmlScan_ReadUtf16IntoUtf8Buffer( uchar *ptr, size_t max_size )
{
   uint w1, w2, code;
   int  c1, c2;
   size_t result = 0;

   while (result + 4 <= max_size)
   {
      /* load first UTF-16 codeword */
      if ( ((c1 = getc(yyin)) == EOF) ||
           ((c2 = getc(yyin)) == EOF) )
      {
         if ( ferror(yyin) )
            Xmltv_ScanFatalError( "input in flex scanner failed" );
         break;
      }
      if (xml_scan_encoding == XML_ENC_UTF16BE)
         w1 = (c1 << 8) | c2;
      else
         w1 = (c2 << 8) | c1;

      if ((w1 < 0xD800) || (w1 > 0xDFFF))
      {  /* it's in the lower range, no decoding needed */
         code = w1;
      }
      else
      {
         if (w1 > 0xDBFF)
         {  /* 2nd word of a pair found in place of the 1st -> error */
            if ( ferror(yyin) )
               Xmltv_ScanFatalError( "UTF-16 input sequence error" );
            break;
         }
         if ( ((c1 = getc(yyin)) == EOF) ||
              ((c2 = getc(yyin)) == EOF) )
         {
            if ( ferror(yyin) )
               Xmltv_ScanFatalError( "input in flex scanner failed" );
            break;
         }
         /* decode UTF-16 to UCS-4 (acc. to RFC 2781 ch. 2.2) */
         if (xml_scan_encoding == XML_ENC_UTF16BE)
            w2 = (c1 << 8) | c2;
         else
            w2 = (c2 << 8) | c1;

         if ((w2 < 0xDC00) || (w2 > 0xDFFF))
         {  /* 1st word of a pair found in place of the 2nd -> error */
            if ( ferror(yyin) )
               Xmltv_ScanFatalError( "UTF-16 input sequence error" );
            break;
         }
         code = 0x100000 | ((w1 & 0x3FF) << 10) | (w2 & 0x3FF);
      }

      /* now encode in UTF-8 and copy to the input buffer */
      if (code <= 0x7F)
      {
         *(ptr++) = (char) code;
         result += 1;
      }
      else if (code <= 0x7FF)
      {
         *(ptr++) = 0xC0 | (char) (code >> 6);
         *(ptr++) = 0x80 | (char) (code & 0x3F);
         result += 2;
      }
      else if (code <= 0xFFFF)
      {
         *(ptr++) = 0xE0 | (char)  (code >> (2*6));
         *(ptr++) = 0x80 | (char) ((code >>    6)  & 0x3F);
         *(ptr++) = 0x80 | (char) ( code           & 0x3F);
         result += 3;
      }
      else /* if (code <= 0x1FFFFF) */
      {
         *(ptr++) = 0xF0 | (char)  (code >> (3*6));
         *(ptr++) = 0x80 | (char) ((code >> (2*6)) & 0x3F);
         *(ptr++) = 0x80 | (char) ((code >>    6)  & 0x3F);
         *(ptr++) = 0x80 | (char) ( code           & 0x3F);
         result += 4;
      }
   }
   return result;
}

/* Helper macro used in error states to skip all input until a given
** synchonization character is found (e.g. closing tag '>') and then
** resume scanning in a given state */
static int xml_scan_resync_start;
static int xml_scan_resync_char;
static int xml_scan_resync_state;
static const char * p_xml_scan_err_msg;
#define XML_SCAN_RESYNC(CHAR,STATE,MSG) \
  do { \
     xml_scan_resync_start = 1; \
     xml_scan_resync_char = (CHAR); \
     xml_scan_resync_state = (STATE); \
     p_xml_scan_err_msg = (MSG); \
     BEGIN resync; \
     yyless(0); \
  }while(0)

/* For debug purposes, report characters which don't match any rule in
** the current state; this should never happen, i.e. there should always
** be an error rule to catch unexpected input. Unmatched characters will
** simply be skipped and ignored by the scanner. */
#define ECHO do{debug2("XMLTV yylex unmatched text in state %d: #%s#", YYSTATE, yytext);}while(0)

/* Override the scanner's predefined error handler with one which doesn't call
** exit() nor fprintf(stderr) because neither is useful in a GUI application */
#define YY_FATAL_ERROR(MSG) do{Xmltv_ScanFatalError(MSG);}while(0)

/* Replace the scanner's predefined input function (or macro, actually)
** with one which supports trancoding UTF-16 to a supported input encoding */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ((xml_scan_encoding == XML_ENC_UTF16BE) || (xml_scan_encoding == XML_ENC_UTF16LE)) \
           result = XmlScan_ReadUtf16IntoUtf8Buffer(buf, max_size); \
	else if ( ((result = fread( buf, 1, max_size, yyin )) == 0) \
		  && ferror( yyin ) ) \
		YY_FATAL_ERROR( "input in flex scanner failed" );

#define YY_USE_CONST
#define YY_USE_PROTOS

int yylex( void );
%}

%option noyywrap
%option never-interactive

 /* Auto-detection of encoding; required to parse <?xml encoding=...?>, see XML 1.0 apx. F.1 */
 /* BOM stands for "byte order mark"; it's used on Windows only */
DETECT_UCS4_BOM         (\0\0(\xFE\xFF)|(\xFF\xFE))|((\xFF\xFE)|(\xFE\xFF)\0\0)
DETECT_UTF16BE_BOM      (\xFE\xFF)([^\0][\0-\xff]|[\0-\xff][^\0])
DETECT_UTF16LE_BOM      (\xFF\xFE)([^\0][\0-\xff]|[\0-\xff][^\0])
DETECT_UTF8_BOM         \xEF\xBB\xBF
DETECT_UCS4             (\0\0\0\x3C)|(\x3C\0\0\0)|(\0\0\x3C\0)|(\0\x3C\0\0)
DETECT_UTF16BE          (\0\x3C\0\x3F)
DETECT_UTF16LE          (\x3C\0\x3F\0)
DETECT_UTF8_ASCII       "<?xml"
DETECT_EBCDIC           \x4C\x6F\xA7\x94

 /* Note about UTF-8 and ISO-8859-x character classes: in the scanner we only match on
 ** characters in the ASCII range, whose encoding is identical in either coding scheme.
 ** ISO characters in range 0x80-0xff and multi-byte UTF-8 characters are never used
 ** for markup or syntax separators, so we can safely assume them to be part of the
 ** current name or content element. This saves a lot of effort, esp. for UTF-8.
 */
NON_ASCII       [\x80-\xff]

XMLWS           [ \t\n\r]
XMLSPACE        {XMLWS}+
XMLLETTER       ([a-zA-Z]|{NON_ASCII})
XMLDIGIT        [0-9]
XMLNAMECHAR     ({XMLLETTER}|{XMLDIGIT}|[\.\-_:])
XMLNAME         ({XMLLETTER}|[_:]){XMLNAMECHAR}*
XMLNMTOKEN      {XMLNAMECHAR}+
COMMENT         "<!--"
CDATA           "<![CDATA["
CDEND           "]]>"
ENCNAME         [a-zA-Z][a-zA-Z0-9.\-_]*
ENTPUB1         \"[ \n\ra-zA-Z0-9\-'()+,./:=?;!*#@$_%]*\"
ENTPUB2         '[ \n\ra-zA-Z0-9\-()+,./:=?;!*#@$_%]*'
ENTSYS1         \"[^"]*\"
ENTSYS2         '[^']*'

 /* prolog states */
%s prolog
%s prologerr1
%s prologerr2
%s xmlenc
%s xmlencenc
%s xmlencsdecl
%s xmlencv
%s xmlencerr
%s piname
%s pispace
%s pi0
%s pi1
%s piclose
%s pierr
%s doctype
%s doctypedecl
%s doctypesys
%s doctypepub
%s docdtd
%s docdtderr
%s entdef
%s entval
%s entval1
%s entval2
%s entval3
%s entpub
%s entsys
%s entndata
%s dtdelem
%s dtdelspec
%s dtdeldef
%s dtdatt
%s dtdattnam
%s dtdattval1
%s dtdattval2
%s dtdattnot
%s dtdattenu
%s dtdatttyp
%s docnotnam
%s docnottyp
%s docnotsys
%s docnotpub

 /* content states */
%s content
%s tagname
%s tagclosename
%s tagcloseend
%s tagattrname
%s tagattrspace
%s tagattrass
%s tagattrval
%s tagattrval1
%s tagattrval2
%s tagattrval3
%s tagattrval3err
%s tagerror
%s cdata0
%s cdata1
%s cdata2
%s comment0
%s comment1
%s comment2
%s stopped
%s stopped_end
%s resync

%%

 /* ------------  P  R  O  L  O  G  ------------ */

 /* Auto-detect character encoding <?xml...?> */

<INITIAL>{DETECT_UCS4_BOM}|{DETECT_UCS4}        { BEGIN stopped_end; XmltvTags_ScanUnsupEncoding("UCS-4"); return XMLTOK_CONTENT_END; }
<INITIAL>{DETECT_EBCDIC}                        { BEGIN stopped_end; XmltvTags_ScanUnsupEncoding("EBCDIC"); return XMLTOK_CONTENT_END; }
<INITIAL>{DETECT_UTF16BE_BOM}                   { BEGIN prolog; yyless(0);
                                                  xml_scan_encoding = XML_ENC_UTF16BE;
                                                  YY_FLUSH_BUFFER;
                                                  fseek(yyin, 2, SEEK_SET);
                                                }
<INITIAL>{DETECT_UTF16LE_BOM}                   { BEGIN prolog; yyless(0);
                                                  xml_scan_encoding = XML_ENC_UTF16LE;
                                                  YY_FLUSH_BUFFER;
                                                  fseek(yyin, 2, SEEK_SET);
                                                }
<INITIAL>{DETECT_UTF16BE}                       { BEGIN prolog; yyless(0);
                                                  xml_scan_encoding = XML_ENC_UTF16BE;
                                                  YY_FLUSH_BUFFER;
                                                  fseek(yyin, 0, SEEK_SET);
                                                }
<INITIAL>{DETECT_UTF16LE}                       { BEGIN prolog; yyless(0);
                                                  xml_scan_encoding = XML_ENC_UTF16LE;
                                                  YY_FLUSH_BUFFER;
                                                  fseek(yyin, 0, SEEK_SET);
                                                }
<INITIAL>{DETECT_UTF8_BOM}                      { BEGIN prolog; xml_scan_encoding = XML_ENC_UTF8; }
<INITIAL>{DETECT_UTF8_ASCII}                    { BEGIN prolog; yyless(0); xml_scan_encoding = XML_ENC_UTF8; }
<INITIAL>[\0-\xff]                              { BEGIN prolog; yyless(0); xml_scan_encoding = XML_ENC_UTF8; }

<prolog>{XMLSPACE}                              { return XMLTOK_SPACE; }

 /* Scan encoding <?xml...?> */

<prolog>"<?xml"                                 {  BEGIN xmlenc; return XMLTOK_TAGENC; }
<xmlenc>"version"                               {  BEGIN xmlencv; return XMLTOK_VERSION; }
<xmlencv>"="                                    {  return XMLTOK_EQ; }
<xmlencv>\'[0-9.]+\'|\"[0-9.]+\"                {  BEGIN xmlenc;
                                                   yytext[yyleng - 1] = 0;
                                                   yylval.str = yytext + 1;
                                                   return XMLTOK_ENCVERS; }
<xmlenc>"standalone"                            {  BEGIN xmlencsdecl; return XMLTOK_STANDALONE; }
<xmlencsdecl>"="                                {  return XMLTOK_EQ; }
<xmlencsdecl>"yes"|'yes'                        {  BEGIN xmlenc; return XMLTOK_YES; }
<xmlencsdecl>"no"|'no'                          {  BEGIN xmlenc; return XMLTOK_NO; }
<xmlenc>"encoding"                              {  BEGIN xmlencenc; return XMLTOK_ENCODING; }
<xmlencenc>"="                                  {  return XMLTOK_EQ; }
<xmlencenc>\"{ENCNAME}\"|\'{ENCNAME}\'          {  BEGIN xmlenc;
                                                   yytext[yyleng - 1] = 0;
                                                   yylval.str = yytext + 1;
                                                   return XMLTOK_ENCNAME;
                                                }
<xmlenc,xmlencv,xmlencsdecl,xmlencenc>{XMLSPACE} { return XMLTOK_SPACE; }
<xmlenc>"?>"                                    {  BEGIN prolog; return XMLTOK_CLOSE; }
<xmlenc,xmlencv,xmlencsdecl,xmlencenc>.         {  XML_SCAN_RESYNC('>', xmlencerr, "Malformed encoding declaration"); }
<xmlencerr>">"                                  {  BEGIN prolog;
                                                   return XMLTOK_ERR_CLOSE;
                                                }

 /* Scan "processing instructions" (e.g. <?foo-bar...?>) */

<prolog,content>"<?"                            {  xml_scan_pre_pi_state = YYSTATE;
                                                   XmlCdata_Reset(&xml_attr);
                                                   BEGIN piname;
                                                   return XMLTOK_TAGPI;
                                                }
<piname>{XMLNAME}                               {  BEGIN pispace;
                                                   yylval.str = yytext;
                                                   return XMLTOK_NAME;
                                                }
<pispace>"?>"                                   {  BEGIN xml_scan_pre_pi_state; return XMLTOK_CLOSE; }
<pispace>{XMLSPACE}                             {  BEGIN pi0; return XMLTOK_SPACE; }
<pi0>"?"                                        {  BEGIN pi1; }
<pi0>\r+                                        {  /*dropped*/ }
<pi0>[^\?\r]+                                   {  XmlScan_CdataAppend(&xml_attr, yytext, yyleng); }
<pi1>">"                                        {  BEGIN piclose; yyless(0);
                                                   yylval.str = XML_STR_BUF_GET_STR(xml_attr);
                                                   return XMLTOK_PICONTENT;
                                                }
<pi1>"?"                                        {  XmlCdata_AppendRaw(&xml_attr, "?", 1); }
<pi1>\r+                                        {  BEGIN pi0; /* dropped, but QM in stack is appended */
                                                   XmlCdata_AppendRaw(&xml_attr, "?", 1);
                                                }
<pi1>[^\?\>\r]                                  {  BEGIN pi0;
                                                   XmlCdata_AppendRaw(&xml_attr, "?", 1);
                                                   XmlScan_CdataAppend(&xml_attr, yytext, yyleng);
                                                }
<piclose>">"                                    {  BEGIN xml_scan_pre_pi_state; return XMLTOK_CLOSE; }
<piname,pispace>.                               {  XML_SCAN_RESYNC('>', pierr, "Parse error in processing instruction"); }
<pierr>">"                                      {  BEGIN xml_scan_pre_pi_state;
                                                   return XMLTOK_ERR_CLOSE;
                                                }

 /* Parse DOCTYPE header */

<prolog>"<!DOCTYPE"                             {  BEGIN doctype; return XMLTOK_TAGDOC; }
<doctype>{XMLNAME}                              {  BEGIN doctypedecl;
                                                   yylval.str = yytext;
                                                   return XMLTOK_NAME;
                                                }
<doctypedecl>"SYSTEM"                           {  BEGIN doctypesys; return XMLTOK_STR_SYSTEM; }
<doctypedecl>"PUBLIC"                           {  BEGIN doctypepub; return XMLTOK_STR_PUBLIC; }
<doctypesys>{ENTSYS1}|{ENTSYS2}                 {  BEGIN doctypedecl;
                                                   yytext[yyleng - 1] = 0; /* strip quotes */
                                                   yylval.str = yytext + 1;
                                                   return XMLTOK_SYSLIT;
                                                }
<doctypepub>{ENTPUB1}|{ENTPUB2}                 {  BEGIN doctypesys;
                                                   yytext[yyleng - 1] = 0;
                                                   yylval.str = yytext + 1;
                                                   return XMLTOK_PUBLIT;
                                                }
<doctypedecl>"["                                {  BEGIN docdtd; return XMLTOK_DTD_OPEN; }
<docdtd>"]"                                     {  BEGIN doctypedecl; return XMLTOK_DTD_CLOSE; }
<doctype,doctypedecl>">"                        {  BEGIN prolog; return XMLTOK_CLOSE; }
<doctype,doctypedecl,doctypesys,doctypepub,docdtd>{XMLSPACE}  { return XMLTOK_SPACE; }
<doctype,doctypedecl,doctypesys,doctypepub>.    {  XML_SCAN_RESYNC('>', prologerr2, "Parse error in DOCTYPE declaration"); }

 /* Scan entitity definitions (inside DTD only) */

<docdtd>"<!ENTITY"                              {  BEGIN entdef; return XMLTOK_TAGENT; }
<entdef>"%"                                     {  return XMLTOK_CHR_PERCENT; }
<entdef,entndata>{XMLNAME}                      {  BEGIN entval;
                                                   yylval.str = yytext;
                                                   return XMLTOK_NAME;
                                                }
<entval>\"                                      {  BEGIN entval1; XmlCdata_Reset(&xml_attr); }
<entval>\'                                      {  BEGIN entval2; XmlCdata_Reset(&xml_attr); }
<entval1,entval2,entval3>&#x[0-9a-fA-F]+;       {  XmlScan_AddCharRef(&xml_attr, yytext + 3, 16); }
<entval1,entval2,entval3>&#[0-9]+;              {  XmlScan_AddCharRef(&xml_attr, yytext + 2, 10); }
<entval1,entval2,entval3>&{XMLNAME};            {  XmlScan_CdataAppend(&xml_attr, yytext, yyleng); /* general entity references are bypassed */ }
<entval1,entval2,entval3>"%"{XMLNAME}";"        {
                                                   const char * pValue;
                                                   yytext[yyleng - 1] = 0; /* remove trailing ';' */
                                                   pValue = XmlScan_QueryEntity(yytext);
                                                   if (pValue != NULL)
                                                   {
                                                      /* recursively parse the replacement text */
                                                      /* enter new state because string delimiters must be ignored inside replacement text (XML 1.0 ch. 4.4.5) */
                                                      if (YYSTATE != entval3)
                                                      {
                                                         XmlScan_ParseEntity(pValue, entval3, 1);
                                                      }
                                                      else
                                                         XmlScan_ParseEntity(pValue, 0, 0);
                                                   }
                                                }
<entval1,entval2,entval3>[&%]                   {  XML_SCAN_RESYNC('>', docdtderr, "Malformed entity reference in entity declaration"); }
<entval1,entval2>\r+                            {  /*dropped*/ }
<entval1>[^\"\&\%\r]+                           {  XmlScan_CdataAppend(&xml_attr, yytext, yyleng); }
<entval2>[^\'\&\%\r]+                           {  XmlScan_CdataAppend(&xml_attr, yytext, yyleng); }
<entval3>[^\&\%]+                               {  XmlScan_CdataAppend(&xml_attr, yytext, yyleng); }
<entval1>\"                                     {  BEGIN entval;
                                                   yylval.str = XML_STR_BUF_GET_STR(xml_attr);
                                                   return XMLTOK_ENTVAL;
                                                }
<entval2>\'                                     {  BEGIN entval;
                                                   yylval.str = XML_STR_BUF_GET_STR(xml_attr);
                                                   return XMLTOK_ENTVAL;
                                                }
<entval>"SYSTEM"                                {  BEGIN entsys; return XMLTOK_STR_SYSTEM; }
<entval>"PUBLIC"                                {  BEGIN entpub; return XMLTOK_STR_PUBLIC; }
<entsys>{ENTSYS1}|{ENTSYS2}                     {  BEGIN entval;
                                                   yytext[yyleng - 1] = 0; /* strip quotes */
                                                   yylval.str = yytext + 1;
                                                   return XMLTOK_SYSLIT;
                                                }
<entpub>{ENTPUB1}|{ENTPUB2}                     {  BEGIN entsys;
                                                   yytext[yyleng - 1] = 0;
                                                   yylval.str = yytext + 1;
                                                   return XMLTOK_PUBLIT;
                                                }
<entval>"NDATA"                                 {  BEGIN entndata; return XMLTOK_NDATA; }
<entdef,entval,entsys,entpub,entndata>{XMLSPACE} {  return XMLTOK_SPACE; }
<entdef,entval,entsys,entpub,entndata>">"       {  BEGIN docdtd; return XMLTOK_CLOSE; }
<entdef,entval,entsys,entpub,entndata>.         {  XML_SCAN_RESYNC('>', docdtderr, "Malformed entity declaration in DTD"); }

 /* Parameter Entity Reference ("PEReference"; inside DTD only) */

<docdtd>"%"{XMLNAME}";"                         {
                                                   const char * pValue;
                                                   yytext[yyleng - 1] = 0; /* remove trailing ';' */
                                                   pValue = XmlScan_QueryEntity(yytext);
                                                   if (pValue != NULL)
                                                   {
                                                      /* recursively parse the replacement text */
                                                      /* insert space before and after (XML 1.0 ch. 4.4.8) */
                                                      XmlScan_ParseEntity(" ", 0, 0);
                                                      XmlScan_ParseEntity(pValue, 0, 0);
                                                      XmlScan_ParseEntity(" ", 0, 0);
                                                   }
                                                }
<docdtd>"%".                                    {  XML_SCAN_RESYNC('>', docdtderr, "Malformed parameter entity reference"); }
<docdtd>"&"                                     {  XML_SCAN_RESYNC('>', docdtderr, "Disallowed entity reference in DTD"); }

 /* DTD element declaration */

<docdtd>"<!ELEMENT"                             {  BEGIN dtdelem; return XMLTOK_TAGELEM; }
<dtdelem>{XMLNAME}                              {  BEGIN dtdelspec;
                                                   yylval.str = yytext;
                                                   return XMLTOK_NAME;
                                                }
<dtdelspec>"EMPTY"                              {  return XMLTOK_STR_EMPTY; }
<dtdelspec>"ANY"                                {  return XMLTOK_STR_ANY; }
<dtdelspec>"("                                  {  BEGIN dtdeldef; return XMLTOK_CHR_OPBR; }
<dtdeldef>"#PCDATA"                             {  return XMLTOK_STR_PCDATA; }
<dtdeldef>"("                                   {  return XMLTOK_CHR_OPBR; }
<dtdeldef>")"                                   {  return XMLTOK_CHR_CLBR; }
<dtdeldef>"*"                                   {  return XMLTOK_CHR_ASTX; }
<dtdeldef>"+"                                   {  return XMLTOK_CHR_PLUS; }
<dtdeldef>"?"                                   {  return XMLTOK_CHR_QUMK; }
<dtdeldef>"|"                                   {  return XMLTOK_CHR_PIPE; }
<dtdeldef>","                                   {  return XMLTOK_CHR_COMA; }
<dtdeldef>{XMLNAME}                             {  yylval.str = yytext;
                                                   return XMLTOK_NAME;
                                                }
<dtdelem,dtdelspec,dtdeldef>{XMLSPACE}          {  return XMLTOK_SPACE; }
<dtdelem,dtdelspec,dtdeldef>">"                 {  BEGIN docdtd; return XMLTOK_CLOSE; }
<dtdelem,dtdelspec,dtdeldef>.                   {  XML_SCAN_RESYNC('>', docdtderr, "Malformed element declaration in DTD"); }

 /* DTD attribute declaration */

<docdtd>"<!ATTLIST"                             {  BEGIN dtdatt; return XMLTOK_TAGATT; }
<dtdatt>{XMLNAME}                               {  BEGIN dtdattnam; yylval.str = yytext; return XMLTOK_NAME; }
<dtdattnam>{XMLNAME}                            {  BEGIN dtdatttyp; yylval.str = yytext; return XMLTOK_NAME; }
<dtdatttyp>"CDATA"                              {  BEGIN dtdattnam; return XMLTOK_STR_CDATA; }
<dtdatttyp>"IDREFS"                             {  BEGIN dtdattnam; return XMLTOK_STR_IDREFS; }
<dtdatttyp>"IDREF"                              {  BEGIN dtdattnam; return XMLTOK_STR_IDREF; }
<dtdatttyp>"ID"                                 {  BEGIN dtdattnam; return XMLTOK_STR_ID; }
<dtdatttyp>"ENTITY"                             {  BEGIN dtdattnam; return XMLTOK_STR_ENTITY; }
<dtdatttyp>"ENTITIES"                           {  BEGIN dtdattnam; return XMLTOK_STR_ENTITIES; }
<dtdatttyp>"NMTOKENS"                           {  BEGIN dtdattnam; return XMLTOK_STR_NMTOKENS; }
<dtdatttyp>"NMTOKEN"                            {  BEGIN dtdattnam; return XMLTOK_STR_NMTOKEN; }
<dtdatttyp>"NOTATION"                           {  BEGIN dtdattnot; return XMLTOK_STR_NOTATION; }
<dtdatttyp>"("                                  {  BEGIN dtdattenu; return XMLTOK_CHR_OPBR; }
<dtdattnot>"("                                  {  return XMLTOK_CHR_OPBR; }
<dtdattnot,dtdattenu>"|"                        {  return XMLTOK_CHR_PIPE; }
<dtdattnot,dtdattenu>")"                        {  BEGIN dtdattnam; return XMLTOK_CHR_CLBR; }
<dtdattnot>{XMLNAME}                            {  yylval.str = yytext; return XMLTOK_NAME; }
<dtdattenu>{XMLNMTOKEN}                         {  yylval.str = yytext; return XMLTOK_NMTOKEN; }
<dtdattnam>"#REQUIRED"                          {  return XMLTOK_STR_REQUIRED; }
<dtdattnam>"#IMPLIED"                           {  return XMLTOK_STR_IMPLIED; }
<dtdattnam>"#FIXED"                             {  return XMLTOK_STR_FIXED; }
<dtdattnam>\"                                   {  BEGIN dtdattval1; /* note: identical processing as in tagattrval1,2 */}
<dtdattnam>\'                                   {  BEGIN dtdattval2; }
<dtdattval1>\"                                  {  BEGIN dtdattnam;
                                                   yylval.str = XML_STR_BUF_GET_STR(xml_attr);
                                                   return XMLTOK_ATTVAL;
                                                }
<dtdattval2>\'                                  {  BEGIN dtdattnam;
                                                   yylval.str = XML_STR_BUF_GET_STR(xml_attr);
                                                   return XMLTOK_ATTVAL;
                                                }
<dtdatt,dtdattnam,dtdatttyp,dtdattnot,dtdattenu>{XMLSPACE} {  return XMLTOK_SPACE; }
<dtdatt,dtdattnam,dtdatttyp,dtdattnot,dtdattenu>">" {  BEGIN docdtd; return XMLTOK_CLOSE; }
<dtdatt,dtdattnam,dtdatttyp,dtdattnot,dtdattenu>. {  XML_SCAN_RESYNC('>', docdtderr, "Malformed ATTLIST declaration in DTD"); }

 /* DTD notation declaration */

<docdtd>"<!NOTATION"                            {  BEGIN docnotnam; return XMLTOK_TAGNOT; }
<docnotnam>{XMLNAME}                            {  BEGIN docnottyp; yylval.str = yytext; return XMLTOK_NAME; }
<docnottyp>"SYSTEM"                             {  BEGIN docnotsys; return XMLTOK_STR_SYSTEM; }
<docnottyp>"PUBLIC"                             {  BEGIN docnotpub; return XMLTOK_STR_PUBLIC; }
<docnotsys>{ENTSYS1}|{ENTSYS2}                  {  yytext[yyleng - 1] = 0; /* strip quotes */
                                                   yylval.str = yytext + 1;
                                                   return XMLTOK_SYSLIT;
                                                }
<docnotpub>{ENTPUB1}|{ENTPUB2}                  {  BEGIN docnotsys;
                                                   yytext[yyleng - 1] = 0;
                                                   yylval.str = yytext + 1;
                                                   return XMLTOK_PUBLIT;
                                                }
<docnotnam,docnottyp,docnotsys,docnotpub>{XMLSPACE} {  return XMLTOK_SPACE; }
<docnotnam,docnottyp,docnotsys,docnotpub>">"    {  BEGIN docdtd; return XMLTOK_CLOSE; }
<docnotnam,docnottyp,docnotsys,docnotpub>.      {  XML_SCAN_RESYNC('>', docdtderr, "Malformed notation declaration in DTD"); }

 /* DTD error rules */

<docdtd>[\0-\xff]                               {  XML_SCAN_RESYNC('>', docdtderr, "Parse error: unknown markup in DTD"); }
<docdtderr>">"                                  {  BEGIN docdtd;
                                                   return XMLTOK_ERR_CLOSE;
                                                }

 /* Transition from prolog into content area */

<prolog>\<{XMLNAME}                             {  BEGIN content; yyless(0); }

<prolog>\<                                      {  XML_SCAN_RESYNC('>', prologerr2, "Invalid markup in XML prolog"); }
<prolog>[\0-\xff]                               {  if (yytext[0] == 0)
                                                      XML_SCAN_RESYNC('<', prologerr1, "Binary data in XML prolog");
                                                   else
                                                      XML_SCAN_RESYNC('<', prologerr1, "Garbage in XML prolog");
                                                }
<prologerr1>"<"                                 {  BEGIN prolog;
                                                   /* preserve '<' which belongs to the subsequent element */
                                                   yyless(0);
                                                   /* return XMLTOK_ERR; - simply skipped, error not indicated to yacc */
                                                }
<prologerr2>">"                                 {  BEGIN prolog;
                                                   return XMLTOK_ERR_CLOSE;
                                                }


 /* ------------  C  O  N  T  E  N  T  ------------ */

 /* Parse closing tags */

<content>\<\/{XMLNAME}                          {  BEGIN tagcloseend;
                                                   XmlScan_ForwardContent();
                                                   xml_scan_docend = XmltvTags_Close(yytext + 2);
                                                }

<content>"</>"                                  {  BEGIN content;
                                                   Xmltv_SyntaxError("Parse error: empty close tag", "</>");
                                                }
<content>"</".                                  {  XML_SCAN_RESYNC('>', tagerror, "Parse error in closing tag: not a valid tag name"); }

<tagcloseend>{XMLWS}*">"                        {
                                                   if (xml_scan_docend == 0)
                                                   {
                                                      BEGIN prolog;
                                                      return XMLTOK_CONTENT_END;
                                                   }
                                                   else
                                                      BEGIN content;
                                                }

<tagcloseend>.                                  {  XML_SCAN_RESYNC('>', tagerror, "Garbage in closing tag after tag name"); }

 /* Parse opening or empty tag (e.g. <tag attr="1"> or <tag /> */

<content>\<                                     {  BEGIN tagname;
                                                   XmlScan_ForwardContent();
                                                }

<tagname>{XMLNAME}                              {
                                                   BEGIN tagattrspace;
                                                   XmltvTags_Open(yytext);
                                                }

<tagname>\>                                     {  BEGIN content;
                                                   Xmltv_SyntaxError("Parse error: empty tag", "<>");
                                                }
<tagname>.                                      {  XML_SCAN_RESYNC('>', tagerror, "Parse error in opening tag: not a valid tag name"); }

<tagattrspace>{XMLWS}+                          {  BEGIN tagattrname; }

<tagattrname>{XMLNAME}                          {  BEGIN tagattrass;
                                                   XmltvTags_AttribIdentify(yytext);
                                                }

<tagattrass>{XMLWS}*={XMLWS}*                   {  BEGIN tagattrval;
                                                   XmlCdata_Reset(&xml_attr);
                                                }
<tagattrass>.                                   {  XML_SCAN_RESYNC('>', tagerror, "Parse error in tag attribute assignment: expected equal sign"); }

<tagattrval>\"                                  {  BEGIN tagattrval1; }
<tagattrval>\'                                  {  BEGIN tagattrval2; }
<tagattrval>.                                   {  XML_SCAN_RESYNC('>', tagerror, "Parse error in tag attribute assignment: expected opening quote"); }

<tagattrval1,tagattrval2,dtdattval1,dtdattval2,tagattrval3>{
   &amp;                                        {  XmlCdata_AppendRaw(&xml_attr, "&", 1); }
   &lt;                                         {  XmlCdata_AppendRaw(&xml_attr, "<", 1); }
   &gt;                                         {  XmlCdata_AppendRaw(&xml_attr, ">", 1); }
   &apos;                                       {  XmlCdata_AppendRaw(&xml_attr, "'", 1); }
   &quot;                                       {  XmlCdata_AppendRaw(&xml_attr, "\"", 1); }
   &#x[0-9a-fA-F]+;                             {  XmlScan_AddCharRef(&xml_attr, yytext + 3, 16); }
   &#[0-9]+;                                    {  XmlScan_AddCharRef(&xml_attr, yytext + 2, 10); }
   &{XMLNAME};                                  {
                                                   const char * pValue;
                                                   yytext[yyleng - 1] = 0; /* remove trailing ';' */
                                                   pValue = XmlScan_QueryEntity(yytext);
                                                   if (pValue == NULL)
                                                   {
                                                      XmlScan_CdataAppend(&xml_attr, yytext, yyleng - 1);
                                                      XmlCdata_AppendRaw(&xml_attr, ";", 1);
                                                   }
                                                   else
                                                   {
                                                      /* enter new state because string delimiters must be ignored inside replacement text (XML 1.0 ch. 4.4.5) */
                                                      if (YYSTATE != tagattrval3)
                                                      {
                                                         XmlScan_ParseEntity(pValue, tagattrval3, 1);
                                                      }
                                                      else
                                                         XmlScan_ParseEntity(pValue, 0, 0);
                                                   }
                                                }
   [\n\t]                                       {  XmlCdata_AppendRaw(&xml_attr, " ", 1); }
}
<tagattrval1,tagattrval2,dtdattval1,dtdattval2>\r+  {  /*dropped*/ }
<tagattrval3>\r                                 {  XmlCdata_AppendRaw(&xml_attr, " ", 1); }
<tagattrval1,dtdattval1>[^\"\<\&\n\r\t]+        {  XmlScan_CdataAppend(&xml_attr, yytext, yyleng); }
<tagattrval2,dtdattval2>[^\'\<\&\n\r\t]+        {  XmlScan_CdataAppend(&xml_attr, yytext, yyleng); }
<tagattrval3>[^\<\&\n\r\t]+                     {  XmlScan_CdataAppend(&xml_attr, yytext, yyleng); }

<tagattrval1>\"                                 {  BEGIN tagattrspace;
                                                   XmltvTags_AttribData(&xml_attr);
                                                }
<tagattrval2>\'                                 {  BEGIN tagattrspace;
                                                   XmltvTags_AttribData(&xml_attr);
                                                }

<tagattrval1,tagattrval2>.                      {  XML_SCAN_RESYNC('>', tagerror, "Invalid character in attribute value"); }
<tagattrval3>.                                  {  BEGIN tagattrval3err; yyless(0); }
<tagattrval3err>[\0-\xff]+                      {  /* discard remaining entity replacement text */
                                                   Xmltv_SyntaxError("Invalid character in attribute value", yytext);
                                                }
<dtdattval1,dtdattval2>.                        {  XML_SCAN_RESYNC('>', docdtderr, "Invalid character in default attribute value in DTD ATTLIST declaration"); }
<tagattrname,tagattrspace>"/>"                  {  BEGIN content;
                                                   if (XmltvTags_Close(NULL) == FALSE)
                                                   {
                                                      BEGIN prolog;
                                                      return XMLTOK_CONTENT_END;
                                                   }
                                                }
<tagattrname,tagattrspace>\/[^>]                {  XML_SCAN_RESYNC('>', tagerror, "Parse error at tag end: garbage after /"); }

<tagattrname,tagattrspace>">"                   {  BEGIN content;
                                                   XmltvTags_AttribsComplete();
                                                }

<tagattrspace>.                                 {  XML_SCAN_RESYNC('>', tagerror, "Parse error in tag: garbage after tag name or assignment"); }
<tagattrname>.                                  {  XML_SCAN_RESYNC('>', tagerror, "Parse error in tag: expected assignment or closing tag"); }

<tagerror>">"                                   {  BEGIN content; }


 /* CDATA inside content area (unparsed character data) */

<content>{CDATA}                                {  BEGIN cdata0; }

<cdata0>"]"                                     {  BEGIN cdata1; }
<cdata0>\r                                      {  /*dropped*/ }
<cdata0>[^\]\r]+                                {  XmlScan_CdataAppend(&xml_content, yytext, yyleng); }
<cdata1>"]"                                     {  BEGIN cdata2; }
<cdata1>[^\]]                                   {  BEGIN cdata0;
                                                   XmlCdata_AppendRaw(&xml_content, "]", 1);
                                                   if (yytext[0] != '\r')
                                                      XmlScan_CdataAppend(&xml_content, yytext, 1);
                                                }
<cdata2>">"                                     {  BEGIN content; }
<cdata2>"]"                                     {  XmlScan_CdataAppend(&xml_content, "]", 1); }
<cdata2>[^>\]]                                  {  BEGIN cdata0;
                                                   XmlCdata_AppendRaw(&xml_content, "]]", 2);
                                                   if (yytext[0] != '\r')
                                                      XmlScan_CdataAppend(&xml_content, yytext, 1);
                                                }

 /* Parse content with entity references */

<content>&amp;                                  {  XmlCdata_AppendRaw(&xml_content, "&", 1); }
<content>&lt;                                   {  XmlCdata_AppendRaw(&xml_content, "<", 1); }
<content>&gt;                                   {  XmlCdata_AppendRaw(&xml_content, ">", 1); }
<content>&apos;                                 {  XmlCdata_AppendRaw(&xml_content, "'", 1); }
<content>&quot;                                 {  XmlCdata_AppendRaw(&xml_content, "\"", 1); }
<content>&#x[0-9a-fA-F]+;                       {  XmlScan_AddCharRef(&xml_content, yytext + 3, 16); }
<content>&#[0-9]+;                              {  XmlScan_AddCharRef(&xml_content, yytext + 2, 10); }
<content>&{XMLNAME};                            {
                                                   const char * pValue;
                                                   yytext[yyleng - 1] = 0; /* remove trailing ';' */
                                                   pValue = XmlScan_QueryEntity(yytext);
                                                   if (pValue == NULL)
                                                   {
                                                      XmlScan_CdataAppend(&xml_content, yytext, yyleng - 1);
                                                      XmlCdata_AppendRaw(&xml_content, ";", 1);
                                                   }
                                                   else
                                                   {
                                                      XmlScan_ParseEntity(pValue, 0, 0);
                                                   }
                                                }
<content>&                                      {
                                                   XmlCdata_AppendRaw(&xml_content, "&", 1);
                                                   Xmltv_SyntaxError("Parse error in entity reference:", yytext);
                                                }
<content>\r+                                    {  if (xml_entity_stack_depth > 0)
                                                   {  /* not dropped when inserted as "&#13;" in entity replacement text (testcase valid/sa/068.xml) */
                                                      XmlCdata_AppendRaw(&xml_content, yytext, yyleng);
                                                   }
                                                }
<content>[^<&\r]+                               {
                                                   XmlScan_CdataAppend(&xml_content, yytext, yyleng);
                                                }

 /* Skip comments <!-- ... --> (both during prolog and content) */
 /* note: catch-all error rules for these state are already above,
 ** but this works anyways because the rule generates a longer match
 ** (3 chars instead of 1 for error rules) */

<prolog,docdtd,content>{COMMENT}                {  xml_scan_pre_pi_state = YYSTATE;
                                                   BEGIN comment0;
                                                }
<comment0>"-"                                   {  BEGIN comment1; }
<comment0>[^\-]                                 {  /* use C code for skipping to avoid matching possibly huge chunks of text */
                                                   int skipped;
                                                   int c;
                                                   for (skipped = 0; skipped < 4000; skipped++)
                                                   {
                                                      c = input();
                                                      if (c == EOF)
                                                      {
                                                         BEGIN xml_scan_pre_pi_state;
                                                         if (xml_entity_stack_depth > 0)
                                                            XmlScan_PopEntityBuffer();
                                                         else
                                                            yyterminate();
                                                         Xmltv_SyntaxError("unterminated comment", "");
                                                      }
                                                      else if (c == '-')
                                                      {
                                                         BEGIN comment1;
                                                         break;
                                                      }
                                                      /* else: continue to skip input */
                                                   }
                                                }
<comment1>"-"                                   {  BEGIN comment2; }
<comment1>[^\-]                                 {  BEGIN comment0; }
<comment2>"-"                                   {  /* keep waiting for '>' */ }
<comment2>">"                                   {  BEGIN xml_scan_pre_pi_state;
                                                   /* no return, continue scanning */
                                                }
<comment2>[^>\-]                                {  BEGIN comment0; }


 /* Skip input until a possible synchronization character is found */
 /* (Implemented in C because in corrupt files there may be a huge amount of garbage which
 ** would be extremely slow to skip with a "*" regexp, esp. if it contains many zero bytes;
 ** also not using ".{0,2000}" because this generates huge state transition tables) */

<resync>[\0-\xff]                               {
                                                   char buf[50];
                                                   int c = (uchar) yytext[0];
                                                   int skip_count = 1;
                                                   buf[0] = c;
                                                   if (c != xml_scan_resync_char)
                                                   {
                                                      do {
                                                         c = input();
                                                         if (skip_count < sizeof(buf))
                                                            buf[skip_count] = c;
                                                         skip_count++;
                                                      } while ( (c != EOF) && (c != xml_scan_resync_char) && (skip_count < 2000) );
                                                   }
                                                   if (xml_scan_resync_start)
                                                   {  /* print error message and the offending characters in the input stream */
                                                      if (skip_count < sizeof(buf))
                                                         buf[skip_count] = 0;
                                                      else
                                                         buf[sizeof(buf) - 1] = 0;
                                                      Xmltv_SyntaxError(p_xml_scan_err_msg, buf);
                                                      /* abort handling if the above call changed the state */
                                                      if (YYSTATE != resync)
                                                         break;
                                                   }
                                                   if (c == EOF)
                                                   {
                                                      BEGIN xml_scan_resync_state;
                                                      if (xml_entity_stack_depth > 0)
                                                         XmlScan_PopEntityBuffer();
                                                      else
                                                         yyterminate();
                                                   }
                                                   else
                                                   {
                                                      if (c == xml_scan_resync_char)
                                                      {  /* sync char found -> resume scanning */
                                                         BEGIN xml_scan_resync_state;
                                                         unput(c);
                                                      }
                                                      else if (xml_scan_resync_start == 0)
                                                      {  /* sync not found yet -> keep reporting to allow parser to abort
                                                         ** after a finite amount of data (e.g. in case some joker feeds us /dev/zero) */
                                                         Xmltv_SyntaxError("Skipped 2000 characters, still trying to resync with input after error...", "");
                                                      }
                                                      xml_scan_resync_start = 0;
                                                   }
                                                }

<stopped>[\0-\xff]                              {  BEGIN stopped_end; return XMLTOK_CONTENT_END; }
<stopped_end>[\0-\xff]                          {  YY_FLUSH_BUFFER; yyterminate(); }

 /* Check the parser state at end-of-file or replacement string buffer */

<<EOF>>                                         {
                                                   if (xml_entity_stack_depth > 0)
                                                   {
                                                      XmlScan_PopEntityBuffer();
                                                   }
                                                   else
                                                   {
                                                      if (YYSTATE == prolog)
                                                         ;
                                                      else if (YYSTATE == content)
                                                         ; /* also an error, but handled at semantic level */
                                                      else if ((YYSTATE >= cdata0) && (YYSTATE < cdata2))
                                                         Xmltv_SyntaxError("end-of-file inside CDATA", "");
                                                      else if ((YYSTATE >= comment0) && (YYSTATE < comment2))
                                                         Xmltv_SyntaxError("end-of-file inside comment", "");
                                                      else
                                                         Xmltv_SyntaxError("end-of-file inside markup", "");
                                                      yyterminate();
                                                   }
                                                }

%%

/* ----------------------------------------------------------------------------
** Pop replacement buffer from the entity stack
** - called after all text in the buffer has been parsed
*/
static void XmlScan_PopEntityBuffer( void )
{
   if (xml_entity_stack_depth > 0)
   {
      yy_delete_buffer(YY_CURRENT_BUFFER);
      xml_entity_stack_depth -= 1;
      yy_switch_to_buffer(xml_entity_stack[xml_entity_stack_depth].buffer);

      if (xml_entity_stack[xml_entity_stack_depth].change_state)
      {
         BEGIN xml_entity_stack[xml_entity_stack_depth].prev_state;
      }
      else /* check if state changed */
      {
         if (YYSTATE != xml_entity_stack[xml_entity_stack_depth].prev_state)
         {
            BEGIN xml_entity_stack[xml_entity_stack_depth].prev_state;
            Xmltv_SyntaxError("Partial markup in entity replacement text", "");
         }
      }
   }
}

/* ----------------------------------------------------------------------------
** Retrieve and parse an entity value (replacement string)
*/
static int XmlScan_ParseEntity( const char * p_buffer, int scan_state, bool change_state )
{
   int result;

   if (xml_entity_stack_depth < MAX_ENTITY_REF_DEPTH)
   {
      xml_entity_stack[xml_entity_stack_depth].buffer = YY_CURRENT_BUFFER;
      xml_entity_stack[xml_entity_stack_depth].prev_state = YYSTATE;
      xml_entity_stack[xml_entity_stack_depth].change_state = change_state;

      if (change_state)
      {
         BEGIN scan_state;
      }

      xml_entity_stack_depth += 1;
      yy_scan_string(p_buffer);
      result = 1;
   }
   else
   {
      Xmltv_SyntaxError("Entity references nested too deeply", yytext);
      result = 0;
   }
   return result;
}

/* ----------------------------------------------------------------------------
** Retrieve an entity value
** - name must include the entity prefix "&" or "%", but not the ";" at the end
*/
static const char * XmlScan_QueryEntity( const char * pName )
{
   XML_ENTITY_HASH_VAL * pEntVal;
   const char * pResult = NULL;

   if (pName != NULL)
   {
      pEntVal = XmlHash_SearchEntry(pEntityHash, pName);
      if (pEntVal != NULL)
      {
         /* name found and type matches -> return value */
         pResult = pEntVal->pValue;
      }

      if (pResult == NULL)
      {
         if (*pName == '%')
            Xmltv_SyntaxError("Parse error: undefined parameter entity", pName);
         else
            Xmltv_SyntaxError("Parse error: undefined (general) entity", pName);
      }
   }
   else
      fatal0("XmlScan-QueryEntity: illegal NULL ptr param");

   return pResult;
}

/* ----------------------------------------------------------------------------
** Part #1 of an entity addition: store the entity name
** - note parameter and general entity have separate namespaces;
**   hence the prefix is included in the name
*/
void XmlScan_EntityDefName( const char * pName, int isParamEntity )
{
   uint  nameLen;

   XmlCdata_Reset(&xml_entity_new_name);

   nameLen = strlen(pName);
   if (nameLen > 0)
   {
      XmlCdata_AppendRaw(&xml_entity_new_name, isParamEntity ? "%":"&", 1);
      XmlCdata_AppendRaw(&xml_entity_new_name, pName, nameLen);
   }
   else  // should never happen, scanner doesn't deliver zero-len names
      debug0("XmlCData-EntityDefName: zero-length name is invalid");
}

/* ----------------------------------------------------------------------------
** Part #2 of an entity addition: create the hash entry and store the value
*/
void XmlScan_EntityDefValue( const char * pValue )
{
   XML_ENTITY_HASH_VAL * pEntVal;
   bool isNew;

   if (XML_STR_BUF_GET_STR_LEN(xml_entity_new_name) > 0)
   {
      pEntVal = XmlHash_CreateEntry(pEntityHash, XML_STR_BUF_GET_STR(xml_entity_new_name), &isNew);
      if (isNew)
      {
         dprintf2("XmlCData-EntityDefValue: entity '%s' = '%s'\n", XML_STR_BUF_GET_STR(xml_entity_new_name), pValue);
         /*assert(sizeof(XML_ENTITY_HASH_VAL) <= HASH_PAYLOAD_SIZE);*/

         pEntVal->pValue = xstrdup(pValue);
      }
      else
         debug2("XmlCData-EntityDefValue: entity '%s' already defined - discard assignment '%s'", XML_STR_BUF_GET_STR(xml_entity_new_name), pValue);
   }
   else
      debug1("XmlCData-EntityDefValue: internal error: entity name not previously defined; discarding value '%s'", pValue);
}

/* ----------------------------------------------------------------------------
** Callback helper function to free entity value strings
** - called when hash array is destroyed
*/
static void XmlScan_EntityDefFree( XML_HASH_PTR pHash, char * pPayload )
{
   XML_ENTITY_HASH_VAL * pEntVal;

   pEntVal = (void *) pPayload;
   xfree(pEntVal->pValue);
}

/* ----------------------------------------------------------------------------
** Set encoding
** - called when the <?xml encoding="..."?> attribute is evaluated by the app.
** - note: the primary encoding is automatically selected by the scanner upon
**   reading the first few bytes; this is necessary to be able to read the
**   <?xml?> tag in the first place; here we can only fine-tune the encoding,
**   i.e. switch between related codes
*/
bool XmlScan_SetEncoding( XML_ENCODING encoding )
{
   bool result;

   if (xml_scan_encoding == encoding)
   {
      result = TRUE;
   }
   else if ( (xml_scan_encoding == XML_ENC_UTF8) &&
             (encoding == XML_ENC_ISO8859) )
   {
      xml_scan_encoding = encoding;
      result = TRUE;
   }
   else if ( ( (xml_scan_encoding == XML_ENC_UTF16BE) ||
               (xml_scan_encoding == XML_ENC_UTF16LE) ) &&
             ( (encoding == XML_ENC_UTF16BE) ||
               (encoding == XML_ENC_UTF16LE) ) )
   {
      // don't copy new value: don't touch endianess - better rely on auto-detection
      result = TRUE;
   }
   else
      result = FALSE;

   if (xml_scan_encoding == XML_ENC_ISO8859)
   {
#ifdef XMLTV_OUTPUT_UTF8
      XmlScan_CdataAppend = XmlCdata_AppendLatin1ToUtf8;
#else /* output Latin-1 */
      XmlScan_CdataAppend = XmlCdata_AppendRawNOINLINE;
#endif
   }

   return result;
}

/* ----------------------------------------------------------------------------
** Early stop of the scanner - simulate end of file
*/
void XmlScan_Stop( void )
{
   BEGIN stopped;
}

/* ----------------------------------------------------------------------------
** Free resources
*/
void XmlScan_Destroy( void )
{
   XmlHash_Destroy(pEntityHash, XmlScan_EntityDefFree);
   XmlCdata_Free(&xml_entity_new_name);
   XmlCdata_Free(&xml_attr);
   XmlCdata_Free(&xml_content);

   /* free entity stack, in case the parser is stopped
   ** during parsing an entity replacement text */
   while (xml_entity_stack_depth > 0)
   {
      yy_delete_buffer(YY_CURRENT_BUFFER);
      xml_entity_stack_depth -= 1;
      yy_switch_to_buffer(xml_entity_stack[xml_entity_stack_depth].buffer);
   }

   /* free internal state and buffer */
   yy_delete_buffer(YY_CURRENT_BUFFER);
   yy_init = 1;
}

/* ----------------------------------------------------------------------------
** Initialize module state (C operators only)
*/
void XmlScan_Init( void )
{
   /* Make a bogus use of yy_fatal_error() to avoid spurious warning */
   (void) &yy_fatal_error;

   XmlCdata_Init(&xml_attr, 256);
   XmlCdata_Init(&xml_content, 4096);
   XmlCdata_Init(&xml_entity_new_name, 256);
   pEntityHash = XmlHash_Init();

   xml_entity_stack_depth = 0;
   xml_scan_encoding = XML_ENC_UNKNOWN;

   /* select transcoding function for character data output */
   /* default input encoding is UTF-8 (when no <?xml?> is present) */
#ifdef XMLTV_OUTPUT_UTF8
   XmlScan_CdataAppend = XmlCdata_AppendRawNOINLINE;
#else
   XmlScan_CdataAppend = XmlCdata_AppendUtf8ToLatin1;
#endif

   BEGIN INITIAL;
}


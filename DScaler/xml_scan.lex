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
 *  $Id: xml_scan.lex,v 1.1 2005-07-06 19:42:39 laurentg Exp $
 */

%{
#include <string.h>
#include <stdlib.h>

#include "mytypes.h"
#include "debug.h"

#include "xml_cdata.h"
#include "xml_hash.h"
#include "xml_prolog.tab.h"
#include "xmltv_db.h"
#include "xmltv_tags.h"


/* buffer used to assemble attribute values */
static XML_STR_BUF xml_attr;
static XML_STR_BUF xml_content;

/* stack used to parse nestesd entity references
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

/* hash and temporary variables used for entity definitions */
static XML_HASH_PTR pEntityHash;
static XML_STR_BUF xml_entity_new_name;

typedef struct
{
   char       * pValue;
} XML_ENTITY_HASH_VAL;

/* temporary flag which indicates the top-level tag was closed */
static int xml_scan_docend;
/* temporary pointer which holds an error message */
static const char * p_xml_scan_err_msg;
/* temporary cache for state while processing PI tag */
static int xml_scan_pre_pi_state;

/* forward declarations of internal operators
** (note: external operators are declard in xmltv_tag.h for lack of a scanner header file)
*/
static int XmlScan_ParseEntity( const char * p_buffer, int scan_state, bool change_state );
static const char * XmlScan_QueryEntity( const char * pName );

/* ----------------------------------------------------------------------------
** Pass pre-processed content data to parser
** - this function is called when a beginning to scan a tag (either opening or
**   closing tag); hence for elements with mixed content data which is separated
**   by child tags is forwarded separately
** - note the content may be assembled from multiple chunks if there are CDATA
**   sections or parsed entities
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
** Add character reference to string buffer
** - note: no need to check for parse errors, because the scanner already
**   made sure we only have decimal or hexadecimal digits; string is not
**   terminated by 0, instead by ';'
*/
#ifdef __GNUC__
__inline
#endif
static void XmlScan_AddCharRef( XML_STR_BUF * p_buf, char * p_text, int base )
{
   ulong lval;
   char cval[1];

   lval = strtol(p_text, NULL, base);
   if (lval > 255)
   {
      Xmltv_SyntaxError("Unsupported UTF character in entity character reference", p_text);
   }
   else if (lval == 0)
   {
      Xmltv_SyntaxError("Disallowed entity character reference", "&#0;");
   }
   else
   {
      cval[0] = (char) lval;
      XmlCdata_AppendRaw(p_buf, cval, 1);
   }
}

int yylex( void );
#define YY_NO_UNPUT
#define YY_USE_CONST
#define YY_USE_PROTOS
#define ECHO do{debug2("XMLTV yylex unmatched text in state %d: #%s#", YYSTATE, yytext);}while(0)
%}

%option noyywrap
%option never-interactive

 /* Auto-detection of encoding; required to parse <?xml encoding=...?>, see XML 1.0 apx. F.1 */
DETECT_UCS4_BOM         (\0\0(\xFE\xFF)|(\xFF\xFE))|((\xFF\xFE)|(\xFE\xFF)\0\0)
DETECT_UTF16_BOM        (\xFE\xFF|\xFF\xFE)([^\0][\0-\xff]|[\0-\xff][^\0])
DETECT_UTF8_BOM         \xEF\xBB\xBF
DETECT_UCS4             (\0\0\0\x3C)|(\x3C\0\0\0)|(\0\0\x3C\0)|(\0\x3C\0\0)
DETECT_UTF16            (\0\x3C\0\x3F)|(\x3C\0\x3F\0)
DETECT_UTF8_ASCII       "<?x"
DETECT_EBCDIC           \x4C\x6F\xA7\x94

XMLSPACE        [ \t\n\r]+
XMLNAME         [a-zA-Z_:][a-zA-Z0-9.\-_:]*
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
%s stopped
%s stopped_end

%%

 /* ------------  P  R  O  L  O  G  ------------ */

 /* Auto-detect character encoding <?xml...?> */

<INITIAL>{DETECT_UCS4_BOM}                      { Xmltv_SyntaxError("Unsupported encoding", "UCS-4"); yyterminate(); }
<INITIAL>{DETECT_UTF16_BOM}                     { Xmltv_SyntaxError("Unsupported encoding", "UTF-16"); yyterminate(); }
<INITIAL>{DETECT_UTF8_BOM}                      { Xmltv_SyntaxError("Unsupported encoding", "UTF-8"); yyterminate(); }
<INITIAL>{DETECT_UCS4}                          { Xmltv_SyntaxError("Unsupported encoding", "UCS-4"); yyterminate(); }
<INITIAL>{DETECT_UTF16}                         { Xmltv_SyntaxError("Unsupported encoding", "UTF-16"); yyterminate(); }
<INITIAL>{DETECT_UTF8_ASCII}                    { BEGIN prolog; yyless(0); }
<INITIAL>{DETECT_EBCDIC}                        { Xmltv_SyntaxError("Unsupported encoding", "EBCDIC"); yyterminate(); }
<INITIAL>.                                      { BEGIN prolog; yyless(0); }

<prolog>{XMLSPACE}                              { return XMLTOK_SPACE; }

 /* Scan encoding <?xml...?> */

<prolog>"<?xml"                                 {  BEGIN xmlenc; return XMLTOK_TAGENC; }
<xmlenc>"version"                               {  BEGIN xmlencv; return XMLTOK_VERSION; }
<xmlencv>"="                                    {  return XMLTOK_EQ; }
<xmlencv>\'[0-9.]+\'|\"[0-9.]+\"                {  BEGIN xmlenc;
                                                   yylval.str = yytext;
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
<xmlenc,xmlencv,xmlencsdecl,xmlencenc>.         {  BEGIN xmlencerr; yyless(0); }
<xmlencerr>[^>]*\>                              {  BEGIN prolog;
                                                   Xmltv_SyntaxError("Malformed encoding declaration", yytext);
                                                }

 /* Scan "processing instructions" (e.g. <?foo-bar...?>) */

<prolog,content>"<?"                            {  xml_scan_pre_pi_state = YYSTATE;
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
<pi0>[^\?\r]+                                   {  XmlCdata_AppendRaw(&xml_attr, yytext, yyleng); }
<pi1>">"                                        {  BEGIN piclose; yyless(0);
                                                   yylval.str = XML_STR_BUF_GET_STR(xml_attr);
                                                   return XMLTOK_PICONTENT;
                                                }
<pi1>"?"                                        {  XmlCdata_AppendRaw(&xml_attr, "?", 1); }
<pi1>\r+                                        {  BEGIN pi0; /* dropped, but QM in stack is appended */
                                                   XmlCdata_AppendRaw(&xml_attr, "?", 1);
                                                }
<pi1>[^\?\>]                                    {  BEGIN pi0;
                                                   XmlCdata_AppendRaw(&xml_attr, "?", 1);
                                                   XmlCdata_AppendRaw(&xml_attr, yytext, yyleng);
                                                }
<piclose>">"                                    {  BEGIN xml_scan_pre_pi_state; return XMLTOK_CLOSE; }
<piname,pispace>.                               {  BEGIN pierr; yyless(0); }
<pierr>[^>]*>                                   {  BEGIN xml_scan_pre_pi_state;
                                                   Xmltv_SyntaxError("Parse error in processing instruction", yytext);
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
<doctype,doctypedecl,doctypesys,doctypepub>.    {  BEGIN prologerr2; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error in DOCTYPE declaration";
                                                }

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
<entval1,entval2,entval3>&{XMLNAME};            {  XmlCdata_AppendRaw(&xml_attr, yytext, yyleng); /* general entity references are bypassed */ }
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
<entval1,entval2,entval3>[&%]                   {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Malformed entity reference in entity declaration";
                                                }
<entval1,entval2>\r+                            {  /*dropped*/ }
<entval1>[^\"\&\%\r]+                           {  XmlCdata_AppendRaw(&xml_attr, yytext, yyleng); }
<entval2>[^\'\&\%\r]+                           {  XmlCdata_AppendRaw(&xml_attr, yytext, yyleng); }
<entval3>[^\&\%]+                               {  XmlCdata_AppendRaw(&xml_attr, yytext, yyleng); }
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
<entdef,entval,entsys,entpub,entndata>.         {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Malformed entity declaration in DTD";
                                                }

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
<docdtd>"%".                                    {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Malformed parameter entity reference";
                                                }
<docdtd>"&"                                     {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Disallowed entity reference in DTD";
                                                }

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
<dtdelem,dtdelspec,dtdeldef>.                   {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Malformed element declaration in DTD";
                                                }

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
<dtdattenu>[a-zA-Z0-9.\-_:]+                    {  return XMLTOK_NMTOKEN; }
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
<dtdatt,dtdattnam,dtdatttyp,dtdattnot,dtdattenu>. {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Malformed ATTLIST declaration in DTD";
                                                }

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
<docnotnam,docnottyp,docnotsys,docnotpub>.      {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Malformed notation declaration in DTD";
                                                }

 /* DTD error rules */

<docdtd>.                                       {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error: unknown markup in DTD";
                                                }
<docdtderr>[^>]*>                               {  BEGIN docdtd;
                                                   Xmltv_SyntaxError(p_xml_scan_err_msg, yytext);
                                                }

 /* Transition from prolog into content area */

<prolog>\<{XMLNAME}                             {  BEGIN content; yyless(0); }
<prolog>\<                                      {  BEGIN prologerr2; yyless(0);
                                                   p_xml_scan_err_msg = "Invalid markup in XML prolog";
                                                }
<prolog>.                                       {  BEGIN prologerr1; yyless(0);
                                                   p_xml_scan_err_msg = "Garbage in XML prolog";
                                                }
<prologerr1>[^<]+<                              {  BEGIN prolog;
                                                   Xmltv_SyntaxError(p_xml_scan_err_msg, yytext);
                                                   yyless(yyleng - 1);
                                                }
<prologerr2>[^>]+>                              {  BEGIN prolog;
                                                   Xmltv_SyntaxError(p_xml_scan_err_msg, yytext);
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
<content>"</".                                  {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error in closing tag: not a valid tag name";
                                                }

<tagcloseend>[[:space:]]*\>                     {
                                                   if (xml_scan_docend == 0)
                                                   {
                                                      BEGIN prolog;
                                                      return XMLTOK_CONTENT_END;
                                                   }
                                                   else
                                                      BEGIN content;
                                                }

<tagcloseend>.                                  {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Garbage in closing tag after tag name";
                                                }

 /* Skip comments <!-- ... --> */

<prolog,docdtd,content>{COMMENT}                {
                                                   static const char * const echars = "-->";
                                                   int c;
                                                   int estate = 0;

                                                   /* eat up text of comment */
                                                   while (1)
                                                   {
                                                      c = input();
                                                      if (c != EOF)
                                                      {
                                                         /* check for stop sequence */
                                                         if (c == echars[estate])
                                                         {
                                                            estate++;
                                                            if (estate >= 3)
                                                               break;
                                                         }
                                                         else
                                                            estate = 0;
                                                         /* note: the XML spec disallows "--" inside comments;
                                                         ** we could emit a warning, but currently it's ignored */
                                                      }
                                                      else
                                                      {
                                                         Xmltv_SyntaxError("end-of-file inside comment", "");
                                                         return 0;
                                                         break;
                                                      }
                                                   }
                                                   /*printf("COMMENT skipped\n");*/
                                                   /* no return, continue scanning */
                                                }

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
<tagname>.                                      {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error in opening tag: not a valid tag name";
                                                }

<tagattrspace>[[:space:]]+                      {  BEGIN tagattrname; }

<tagattrname>{XMLNAME}                          {  BEGIN tagattrass;
                                                   XmltvTags_AttribIdentify(yytext);
                                                }

<tagattrass>[[:space:]]*=[[:space:]]*           {  BEGIN tagattrval;
                                                   XmlCdata_Reset(&xml_attr);
                                                }
<tagattrass>.                                   {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error in tag attribute assignment: expected equal sign";
                                                }

<tagattrval>\"                                  {  BEGIN tagattrval1; }
<tagattrval>\'                                  {  BEGIN tagattrval2; }
<tagattrval>.                                   {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error in tag attribute assignment: expected opening quote";
                                                }
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
                                                      XmlCdata_AppendRaw(&xml_attr, yytext, yyleng - 1);
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
<tagattrval1,dtdattval1>[^\"\<\&\n\r\t]+        {  XmlCdata_AppendRaw(&xml_attr, yytext, yyleng); }
<tagattrval2,dtdattval2>[^\'\<\&\n\r\t]+        {  XmlCdata_AppendRaw(&xml_attr, yytext, yyleng); }
<tagattrval3>[^\<\&\n\r\t]+                     {  XmlCdata_AppendRaw(&xml_attr, yytext, yyleng); }

<tagattrval1>\"                                 {  BEGIN tagattrspace;
                                                   XmltvTags_AttribData(&xml_attr);
                                                }
<tagattrval2>\'                                 {  BEGIN tagattrspace;
                                                   XmltvTags_AttribData(&xml_attr);
                                                }

<tagattrval1,tagattrval2>.                      {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Invalid character in attribute value";
                                                }
<tagattrval3>.                                  {  BEGIN tagattrval3err; yyless(0); }
<tagattrval3err>[\0-\xff]+                      {  /* discard remaining entity replacement text */
                                                   Xmltv_SyntaxError("Invalid character in attribute value", yytext);
                                                }
<dtdattval1,dtdattval2>.                        {  BEGIN docdtderr; yyless(0);
                                                   p_xml_scan_err_msg = "Invalid character in default attribute value in DTD ATTLIST declaration";
                                                }
<tagattrname,tagattrspace>"/>"                  {  BEGIN content;
                                                   if (XmltvTags_Close(NULL) == FALSE)
                                                   {
                                                      BEGIN prolog;
                                                      return XMLTOK_CONTENT_END;
                                                   }
                                                }

<tagattrname,tagattrspace>\/[^>]                {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error at tag end: garbage after /";
                                                }

<tagattrname,tagattrspace>">"                   {  BEGIN content;
                                                   XmltvTags_AttribsComplete();
                                                }

<tagattrspace>.                                 {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error in tag: garbage after tag name or assignment";
                                                }
<tagattrname>.                                  {  BEGIN tagerror; yyless(0);
                                                   p_xml_scan_err_msg = "Parse error in tag: expected assignment or closing tag";
                                                }

<tagerror>[^>]*\>                               {  BEGIN content;
                                                   Xmltv_SyntaxError(p_xml_scan_err_msg, yytext);
                                                   p_xml_scan_err_msg = NULL;
                                                }


 /* CDATA inside content area (unparsed character data) */

<content>{CDATA}                                {  BEGIN cdata0; }

<cdata0>"]"                                     {  BEGIN cdata1; }
<cdata0>\r                                      {  /*dropped*/ }
<cdata0>[^\]\r]+                                {  XmlCdata_AppendRaw(&xml_content, yytext, yyleng); }
<cdata1>"]"                                     {  BEGIN cdata2; }
<cdata1>.                                       {  BEGIN cdata0;
                                                   XmlCdata_AppendRaw(&xml_content, "]", 1);
                                                   if (yytext[0] != '\r')
                                                      XmlCdata_AppendRaw(&xml_content, yytext, 1);
                                                }
<cdata2>">"                                     {  BEGIN content; }
<cdata2>"]"                                     {  XmlCdata_AppendRaw(&xml_content, "]", 1); }
<cdata2>.                                       {  BEGIN cdata0;
                                                   XmlCdata_AppendRaw(&xml_content, "]]", 2);
                                                   if (yytext[0] != '\r')
                                                      XmlCdata_AppendRaw(&xml_content, yytext, 1);
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
                                                      XmlCdata_AppendRaw(&xml_content, yytext, yyleng - 1);
                                                      XmlCdata_AppendRaw(&xml_content, ";", 1);
                                                   }
                                                   else
                                                   {
                                                      XmlScan_ParseEntity(pValue, 0, 0);
                                                   }
                                                }
<content>&                                      {
                                                   Xmltv_SyntaxError("Parse error in entity reference:", yytext);
                                                   XmlCdata_AppendRaw(&xml_content, "&", 1);
                                                }
<content>\r+                                    {  if (xml_entity_stack_depth > 0)
                                                   {  /* not dropped when inserted as "&#13;" in entity replacement text (testcase valid/sa/068.xml) */
                                                      XmlCdata_AppendRaw(&xml_content, yytext, yyleng);
                                                   }
                                                }
<content>[^<&\r]+                               {
                                                   XmlCdata_AppendRaw(&xml_content, yytext, yyleng);
                                                }


 /* Check the parser state at end-of-file or replacement string buffer */

<stopped>.                                      {  BEGIN stopped_end; return XMLTOK_CONTENT_END; }
<stopped_end>.                                  {  YY_FLUSH_BUFFER; yyterminate(); }


<<EOF>>                                         {
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
                                                            Xmltv_SyntaxError("Partial markup in entity replacement text:", "");
                                                            BEGIN xml_entity_stack[xml_entity_stack_depth].prev_state;
                                                         }
                                                      }
                                                   }
                                                   else
                                                   {
                                                      if (YYSTATE == prolog)
                                                         ;
                                                      else if (YYSTATE == content)
                                                         ; /* also an error, but handled at semantic level */
                                                      else if ((YYSTATE >= cdata0) && (YYSTATE < cdata2))
                                                         Xmltv_SyntaxError("end-of-file inside CDATA", "");
                                                      else
                                                         Xmltv_SyntaxError("end-of-file inside markup", "");
                                                      yyterminate();
                                                   }
                                                }

%%

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
         assert(sizeof(XML_ENTITY_HASH_VAL) <= HASH_PAYLOAD_SIZE);

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
** Early stop of the scanner - simulate end of file
*/
void XmlScan_Stop( void )
{
   BEGIN stopped;
}

/* ----------------------------------------------------------------------------
** Free resources allocated by C operators
*/
void XmlScan_Destroy( void )
{
   XmlHash_Destroy(pEntityHash, XmlScan_EntityDefFree);
   XmlCdata_Free(&xml_entity_new_name);
   XmlCdata_Free(&xml_attr);
   XmlCdata_Free(&xml_content);
}

/* ----------------------------------------------------------------------------
** Initialize module state (C operators only)
*/
void XmlScan_Init( void )
{
   XmlCdata_Init(&xml_attr, 256);
   XmlCdata_Init(&xml_content, 4096);
   XmlCdata_Init(&xml_entity_new_name, 256);
   pEntityHash = XmlHash_Init();

   xml_entity_stack_depth = 0;

   p_xml_scan_err_msg = NULL;

   BEGIN INITIAL;
}


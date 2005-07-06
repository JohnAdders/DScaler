/*
 *  Parser for XML 1.0
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
 *    This module parses an XML file with rules directly derived from
 *    W3C XML spec 1.0 (3rd edition)  The content (i.e. all elements and
 *    character data) are reduced to a single token however because the
 *    scanner directly passes tags, attributes and character data to
 *    the parser.
 *
 *  Author: Tom Zoerner
 *
 *  $Id: xml_prolog.yy,v 1.1 2005-07-06 19:42:39 laurentg Exp $
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "xmltv_mytypes.h"
#include "xmltv_debug.h"

#include "xml_prolog.tab.h"
#include "xml_cdata.h"
#include "xmltv_db.h"
#include "xmltv_tags.h"

extern int yylex( void );
void yyerror( const char * p_msg );

#define YYDEBUG DEBUG_SWITCH_XMLTV
%}

%union { char * str; }

%token <str> XMLTOK_NAME XMLTOK_ENCNAME XMLTOK_ENCVERS XMLTOK_ATTVAL
%token XMLTOK_TAGDOC XMLTOK_TAGENC XMLTOK_TAGENT XMLTOK_TAGPI
%token XMLTOK_TAGELEM XMLTOK_TAGATT XMLTOK_TAGNOT
%token <str> XMLTOK_SYSLIT XMLTOK_PUBLIT XMLTOK_ENTVAL XMLTOK_PICONTENT
%token XMLTOK_CLOSE XMLTOK_SPACE XMLTOK_CONTENT XMLTOK_CONTENT_END
%token XMLTOK_ENCODING XMLTOK_STANDALONE XMLTOK_YES XMLTOK_NO XMLTOK_CHR_PERCENT
%token XMLTOK_VERSION XMLTOK_NDATA XMLTOK_EQ XMLTOK_STR_PUBLIC XMLTOK_STR_SYSTEM
%token XMLTOK_DTD_OPEN XMLTOK_DTD_CLOSE
%token XMLTOK_STR_EMPTY XMLTOK_STR_ANY XMLTOK_STR_PCDATA XMLTOK_CHR_OPBR
%token XMLTOK_CHR_CLBR XMLTOK_CHR_ASTX XMLTOK_CHR_PLUS XMLTOK_CHR_QUMK
%token XMLTOK_CHR_PIPE XMLTOK_CHR_COMA
%token XMLTOK_STR_FIXED XMLTOK_STR_NOTATION XMLTOK_STR_ENTITY XMLTOK_STR_ENTITIES
%token XMLTOK_STR_REQUIRED XMLTOK_STR_NMTOKEN XMLTOK_STR_CDATA XMLTOK_STR_IMPLIED
%token XMLTOK_STR_NMTOKENS XMLTOK_STR_ID XMLTOK_STR_IDREF XMLTOK_STR_IDREFS
%token XMLTOK_NMTOKEN

%start  xml

%%

xml             : prolog content ;
prolog          : XMLDeclO MiscL doctypeMiscO ;

 /* XML encoding: <?xml version="1.0" encoding="ISO-8859-1"?> */
 /* no space or comments are allowed before this element in the document */
XMLDeclO        : | XMLDecl ;
XMLDecl         : XMLTOK_TAGENC VersionInfo XMLDecl2 ;
XMLDecl2        : SO XMLTOK_CLOSE
                | SS EncodingDecl SO XMLTOK_CLOSE
                | SS EncodingDecl SS SDDecl SO XMLTOK_CLOSE
                | SS SDDecl SO XMLTOK_CLOSE ;
EncodingDecl    : XMLTOK_ENCODING Eq EncName ;
SDDecl          : XMLTOK_STANDALONE Eq YesOrNo ;
YesOrNo         : XMLTOK_YES | XMLTOK_NO;
VersionInfo     : SS XMLTOK_VERSION Eq VersionStr ;
Eq              : SO XMLTOK_EQ SO ;
VersionStr      : XMLTOK_ENCVERS;

 /* uninterpreted filler stuff */
MiscL           : | MiscL Misc ;
Misc            : /*| Comment |*/ PI | SS ;
PI              : XMLTOK_TAGPI PIName PITarget XMLTOK_CLOSE ;
PITarget        : { XmltvTags_PiContent(NULL); }
                | SS PIContent;

 /* document declaration: <!DOCTYPE tv SYSTEM "xmltv.dtd"> */
doctypeMiscO    : | doctypedecl MiscL ;
doctypedecl     : XMLTOK_TAGDOC SS DocName docExtIDIntO XMLTOK_CLOSE ;
docExtIDIntO    :
                | SS ExternalID
                | SS ExternalID SO intSubsetDecl {XmltvTags_DocIntDtdClose();}
                | SO intSubsetDecl {XmltvTags_DocIntDtdClose();} ;
intSubsetDecl   : XMLTOK_DTD_OPEN intSubsetL XMLTOK_DTD_CLOSE SO ;
intSubsetL      : | intSubsetL intSubset ;
intSubset       : markupdecl | DeclSep ;
markupdecl      : elementdecl | AttlistDecl | EntityDecl | NotationDecl | XMLTOK_TAGPI /*| Comment*/ ;
DeclSep         : /*PEReference |*/ SS ;

 /* entity declaration (in internal DTD) */
EntityDecl      : GEDecl | PEDecl ;
GEDecl          : XMLTOK_TAGENT SS GEName SS EntityDef XMLTOK_CLOSE ;
PEDecl          : XMLTOK_TAGENT SS XMLTOK_CHR_PERCENT SS PEName SS PEDef SO XMLTOK_CLOSE ;
EntityDef       : EntityValue SO | ExternalID NDataDeclO ;
PEDef           : EntityValue | ExternalID ;
ExternalID      : XMLTOK_STR_SYSTEM SS SystemLiteral
                | XMLTOK_STR_PUBLIC SS PubidLiteral SS SystemLiteral ;
NDataDeclO      : SO | NDataDecl SO ;
NDataDecl       : SS XMLTOK_NDATA SS Name ;

 /* DTD element declaration */
elementdecl     : XMLTOK_TAGELEM SS Name SS contentspec SO XMLTOK_CLOSE ;
contentspec     : XMLTOK_STR_EMPTY
                | XMLTOK_STR_ANY
                | Mixed
                | children ;
children        : choice childReCharO
                | seq childReCharO ;
cp              : Name childReCharO
                | choice childReCharO
                | seq childReCharO ;
choice          : XMLTOK_CHR_OPBR SO cp SO choiceNextL1 SO XMLTOK_CHR_CLBR ;
choiceNextL1    : choiceNext | choiceNextL1 SO choiceNext ;
choiceNext      : XMLTOK_CHR_PIPE SO cp ;
seq             : XMLTOK_CHR_OPBR SO cp seqNextL XMLTOK_CHR_CLBR ;
seqNextL        : SO | seqNextL seqNext SO ;
seqNext         : XMLTOK_CHR_COMA SO cp ;
Mixed           : XMLTOK_CHR_OPBR SO XMLTOK_STR_PCDATA SO MixedPipeL1 SO XMLTOK_CHR_CLBR XMLTOK_CHR_ASTX
                | XMLTOK_CHR_OPBR SO XMLTOK_STR_PCDATA SO XMLTOK_CHR_CLBR ;
 /* note: the spec allows for zero occurences of MixedPipe but that's ambiguous
 ** because it would also be matched by "(#PCDATA)*" */
MixedPipeL1     : MixedPipe | MixedPipeL1 SO MixedPipe ;
MixedPipe       : XMLTOK_CHR_PIPE SO Name ;
childReCharO    : | childReChar ;
childReChar     : XMLTOK_CHR_QUMK | XMLTOK_CHR_ASTX | XMLTOK_CHR_PLUS ;

 /* DTD attribute declaration */
AttlistDecl     : XMLTOK_TAGATT SS Name AttDefL SO XMLTOK_CLOSE ;
AttDefL         : | AttDefL AttDef ;
AttDef          : SS Name SS AttType SS DefaultDecl ;
AttType         : StringType | TokenizedType | EnumeratedType ;
StringType      : XMLTOK_STR_CDATA ;
TokenizedType   : XMLTOK_STR_ID
                | XMLTOK_STR_IDREF
                | XMLTOK_STR_IDREFS
                | XMLTOK_STR_ENTITY
                | XMLTOK_STR_ENTITIES
                | XMLTOK_STR_NMTOKEN
                | XMLTOK_STR_NMTOKENS ;
EnumeratedType  : NotationType | Enumeration ;
NotationType    : XMLTOK_STR_NOTATION SS XMLTOK_CHR_OPBR SO NotationNameL SO XMLTOK_CHR_CLBR ;
NotationNameL   : Name | NotationNameL SO XMLTOK_CHR_PIPE SO Name ;
Enumeration     : XMLTOK_CHR_OPBR SO EnumNmtokenL SO XMLTOK_CHR_CLBR ;
EnumNmtokenL    : Nmtoken | EnumNmtokenL SO XMLTOK_CHR_PIPE SO Nmtoken ;
DefaultDecl     : XMLTOK_STR_REQUIRED
                | XMLTOK_STR_IMPLIED
                | DefaultFixedO AttValue ;
DefaultFixedO   : | XMLTOK_STR_FIXED SS ;
Nmtoken         : XMLTOK_NMTOKEN ;
AttValue        : XMLTOK_ATTVAL ;

 /* DTD notation declaration */
NotationDecl    : XMLTOK_TAGNOT SS NotationName SS ExtOrPubID XMLTOK_CLOSE ;
ExtOrPubID      : XMLTOK_STR_SYSTEM SS XMLTOK_SYSLIT SO {XmltvTags_Notation(1, $3);}
                | XMLTOK_STR_PUBLIC SS XMLTOK_PUBLIT SS {XmltvTags_Notation(2, $3);} XMLTOK_SYSLIT {XmltvTags_Notation(4, $6);} SO
                | XMLTOK_STR_PUBLIC SS XMLTOK_PUBLIT SO {XmltvTags_Notation(3, $3);} ;

 /* Content: must contain at least one tag (XML 1.0 ch. 2.1); may be interrupted by PI */
 /* toplevel-tag must occur only one time (marked by _END) */
content         : XMLTOK_CONTENT contentL XMLTOK_CONTENT_END MiscL
                | XMLTOK_CONTENT_END MiscL ;
contentL        : | contentL contents;
contents        : XMLTOK_CONTENT | Misc ;

 /* some shared terminals */
SS              : XMLTOK_SPACE;         /* one or more spaces */
SO              : | XMLTOK_SPACE;       /* zero or more spaces */
Name            : XMLTOK_NAME;          /* an identifier */
EncName         : XMLTOK_ENCNAME        { /* TODO */ };
DocName         : XMLTOK_NAME           { XmltvTags_DocType($1); };
NotationName    : XMLTOK_NAME           { XmltvTags_Notation(0, $1); }
SystemLiteral   : XMLTOK_SYSLIT;
PubidLiteral    : XMLTOK_PUBLIT;
PIName          : XMLTOK_NAME           { XmltvTags_PiTarget($1); }
PIContent       : XMLTOK_PICONTENT      { XmltvTags_PiContent($1); }
GEName          : XMLTOK_NAME           { XmlScan_EntityDefName($1, 0); };
PEName          : XMLTOK_NAME           { XmlScan_EntityDefName($1, 1); };
EntityValue     : XMLTOK_ENTVAL         { XmlScan_EntityDefValue($1); };

%%

void yyerror( const char * p_msg )
{
   Xmltv_SyntaxError("Prolog parse error", p_msg);
}


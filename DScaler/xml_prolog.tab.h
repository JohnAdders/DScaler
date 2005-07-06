/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     XMLTOK_NAME = 258,
     XMLTOK_ENCNAME = 259,
     XMLTOK_ENCVERS = 260,
     XMLTOK_ATTVAL = 261,
     XMLTOK_TAGDOC = 262,
     XMLTOK_TAGENC = 263,
     XMLTOK_TAGENT = 264,
     XMLTOK_TAGPI = 265,
     XMLTOK_TAGELEM = 266,
     XMLTOK_TAGATT = 267,
     XMLTOK_TAGNOT = 268,
     XMLTOK_SYSLIT = 269,
     XMLTOK_PUBLIT = 270,
     XMLTOK_ENTVAL = 271,
     XMLTOK_PICONTENT = 272,
     XMLTOK_CLOSE = 273,
     XMLTOK_SPACE = 274,
     XMLTOK_CONTENT = 275,
     XMLTOK_CONTENT_END = 276,
     XMLTOK_ENCODING = 277,
     XMLTOK_STANDALONE = 278,
     XMLTOK_YES = 279,
     XMLTOK_NO = 280,
     XMLTOK_CHR_PERCENT = 281,
     XMLTOK_VERSION = 282,
     XMLTOK_NDATA = 283,
     XMLTOK_EQ = 284,
     XMLTOK_STR_PUBLIC = 285,
     XMLTOK_STR_SYSTEM = 286,
     XMLTOK_DTD_OPEN = 287,
     XMLTOK_DTD_CLOSE = 288,
     XMLTOK_STR_EMPTY = 289,
     XMLTOK_STR_ANY = 290,
     XMLTOK_STR_PCDATA = 291,
     XMLTOK_CHR_OPBR = 292,
     XMLTOK_CHR_CLBR = 293,
     XMLTOK_CHR_ASTX = 294,
     XMLTOK_CHR_PLUS = 295,
     XMLTOK_CHR_QUMK = 296,
     XMLTOK_CHR_PIPE = 297,
     XMLTOK_CHR_COMA = 298,
     XMLTOK_STR_FIXED = 299,
     XMLTOK_STR_NOTATION = 300,
     XMLTOK_STR_ENTITY = 301,
     XMLTOK_STR_ENTITIES = 302,
     XMLTOK_STR_REQUIRED = 303,
     XMLTOK_STR_NMTOKEN = 304,
     XMLTOK_STR_CDATA = 305,
     XMLTOK_STR_IMPLIED = 306,
     XMLTOK_STR_NMTOKENS = 307,
     XMLTOK_STR_ID = 308,
     XMLTOK_STR_IDREF = 309,
     XMLTOK_STR_IDREFS = 310,
     XMLTOK_NMTOKEN = 311
   };
#endif
#define XMLTOK_NAME 258
#define XMLTOK_ENCNAME 259
#define XMLTOK_ENCVERS 260
#define XMLTOK_ATTVAL 261
#define XMLTOK_TAGDOC 262
#define XMLTOK_TAGENC 263
#define XMLTOK_TAGENT 264
#define XMLTOK_TAGPI 265
#define XMLTOK_TAGELEM 266
#define XMLTOK_TAGATT 267
#define XMLTOK_TAGNOT 268
#define XMLTOK_SYSLIT 269
#define XMLTOK_PUBLIT 270
#define XMLTOK_ENTVAL 271
#define XMLTOK_PICONTENT 272
#define XMLTOK_CLOSE 273
#define XMLTOK_SPACE 274
#define XMLTOK_CONTENT 275
#define XMLTOK_CONTENT_END 276
#define XMLTOK_ENCODING 277
#define XMLTOK_STANDALONE 278
#define XMLTOK_YES 279
#define XMLTOK_NO 280
#define XMLTOK_CHR_PERCENT 281
#define XMLTOK_VERSION 282
#define XMLTOK_NDATA 283
#define XMLTOK_EQ 284
#define XMLTOK_STR_PUBLIC 285
#define XMLTOK_STR_SYSTEM 286
#define XMLTOK_DTD_OPEN 287
#define XMLTOK_DTD_CLOSE 288
#define XMLTOK_STR_EMPTY 289
#define XMLTOK_STR_ANY 290
#define XMLTOK_STR_PCDATA 291
#define XMLTOK_CHR_OPBR 292
#define XMLTOK_CHR_CLBR 293
#define XMLTOK_CHR_ASTX 294
#define XMLTOK_CHR_PLUS 295
#define XMLTOK_CHR_QUMK 296
#define XMLTOK_CHR_PIPE 297
#define XMLTOK_CHR_COMA 298
#define XMLTOK_STR_FIXED 299
#define XMLTOK_STR_NOTATION 300
#define XMLTOK_STR_ENTITY 301
#define XMLTOK_STR_ENTITIES 302
#define XMLTOK_STR_REQUIRED 303
#define XMLTOK_STR_NMTOKEN 304
#define XMLTOK_STR_CDATA 305
#define XMLTOK_STR_IMPLIED 306
#define XMLTOK_STR_NMTOKENS 307
#define XMLTOK_STR_ID 308
#define XMLTOK_STR_IDREF 309
#define XMLTOK_STR_IDREFS 310
#define XMLTOK_NMTOKEN 311




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 48 "xml_prolog.yy"
typedef union YYSTYPE { char * str; } YYSTYPE;
/* Line 1249 of yacc.c.  */
#line 150 "xml_prolog.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;




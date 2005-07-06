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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 28 "xml_prolog.yy"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmltv_types.h"
#include "xmltv_debug.h"

#include "xml_prolog.tab.h"
#include "xml_cdata.h"
#include "xmltv_db.h"
#include "xmltv_tags.h"

extern int yylex( void );
void yyerror( const char * p_msg );

#define YYDEBUG DEBUG_SWITCH_XMLTV


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 48 "xml_prolog.yy"
typedef union YYSTYPE { char * str; } YYSTYPE;
/* Line 191 of yacc.c.  */
#line 209 "xml_prolog.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 221 "xml_prolog.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   294

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  82
/* YYNRULES -- Number of rules. */
#define YYNRULES  139
/* YYNRULES -- Number of states. */
#define YYNSTATES  266

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   311

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     6,    10,    11,    13,    17,    20,    25,
      32,    37,    41,    45,    47,    49,    54,    58,    60,    61,
      64,    66,    68,    73,    74,    77,    78,    81,    87,    88,
      91,    96,    99,   104,   105,   108,   110,   112,   114,   116,
     118,   120,   122,   124,   126,   128,   135,   145,   148,   151,
     153,   155,   159,   165,   167,   170,   175,   183,   185,   187,
     189,   191,   194,   197,   200,   203,   206,   214,   216,   220,
     224,   230,   232,   236,   240,   249,   255,   257,   261,   265,
     266,   268,   270,   272,   274,   281,   282,   285,   292,   294,
     296,   298,   300,   302,   304,   306,   308,   310,   312,   314,
     316,   318,   326,   328,   334,   340,   342,   348,   350,   352,
     355,   356,   359,   361,   363,   370,   375,   376,   377,   386,
     391,   396,   399,   400,   403,   405,   407,   409,   410,   412,
     414,   416,   418,   420,   422,   424,   426,   428,   430,   432
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      58,     0,    -1,    59,   123,    -1,    60,    69,    73,    -1,
      -1,    61,    -1,     8,    66,    62,    -1,   127,    18,    -1,
     126,    63,   127,    18,    -1,   126,    63,   126,    64,   127,
      18,    -1,   126,    64,   127,    18,    -1,    22,    67,   129,
      -1,    23,    67,    65,    -1,    24,    -1,    25,    -1,   126,
      27,    67,    68,    -1,   127,    29,   127,    -1,     5,    -1,
      -1,    69,    70,    -1,    71,    -1,   126,    -1,    10,   134,
      72,    18,    -1,    -1,   126,   135,    -1,    -1,    74,    69,
      -1,     7,   126,   130,    75,    18,    -1,    -1,   126,    86,
      -1,   126,    86,   127,    76,    -1,   127,    76,    -1,    32,
      77,    33,   127,    -1,    -1,    77,    78,    -1,    79,    -1,
      80,    -1,    89,    -1,   104,    -1,    81,    -1,   119,    -1,
      10,    -1,   126,    -1,    82,    -1,    83,    -1,     9,   126,
     136,   126,    84,    18,    -1,     9,   126,    26,   126,   137,
     126,    85,   127,    18,    -1,   138,   127,    -1,    86,    87,
      -1,   138,    -1,    86,    -1,    31,   126,   132,    -1,    30,
     126,   133,   126,   132,    -1,   127,    -1,    88,   127,    -1,
     126,    28,   126,   128,    -1,    11,   126,   128,   126,    90,
     127,    18,    -1,    34,    -1,    35,    -1,    99,    -1,    91,
      -1,    93,   102,    -1,    96,   102,    -1,   128,   102,    -1,
      93,   102,    -1,    96,   102,    -1,    37,   127,    92,   127,
      94,   127,    38,    -1,    95,    -1,    94,   127,    95,    -1,
      42,   127,    92,    -1,    37,   127,    92,    97,    38,    -1,
     127,    -1,    97,    98,   127,    -1,    43,   127,    92,    -1,
      37,   127,    36,   127,   100,   127,    38,    39,    -1,    37,
     127,    36,   127,    38,    -1,   101,    -1,   100,   127,   101,
      -1,    42,   127,   128,    -1,    -1,   103,    -1,    41,    -1,
      39,    -1,    40,    -1,    12,   126,   128,   105,   127,    18,
      -1,    -1,   105,   106,    -1,   126,   128,   126,   107,   126,
     115,    -1,   108,    -1,   109,    -1,   110,    -1,    50,    -1,
      53,    -1,    54,    -1,    55,    -1,    46,    -1,    47,    -1,
      49,    -1,    52,    -1,   111,    -1,   113,    -1,    45,   126,
      37,   127,   112,   127,    38,    -1,   128,    -1,   112,   127,
      42,   127,   128,    -1,    37,   127,   114,   127,    38,    -1,
     117,    -1,   114,   127,    42,   127,   117,    -1,    48,    -1,
      51,    -1,   116,   118,    -1,    -1,    44,   126,    -1,    56,
      -1,     6,    -1,    13,   126,   131,   126,   120,    18,    -1,
      31,   126,    14,   127,    -1,    -1,    -1,    30,   126,    15,
     126,   121,    14,   122,   127,    -1,    30,   126,    15,   127,
      -1,    20,   124,    21,    69,    -1,    21,    69,    -1,    -1,
     124,   125,    -1,    20,    -1,    70,    -1,    19,    -1,    -1,
      19,    -1,     3,    -1,     4,    -1,     3,    -1,     3,    -1,
      14,    -1,    15,    -1,     3,    -1,    17,    -1,     3,    -1,
       3,    -1,    16,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    70,    70,    71,    75,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    83,    84,    85,    86,    89,    89,
      90,    90,    91,    92,    93,    96,    96,    97,    98,    99,
     100,   101,   102,   103,   103,   104,   104,   105,   105,   105,
     105,   105,   106,   109,   109,   110,   111,   112,   112,   113,
     113,   114,   115,   116,   116,   117,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   131,   132,
     133,   134,   134,   135,   136,   137,   140,   140,   141,   142,
     142,   143,   143,   143,   146,   147,   147,   148,   149,   149,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     158,   159,   160,   160,   161,   162,   162,   163,   164,   165,
     166,   166,   167,   168,   171,   172,   173,   173,   173,   174,
     178,   179,   180,   180,   181,   181,   184,   185,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "XMLTOK_NAME", "XMLTOK_ENCNAME", 
  "XMLTOK_ENCVERS", "XMLTOK_ATTVAL", "XMLTOK_TAGDOC", "XMLTOK_TAGENC", 
  "XMLTOK_TAGENT", "XMLTOK_TAGPI", "XMLTOK_TAGELEM", "XMLTOK_TAGATT", 
  "XMLTOK_TAGNOT", "XMLTOK_SYSLIT", "XMLTOK_PUBLIT", "XMLTOK_ENTVAL", 
  "XMLTOK_PICONTENT", "XMLTOK_CLOSE", "XMLTOK_SPACE", "XMLTOK_CONTENT", 
  "XMLTOK_CONTENT_END", "XMLTOK_ENCODING", "XMLTOK_STANDALONE", 
  "XMLTOK_YES", "XMLTOK_NO", "XMLTOK_CHR_PERCENT", "XMLTOK_VERSION", 
  "XMLTOK_NDATA", "XMLTOK_EQ", "XMLTOK_STR_PUBLIC", "XMLTOK_STR_SYSTEM", 
  "XMLTOK_DTD_OPEN", "XMLTOK_DTD_CLOSE", "XMLTOK_STR_EMPTY", 
  "XMLTOK_STR_ANY", "XMLTOK_STR_PCDATA", "XMLTOK_CHR_OPBR", 
  "XMLTOK_CHR_CLBR", "XMLTOK_CHR_ASTX", "XMLTOK_CHR_PLUS", 
  "XMLTOK_CHR_QUMK", "XMLTOK_CHR_PIPE", "XMLTOK_CHR_COMA", 
  "XMLTOK_STR_FIXED", "XMLTOK_STR_NOTATION", "XMLTOK_STR_ENTITY", 
  "XMLTOK_STR_ENTITIES", "XMLTOK_STR_REQUIRED", "XMLTOK_STR_NMTOKEN", 
  "XMLTOK_STR_CDATA", "XMLTOK_STR_IMPLIED", "XMLTOK_STR_NMTOKENS", 
  "XMLTOK_STR_ID", "XMLTOK_STR_IDREF", "XMLTOK_STR_IDREFS", 
  "XMLTOK_NMTOKEN", "$accept", "xml", "prolog", "XMLDeclO", "XMLDecl", 
  "XMLDecl2", "EncodingDecl", "SDDecl", "YesOrNo", "VersionInfo", "Eq", 
  "VersionStr", "MiscL", "Misc", "PI", "PITarget", "doctypeMiscO", 
  "doctypedecl", "docExtIDIntO", "intSubsetDecl", "intSubsetL", 
  "intSubset", "markupdecl", "DeclSep", "EntityDecl", "GEDecl", "PEDecl", 
  "EntityDef", "PEDef", "ExternalID", "NDataDeclO", "NDataDecl", 
  "elementdecl", "contentspec", "children", "cp", "choice", 
  "choiceNextL1", "choiceNext", "seq", "seqNextL", "seqNext", "Mixed", 
  "MixedPipeL1", "MixedPipe", "childReCharO", "childReChar", 
  "AttlistDecl", "AttDefL", "AttDef", "AttType", "StringType", 
  "TokenizedType", "EnumeratedType", "NotationType", "NotationNameL", 
  "Enumeration", "EnumNmtokenL", "DefaultDecl", "DefaultFixedO", 
  "Nmtoken", "AttValue", "NotationDecl", "ExtOrPubID", "@1", "@2", 
  "content", "contentL", "contents", "SS", "SO", "Name", "EncName", 
  "DocName", "NotationName", "SystemLiteral", "PubidLiteral", "PIName", 
  "PIContent", "GEName", "PEName", "EntityValue", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    57,    58,    59,    60,    60,    61,    62,    62,    62,
      62,    63,    64,    65,    65,    66,    67,    68,    69,    69,
      70,    70,    71,    72,    72,    73,    73,    74,    75,    75,
      75,    75,    76,    77,    77,    78,    78,    79,    79,    79,
      79,    79,    80,    81,    81,    82,    83,    84,    84,    85,
      85,    86,    86,    87,    87,    88,    89,    90,    90,    90,
      90,    91,    91,    92,    92,    92,    93,    94,    94,    95,
      96,    97,    97,    98,    99,    99,   100,   100,   101,   102,
     102,   103,   103,   103,   104,   105,   105,   106,   107,   107,
     107,   108,   109,   109,   109,   109,   109,   109,   109,   110,
     110,   111,   112,   112,   113,   114,   114,   115,   115,   115,
     116,   116,   117,   118,   119,   120,   121,   122,   120,   120,
     123,   123,   124,   124,   125,   125,   126,   127,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     3,     0,     1,     3,     2,     4,     6,
       4,     3,     3,     1,     1,     4,     3,     1,     0,     2,
       1,     1,     4,     0,     2,     0,     2,     5,     0,     2,
       4,     2,     4,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     6,     9,     2,     2,     1,
       1,     3,     5,     1,     2,     4,     7,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     7,     1,     3,     3,
       5,     1,     3,     3,     8,     5,     1,     3,     3,     0,
       1,     1,     1,     1,     6,     0,     2,     6,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     7,     1,     5,     5,     1,     5,     1,     1,     2,
       0,     2,     1,     1,     6,     4,     0,     0,     8,     4,
       4,     2,     0,     2,     1,     1,     1,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,    18,     5,   126,   127,     0,     1,
     122,    18,     2,    25,   126,     6,     0,     0,   127,     0,
     121,     0,     0,    19,    20,     3,    18,    21,   127,   127,
     127,   127,     7,   128,     0,     0,   124,    18,   125,   123,
       0,   135,    23,    26,     0,     0,     0,     0,     0,    17,
      15,   127,   120,   131,    28,     0,     0,   130,    11,    13,
      14,    12,   127,     8,    10,    16,     0,     0,     0,    22,
     136,    24,     0,    27,     0,     0,    29,    33,    31,     9,
       0,     0,     0,     0,   134,     0,   133,    51,    30,     0,
      41,     0,     0,     0,   127,    34,    35,    36,    39,    43,
      44,    37,    38,    40,    42,     0,     0,     0,     0,     0,
      32,    52,   137,     0,     0,   129,     0,    85,   132,     0,
       0,     0,     0,   127,     0,   138,     0,   139,     0,   127,
     127,    57,    58,   127,   127,    60,    79,    79,    59,    86,
       0,     0,     0,     0,     0,     0,    45,    48,   127,     0,
      53,    47,     0,     0,    82,    83,    81,    61,    80,    62,
       0,    84,     0,     0,   114,   127,    50,    49,    54,     0,
     127,   127,   127,    79,    79,    79,    56,     0,   127,   127,
       0,     0,     0,     0,     0,    71,    64,    65,    63,   127,
       0,    95,    96,    97,    91,    98,    92,    93,    94,     0,
      88,    89,    90,    99,   100,   116,   119,   115,    46,    55,
      75,   127,   127,    76,    70,   127,   127,   127,   127,    67,
       0,     0,   110,     0,     0,     0,     0,    72,     0,     0,
     112,   127,   105,   127,     0,   107,   108,    87,     0,   117,
      78,     0,    77,    73,    69,    66,    68,     0,     0,   111,
     113,   109,   127,    74,   104,   127,   127,   102,   118,     0,
       0,   106,   101,   127,     0,   103
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     2,     3,     4,     5,    15,    30,    31,    61,     7,
      34,    50,    13,    23,    24,    55,    25,    26,    66,    78,
      83,    95,    96,    97,    98,    99,   100,   128,   165,    76,
     147,   148,   101,   134,   135,   172,   173,   218,   219,   174,
     184,   216,   138,   212,   213,   157,   158,   102,   123,   139,
     199,   200,   201,   202,   203,   256,   204,   231,   237,   238,
     232,   251,   103,   144,   223,   252,    12,    19,    39,    27,
      35,   175,    58,    54,   119,    87,    85,    42,    71,   114,
     126,   130
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -137
static const short yypact[] =
{
      24,     7,    35,    78,  -137,  -137,  -137,    31,    38,  -137,
    -137,  -137,  -137,    41,    -2,  -137,    81,    55,    64,    43,
      -4,     7,    93,  -137,  -137,  -137,  -137,  -137,    64,    64,
      31,    64,  -137,  -137,   104,    83,  -137,  -137,  -137,  -137,
     110,  -137,     7,    -4,   111,    76,    94,   100,   101,  -137,
    -137,    64,    -4,  -137,    -5,   103,   107,  -137,  -137,  -137,
    -137,  -137,    64,  -137,  -137,  -137,   112,    75,    96,  -137,
    -137,  -137,   113,  -137,     7,     7,     2,  -137,  -137,  -137,
     114,   118,    96,     0,  -137,     7,  -137,  -137,  -137,     7,
    -137,     7,     7,     7,    64,  -137,  -137,  -137,  -137,  -137,
    -137,  -137,  -137,  -137,  -137,   118,    17,   122,   122,   130,
    -137,  -137,  -137,     7,     7,  -137,     7,  -137,  -137,     7,
     133,    15,    45,    31,    77,  -137,     7,  -137,   119,    31,
      64,  -137,  -137,    64,    64,  -137,    18,    18,  -137,  -137,
     122,   120,     7,     7,   121,    15,  -137,  -137,    64,   126,
    -137,  -137,     4,   124,  -137,  -137,  -137,  -137,  -137,  -137,
       7,  -137,   125,   132,  -137,    64,  -137,  -137,  -137,     7,
      64,    64,    64,    18,    18,    18,  -137,    39,    31,    64,
     129,   122,    14,     5,    11,   106,  -137,  -137,  -137,    64,
       7,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,     7,
    -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
    -137,    64,    64,  -137,  -137,    64,    64,    64,    64,  -137,
      99,   131,   -26,   136,   122,    28,     5,  -137,     5,    30,
    -137,    64,  -137,    64,     7,  -137,  -137,  -137,   150,  -137,
    -137,   123,  -137,  -137,  -137,  -137,  -137,    33,   122,  -137,
    -137,  -137,    64,  -137,  -137,    64,    64,  -137,  -137,    99,
      36,  -137,  -137,    64,   122,  -137
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -137,  -137,  -137,  -137,  -137,  -137,  -137,   115,  -137,  -137,
      82,  -137,    -9,   138,  -137,  -137,  -137,  -137,  -137,    84,
    -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -116,
    -137,  -137,  -137,  -137,  -137,  -131,    37,  -137,   -62,    47,
    -137,  -137,  -137,  -137,   -55,  -136,  -137,  -137,  -137,  -137,
    -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
     -84,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,    60,
      -7,  -104,  -137,  -137,  -137,    72,  -137,  -137,  -137,  -137,
    -137,    40
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -129
static const short yytable[] =
{
      17,   159,    20,   116,   117,   129,    22,   115,   115,    89,
      90,    91,    92,    93,    14,     6,  -128,    43,   234,     6,
     112,    33,   235,    47,    48,   236,     6,  -127,    52,   166,
    -128,   127,     1,    94,  -127,     9,   160,   186,   187,   188,
     170,   171,   171,   113,    65,    74,    75,    68,    21,   214,
      14,    22,   210,    22,   215,    72,   211,   154,   155,   156,
       6,     8,     6,    36,    37,    18,   241,    16,   245,    82,
     211,   254,   217,    32,   262,   255,   189,   209,   263,   131,
     132,    40,   133,    33,   190,   191,   192,   110,   193,   194,
      46,   195,   196,   197,   198,   243,    41,   244,    10,    11,
      59,    60,    56,    28,    29,    74,    75,   142,   143,    49,
      44,    45,    51,    53,    67,    57,   141,    29,    63,    64,
     240,    69,   150,   151,    70,   115,   152,   153,    77,    84,
      73,    79,    86,   118,    80,    81,   125,   146,   161,   164,
     178,   168,   176,   104,   257,   105,   179,   208,   217,   106,
     239,   107,   108,   109,   169,   230,   250,    38,   180,   136,
     265,    62,   253,   182,   183,   185,    88,   246,   233,   137,
     242,   206,   207,   120,   121,   261,   122,   111,     0,   124,
       0,     0,   220,   140,     0,   167,   145,     0,     0,   149,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   162,   163,   224,   225,     0,     0,   226,   227,
     228,   229,     0,     0,     0,     0,     0,     0,     0,     0,
     177,     0,     0,     0,   247,     0,   248,     0,     0,   181,
       0,     0,     0,     0,     0,     0,     0,     0,   205,     0,
       0,     0,     0,     0,     0,   258,     0,     0,   259,   260,
     221,     0,     0,     0,     0,     0,   264,     0,     0,   222,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   249
};

static const short yycheck[] =
{
       7,   137,    11,   107,   108,   121,    10,     3,     3,     9,
      10,    11,    12,    13,    19,    19,    18,    26,    44,    19,
       3,    19,    48,    30,    31,    51,    19,    32,    37,   145,
      32,    16,     8,    33,    32,     0,   140,   173,   174,   175,
      36,    37,    37,    26,    51,    30,    31,    54,     7,    38,
      19,    10,    38,    10,    43,    62,    42,    39,    40,    41,
      19,     1,    19,    20,    21,    27,    38,     7,    38,    76,
      42,    38,    42,    18,    38,    42,    37,   181,    42,    34,
      35,    21,    37,    19,    45,    46,    47,    94,    49,    50,
      30,    52,    53,    54,    55,   226,     3,   228,    20,    21,
      24,    25,    42,    22,    23,    30,    31,    30,    31,     5,
      28,    29,    29,     3,    54,     4,   123,    23,    18,    18,
     224,    18,   129,   130,    17,     3,   133,   134,    32,    15,
      18,    18,    14,     3,    74,    75,     3,    18,    18,    18,
      15,   148,    18,    83,   248,    85,    14,    18,    42,    89,
      14,    91,    92,    93,    28,    56,     6,    19,   165,   122,
     264,    46,    39,   170,   171,   172,    82,   229,    37,   122,
     225,   178,   179,   113,   114,   259,   116,   105,    -1,   119,
      -1,    -1,   189,   123,    -1,   145,   126,    -1,    -1,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   142,   143,   211,   212,    -1,    -1,   215,   216,
     217,   218,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     160,    -1,    -1,    -1,   231,    -1,   233,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   178,    -1,
      -1,    -1,    -1,    -1,    -1,   252,    -1,    -1,   255,   256,
     190,    -1,    -1,    -1,    -1,    -1,   263,    -1,    -1,   199,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   234
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     8,    58,    59,    60,    61,    19,    66,   126,     0,
      20,    21,   123,    69,    19,    62,   126,   127,    27,   124,
      69,     7,    10,    70,    71,    73,    74,   126,    22,    23,
      63,    64,    18,    19,    67,   127,    20,    21,    70,   125,
     126,     3,   134,    69,    67,    67,   126,   127,   127,     5,
      68,    29,    69,     3,   130,    72,   126,     4,   129,    24,
      25,    65,    64,    18,    18,   127,    75,   126,   127,    18,
      17,   135,   127,    18,    30,    31,    86,    32,    76,    18,
     126,   126,   127,    77,    15,   133,    14,   132,    76,     9,
      10,    11,    12,    13,    33,    78,    79,    80,    81,    82,
      83,    89,   104,   119,   126,   126,   126,   126,   126,   126,
     127,   132,     3,    26,   136,     3,   128,   128,     3,   131,
     126,   126,   126,   105,   126,     3,   137,    16,    84,    86,
     138,    34,    35,    37,    90,    91,    93,    96,    99,   106,
     126,   127,    30,    31,   120,   126,    18,    87,    88,   126,
     127,   127,   127,   127,    39,    40,    41,   102,   103,   102,
     128,    18,   126,   126,    18,    85,    86,   138,   127,    28,
      36,    37,    92,    93,    96,   128,    18,   126,    15,    14,
     127,   126,   127,   127,    97,   127,   102,   102,   102,    37,
      45,    46,    47,    49,    50,    52,    53,    54,    55,   107,
     108,   109,   110,   111,   113,   126,   127,   127,    18,   128,
      38,    42,   100,   101,    38,    43,    98,    42,    94,    95,
     127,   126,   126,   121,   127,   127,   127,   127,   127,   127,
      56,   114,   117,    37,    44,    48,    51,   115,   116,    14,
     128,    38,   101,    92,    92,    38,    95,   127,   127,   126,
       6,   118,   122,    39,    38,    42,   112,   128,   127,   127,
     127,   117,    38,    42,   127,   128
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 23:
#line 92 "xml_prolog.yy"
    { XmltvTags_PiContent(NULL); }
    break;

  case 30:
#line 100 "xml_prolog.yy"
    {XmltvTags_DocIntDtdClose();}
    break;

  case 31:
#line 101 "xml_prolog.yy"
    {XmltvTags_DocIntDtdClose();}
    break;

  case 115:
#line 172 "xml_prolog.yy"
    {XmltvTags_Notation(1, yyvsp[-1].str);}
    break;

  case 116:
#line 173 "xml_prolog.yy"
    {XmltvTags_Notation(2, yyvsp[-1].str);}
    break;

  case 117:
#line 173 "xml_prolog.yy"
    {XmltvTags_Notation(4, yyvsp[0].str);}
    break;

  case 119:
#line 174 "xml_prolog.yy"
    {XmltvTags_Notation(3, yyvsp[-1].str);}
    break;

  case 130:
#line 187 "xml_prolog.yy"
    { /* TODO */ }
    break;

  case 131:
#line 188 "xml_prolog.yy"
    { XmltvTags_DocType(yyvsp[0].str); }
    break;

  case 132:
#line 189 "xml_prolog.yy"
    { XmltvTags_Notation(0, yyvsp[0].str); }
    break;

  case 135:
#line 192 "xml_prolog.yy"
    { XmltvTags_PiTarget(yyvsp[0].str); }
    break;

  case 136:
#line 193 "xml_prolog.yy"
    { XmltvTags_PiContent(yyvsp[0].str); }
    break;

  case 137:
#line 194 "xml_prolog.yy"
    { XmlScan_EntityDefName(yyvsp[0].str, 0); }
    break;

  case 138:
#line 195 "xml_prolog.yy"
    { XmlScan_EntityDefName(yyvsp[0].str, 1); }
    break;

  case 139:
#line 196 "xml_prolog.yy"
    { XmlScan_EntityDefValue(yyvsp[0].str); }
    break;


    }

/* Line 991 of yacc.c.  */
#line 1444 "xml_prolog.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 198 "xml_prolog.yy"


void yyerror( const char * p_msg )
{
   Xmltv_SyntaxError("Prolog parse error", p_msg);
}



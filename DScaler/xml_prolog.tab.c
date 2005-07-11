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
     XMLTOK_NMTOKEN = 262,
     XMLTOK_TAGDOC = 263,
     XMLTOK_TAGENC = 264,
     XMLTOK_TAGENT = 265,
     XMLTOK_TAGPI = 266,
     XMLTOK_TAGELEM = 267,
     XMLTOK_TAGATT = 268,
     XMLTOK_TAGNOT = 269,
     XMLTOK_SYSLIT = 270,
     XMLTOK_PUBLIT = 271,
     XMLTOK_ENTVAL = 272,
     XMLTOK_PICONTENT = 273,
     XMLTOK_CLOSE = 274,
     XMLTOK_SPACE = 275,
     XMLTOK_CONTENT = 276,
     XMLTOK_CONTENT_END = 277,
     XMLTOK_ENCODING = 278,
     XMLTOK_STANDALONE = 279,
     XMLTOK_YES = 280,
     XMLTOK_NO = 281,
     XMLTOK_CHR_PERCENT = 282,
     XMLTOK_VERSION = 283,
     XMLTOK_NDATA = 284,
     XMLTOK_EQ = 285,
     XMLTOK_STR_PUBLIC = 286,
     XMLTOK_STR_SYSTEM = 287,
     XMLTOK_DTD_OPEN = 288,
     XMLTOK_DTD_CLOSE = 289,
     XMLTOK_STR_EMPTY = 290,
     XMLTOK_STR_ANY = 291,
     XMLTOK_STR_PCDATA = 292,
     XMLTOK_CHR_OPBR = 293,
     XMLTOK_CHR_CLBR = 294,
     XMLTOK_CHR_ASTX = 295,
     XMLTOK_CHR_PLUS = 296,
     XMLTOK_CHR_QUMK = 297,
     XMLTOK_CHR_PIPE = 298,
     XMLTOK_CHR_COMA = 299,
     XMLTOK_STR_FIXED = 300,
     XMLTOK_STR_NOTATION = 301,
     XMLTOK_STR_ENTITY = 302,
     XMLTOK_STR_ENTITIES = 303,
     XMLTOK_STR_REQUIRED = 304,
     XMLTOK_STR_NMTOKEN = 305,
     XMLTOK_STR_CDATA = 306,
     XMLTOK_STR_IMPLIED = 307,
     XMLTOK_STR_NMTOKENS = 308,
     XMLTOK_STR_ID = 309,
     XMLTOK_STR_IDREF = 310,
     XMLTOK_STR_IDREFS = 311,
     XMLTOK_ERR_CLOSE = 312
   };
#endif
#define XMLTOK_NAME 258
#define XMLTOK_ENCNAME 259
#define XMLTOK_ENCVERS 260
#define XMLTOK_ATTVAL 261
#define XMLTOK_NMTOKEN 262
#define XMLTOK_TAGDOC 263
#define XMLTOK_TAGENC 264
#define XMLTOK_TAGENT 265
#define XMLTOK_TAGPI 266
#define XMLTOK_TAGELEM 267
#define XMLTOK_TAGATT 268
#define XMLTOK_TAGNOT 269
#define XMLTOK_SYSLIT 270
#define XMLTOK_PUBLIT 271
#define XMLTOK_ENTVAL 272
#define XMLTOK_PICONTENT 273
#define XMLTOK_CLOSE 274
#define XMLTOK_SPACE 275
#define XMLTOK_CONTENT 276
#define XMLTOK_CONTENT_END 277
#define XMLTOK_ENCODING 278
#define XMLTOK_STANDALONE 279
#define XMLTOK_YES 280
#define XMLTOK_NO 281
#define XMLTOK_CHR_PERCENT 282
#define XMLTOK_VERSION 283
#define XMLTOK_NDATA 284
#define XMLTOK_EQ 285
#define XMLTOK_STR_PUBLIC 286
#define XMLTOK_STR_SYSTEM 287
#define XMLTOK_DTD_OPEN 288
#define XMLTOK_DTD_CLOSE 289
#define XMLTOK_STR_EMPTY 290
#define XMLTOK_STR_ANY 291
#define XMLTOK_STR_PCDATA 292
#define XMLTOK_CHR_OPBR 293
#define XMLTOK_CHR_CLBR 294
#define XMLTOK_CHR_ASTX 295
#define XMLTOK_CHR_PLUS 296
#define XMLTOK_CHR_QUMK 297
#define XMLTOK_CHR_PIPE 298
#define XMLTOK_CHR_COMA 299
#define XMLTOK_STR_FIXED 300
#define XMLTOK_STR_NOTATION 301
#define XMLTOK_STR_ENTITY 302
#define XMLTOK_STR_ENTITIES 303
#define XMLTOK_STR_REQUIRED 304
#define XMLTOK_STR_NMTOKEN 305
#define XMLTOK_STR_CDATA 306
#define XMLTOK_STR_IMPLIED 307
#define XMLTOK_STR_NMTOKENS 308
#define XMLTOK_STR_ID 309
#define XMLTOK_STR_IDREF 310
#define XMLTOK_STR_IDREFS 311
#define XMLTOK_ERR_CLOSE 312




/* Copy the first part of user declarations.  */
#line 31 "xml_prolog.yy"

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
#line 50 "xml_prolog.yy"
typedef union YYSTYPE { char * str; } YYSTYPE;
/* Line 191 of yacc.c.  */
#line 210 "xml_prolog.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 222 "xml_prolog.tab.c"

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
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   302

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  85
/* YYNRULES -- Number of rules. */
#define YYNRULES  148
/* YYNRULES -- Number of states. */
#define YYNSTATES  276

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

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
      55,    56,    57
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     6,    10,    11,    13,    17,    20,    23,
      28,    35,    40,    44,    48,    50,    52,    57,    61,    62,
      65,    67,    69,    71,    76,    77,    80,    81,    84,    90,
      91,    94,    99,   102,   107,   108,   111,   113,   115,   117,
     119,   121,   123,   125,   127,   129,   133,   135,   137,   139,
     140,   143,   145,   147,   154,   164,   167,   170,   172,   174,
     178,   184,   186,   189,   194,   202,   204,   206,   208,   210,
     213,   216,   219,   222,   225,   233,   235,   239,   243,   249,
     251,   255,   259,   268,   274,   276,   280,   284,   285,   287,
     289,   291,   293,   300,   301,   304,   311,   313,   315,   317,
     319,   321,   323,   325,   327,   329,   331,   333,   335,   337,
     345,   347,   353,   359,   361,   367,   369,   371,   374,   375,
     378,   385,   390,   391,   392,   401,   406,   411,   414,   415,
     418,   420,   422,   424,   425,   427,   429,   431,   433,   435,
     437,   439,   441,   443,   445,   447,   449,   451,   453
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      59,     0,    -1,    60,   124,    -1,    61,    69,    73,    -1,
      -1,    62,    -1,     9,    67,    63,    -1,     9,    81,    -1,
     128,    19,    -1,   127,    64,   128,    19,    -1,   127,    64,
     127,    65,   128,    19,    -1,   127,    65,   128,    19,    -1,
      23,    68,   129,    -1,    24,    68,    66,    -1,    25,    -1,
      26,    -1,   127,    28,    68,   130,    -1,   128,    30,   128,
      -1,    -1,    69,    70,    -1,    71,    -1,   127,    -1,    81,
      -1,    11,   137,    72,    19,    -1,    -1,   127,   138,    -1,
      -1,    74,    69,    -1,     8,   127,   131,    75,    19,    -1,
      -1,   127,    89,    -1,   127,    89,   128,    76,    -1,   128,
      76,    -1,    33,    77,    34,   128,    -1,    -1,    77,    78,
      -1,    79,    -1,    80,    -1,    92,    -1,   107,    -1,    84,
      -1,   120,    -1,    11,    -1,    81,    -1,   127,    -1,     1,
      83,    82,    -1,    57,    -1,    19,    -1,    57,    -1,    -1,
      83,     1,    -1,    85,    -1,    86,    -1,    10,   127,   132,
     127,    87,    19,    -1,    10,   127,    27,   127,   133,   127,
      88,   128,    19,    -1,   134,   128,    -1,    89,    90,    -1,
     134,    -1,    89,    -1,    32,   127,   141,    -1,    31,   127,
     136,   127,   141,    -1,   128,    -1,    91,   128,    -1,   127,
      29,   127,   139,    -1,    12,   127,   139,   127,    93,   128,
      19,    -1,    35,    -1,    36,    -1,   102,    -1,    94,    -1,
      96,   105,    -1,    99,   105,    -1,   139,   105,    -1,    96,
     105,    -1,    99,   105,    -1,    38,   128,    95,   128,    97,
     128,    39,    -1,    98,    -1,    97,   128,    98,    -1,    43,
     128,    95,    -1,    38,   128,    95,   100,    39,    -1,   128,
      -1,   100,   101,   128,    -1,    44,   128,    95,    -1,    38,
     128,    37,   128,   103,   128,    39,    40,    -1,    38,   128,
      37,   128,    39,    -1,   104,    -1,   103,   128,   104,    -1,
      43,   128,   139,    -1,    -1,   106,    -1,    42,    -1,    40,
      -1,    41,    -1,    13,   127,   139,   108,   128,    19,    -1,
      -1,   108,   109,    -1,   127,   139,   127,   110,   127,   118,
      -1,   111,    -1,   112,    -1,   113,    -1,    51,    -1,    54,
      -1,    55,    -1,    56,    -1,    47,    -1,    48,    -1,    50,
      -1,    53,    -1,   114,    -1,   116,    -1,    46,   127,    38,
     128,   115,   128,    39,    -1,   139,    -1,   115,   128,    43,
     128,   139,    -1,    38,   128,   117,   128,    39,    -1,   140,
      -1,   117,   128,    43,   128,   140,    -1,    49,    -1,    52,
      -1,   119,   142,    -1,    -1,    45,   127,    -1,    14,   127,
     135,   127,   121,    19,    -1,    32,   127,    15,   128,    -1,
      -1,    -1,    31,   127,    16,   127,   122,    15,   123,   128,
      -1,    31,   127,    16,   128,    -1,    21,   125,    22,    69,
      -1,    22,    69,    -1,    -1,   125,   126,    -1,    21,    -1,
      70,    -1,    20,    -1,    -1,    20,    -1,     4,    -1,     5,
      -1,     3,    -1,     3,    -1,     3,    -1,    17,    -1,     3,
      -1,    16,    -1,     3,    -1,    18,    -1,     3,    -1,     7,
      -1,    15,    -1,     6,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    72,    72,    73,    77,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    86,    87,    88,    91,    91,
      92,    92,    92,    93,    94,    95,    98,    98,    99,   100,
     101,   102,   103,   104,   105,   105,   106,   106,   107,   108,
     109,   110,   111,   112,   114,   122,   123,   124,   124,   125,
     125,   128,   128,   129,   130,   131,   131,   132,   132,   133,
     134,   135,   135,   136,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   150,   151,   152,   153,
     153,   154,   155,   156,   159,   159,   160,   161,   161,   162,
     162,   162,   165,   166,   166,   167,   168,   168,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   177,   178,
     179,   179,   180,   181,   181,   182,   183,   184,   185,   185,
     188,   189,   190,   190,   190,   191,   195,   196,   197,   197,
     198,   198,   201,   202,   202,   203,   204,   205,   206,   207,
     208,   210,   211,   212,   213,   215,   216,   217,   218
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "XMLTOK_NAME", "XMLTOK_ENCNAME", 
  "XMLTOK_ENCVERS", "XMLTOK_ATTVAL", "XMLTOK_NMTOKEN", "XMLTOK_TAGDOC", 
  "XMLTOK_TAGENC", "XMLTOK_TAGENT", "XMLTOK_TAGPI", "XMLTOK_TAGELEM", 
  "XMLTOK_TAGATT", "XMLTOK_TAGNOT", "XMLTOK_SYSLIT", "XMLTOK_PUBLIT", 
  "XMLTOK_ENTVAL", "XMLTOK_PICONTENT", "XMLTOK_CLOSE", "XMLTOK_SPACE", 
  "XMLTOK_CONTENT", "XMLTOK_CONTENT_END", "XMLTOK_ENCODING", 
  "XMLTOK_STANDALONE", "XMLTOK_YES", "XMLTOK_NO", "XMLTOK_CHR_PERCENT", 
  "XMLTOK_VERSION", "XMLTOK_NDATA", "XMLTOK_EQ", "XMLTOK_STR_PUBLIC", 
  "XMLTOK_STR_SYSTEM", "XMLTOK_DTD_OPEN", "XMLTOK_DTD_CLOSE", 
  "XMLTOK_STR_EMPTY", "XMLTOK_STR_ANY", "XMLTOK_STR_PCDATA", 
  "XMLTOK_CHR_OPBR", "XMLTOK_CHR_CLBR", "XMLTOK_CHR_ASTX", 
  "XMLTOK_CHR_PLUS", "XMLTOK_CHR_QUMK", "XMLTOK_CHR_PIPE", 
  "XMLTOK_CHR_COMA", "XMLTOK_STR_FIXED", "XMLTOK_STR_NOTATION", 
  "XMLTOK_STR_ENTITY", "XMLTOK_STR_ENTITIES", "XMLTOK_STR_REQUIRED", 
  "XMLTOK_STR_NMTOKEN", "XMLTOK_STR_CDATA", "XMLTOK_STR_IMPLIED", 
  "XMLTOK_STR_NMTOKENS", "XMLTOK_STR_ID", "XMLTOK_STR_IDREF", 
  "XMLTOK_STR_IDREFS", "XMLTOK_ERR_CLOSE", "$accept", "xml", "prolog", 
  "XMLDeclO", "XMLDecl", "XMLDecl2", "EncodingDecl", "SDDecl", "YesOrNo", 
  "VersionInfo", "Eq", "MiscL", "Misc", "PI", "PITarget", "doctypeMiscO", 
  "doctypedecl", "docExtIDIntO", "intSubsetDecl", "intSubsetL", 
  "intSubset", "markupdecl", "DeclSep", "markupdeclErr", "markupdeclErrE", 
  "errorL", "EntityDecl", "GEDecl", "PEDecl", "EntityDef", "PEDef", 
  "ExternalID", "NDataDeclO", "NDataDecl", "elementdecl", "contentspec", 
  "children", "cp", "choice", "choiceNextL1", "choiceNext", "seq", 
  "seqNextL", "seqNext", "Mixed", "MixedPipeL1", "MixedPipe", 
  "childReCharO", "childReChar", "AttlistDecl", "AttDefL", "AttDef", 
  "AttType", "StringType", "TokenizedType", "EnumeratedType", 
  "NotationType", "NotationNameL", "Enumeration", "EnumNmtokenL", 
  "DefaultDecl", "DefaultFixedO", "NotationDecl", "ExtOrPubID", "@1", 
  "@2", "content", "contentL", "contents", "SS", "SO", "EncName", 
  "VersionStr", "DocName", "GEName", "PEName", "EntityValue", 
  "NotationName", "PubidLiteral", "PIName", "PIContent", "Name", 
  "Nmtoken", "SystemLiteral", "AttValue", 0
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
     305,   306,   307,   308,   309,   310,   311,   312
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    58,    59,    60,    61,    61,    62,    62,    63,    63,
      63,    63,    64,    65,    66,    66,    67,    68,    69,    69,
      70,    70,    70,    71,    72,    72,    73,    73,    74,    75,
      75,    75,    75,    76,    77,    77,    78,    78,    79,    79,
      79,    79,    79,    79,    80,    81,    81,    82,    82,    83,
      83,    84,    84,    85,    86,    87,    87,    88,    88,    89,
      89,    90,    90,    91,    92,    93,    93,    93,    93,    94,
      94,    95,    95,    95,    96,    97,    97,    98,    99,   100,
     100,   101,   102,   102,   103,   103,   104,   105,   105,   106,
     106,   106,   107,   108,   108,   109,   110,   110,   110,   111,
     112,   112,   112,   112,   112,   112,   112,   113,   113,   114,
     115,   115,   116,   117,   117,   118,   118,   118,   119,   119,
     120,   121,   122,   123,   121,   121,   124,   124,   125,   125,
     126,   126,   127,   128,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     3,     0,     1,     3,     2,     2,     4,
       6,     4,     3,     3,     1,     1,     4,     3,     0,     2,
       1,     1,     1,     4,     0,     2,     0,     2,     5,     0,
       2,     4,     2,     4,     0,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     0,
       2,     1,     1,     6,     9,     2,     2,     1,     1,     3,
       5,     1,     2,     4,     7,     1,     1,     1,     1,     2,
       2,     2,     2,     2,     7,     1,     3,     3,     5,     1,
       3,     3,     8,     5,     1,     3,     3,     0,     1,     1,
       1,     1,     6,     0,     2,     6,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     7,
       1,     5,     5,     1,     5,     1,     1,     2,     0,     2,
       6,     4,     0,     0,     8,     4,     4,     2,     0,     2,
       1,     1,     1,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     0,     0,    18,     5,    49,   132,    46,   133,
       7,     0,     1,   128,    18,     2,     0,     0,   132,     6,
       0,     0,   133,     0,     0,     0,     0,    19,    20,     3,
      18,    22,    21,    50,    47,    48,    45,   133,   133,   133,
     133,     8,   134,     0,     0,   130,    18,   131,   129,     0,
     143,    24,     0,     0,     0,     0,     0,     0,   136,    16,
     133,     0,   137,    29,     0,     0,   135,    12,    14,    15,
      13,   133,     9,    11,    17,     0,     0,     0,    23,   144,
      25,     0,    28,     0,     0,    30,    34,    32,    10,     0,
       0,     0,     0,   142,     0,   147,    59,    31,     0,    42,
       0,     0,     0,   133,    35,    36,    37,    43,    40,    51,
      52,    38,    39,    41,    44,     0,     0,     0,     0,     0,
      33,    60,   138,     0,     0,   145,     0,    93,   141,     0,
       0,     0,     0,   133,     0,   139,     0,   140,     0,   133,
     133,    65,    66,   133,   133,    68,    87,    87,    67,    94,
       0,     0,     0,     0,     0,     0,    53,    56,   133,     0,
      61,    55,     0,     0,    90,    91,    89,    69,    88,    70,
       0,    92,     0,     0,   120,   133,    58,    57,    62,     0,
     133,   133,   133,    87,    87,    87,    64,     0,   133,   133,
       0,     0,     0,     0,     0,    79,    72,    73,    71,   133,
       0,   103,   104,   105,    99,   106,   100,   101,   102,     0,
      96,    97,    98,   107,   108,   122,   125,   121,    54,    63,
      83,   133,   133,    84,    78,   133,   133,   133,   133,    75,
       0,     0,   118,     0,     0,     0,     0,    80,     0,     0,
     146,   133,   113,   133,     0,   115,   116,    95,     0,   123,
      86,     0,    85,    81,    77,    74,    76,     0,     0,   119,
     148,   117,   133,    82,   112,   133,   133,   110,   124,     0,
       0,   114,   109,   133,     0,   111
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     2,     3,     4,     5,    19,    39,    40,    70,     9,
      43,    16,    27,    28,    64,    29,    30,    75,    87,    92,
     104,   105,   106,    31,    36,    17,   108,   109,   110,   138,
     175,    85,   157,   158,   111,   144,   145,   182,   183,   228,
     229,   184,   194,   226,   148,   222,   223,   167,   168,   112,
     133,   149,   209,   210,   211,   212,   213,   266,   214,   241,
     247,   248,   113,   154,   233,   262,    15,    23,    48,    32,
      44,    67,    59,    63,   124,   136,   140,   129,    94,    51,
      80,   185,   242,    96,   261
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -224
static const short yypact[] =
{
      13,     1,    53,    20,  -224,  -224,  -224,  -224,  -224,    52,
    -224,    33,  -224,  -224,  -224,  -224,    80,     9,    44,  -224,
     102,    51,    64,    28,     3,    76,    95,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,    64,    64,    52,
      64,  -224,  -224,   114,    77,  -224,  -224,  -224,  -224,   120,
    -224,    76,    94,   123,   103,   112,   121,   126,  -224,  -224,
      64,     7,  -224,    45,   128,   130,  -224,  -224,  -224,  -224,
    -224,    64,  -224,  -224,  -224,   134,   101,   122,  -224,  -224,
    -224,   135,  -224,    76,    76,    49,  -224,  -224,  -224,   141,
     146,   122,    23,  -224,    76,  -224,  -224,  -224,    76,  -224,
      76,    76,    76,    64,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,   146,    17,   160,   160,   161,
    -224,  -224,  -224,    76,    76,  -224,    76,  -224,  -224,    76,
     162,    -6,    82,    52,   107,  -224,    76,  -224,   148,    52,
      64,  -224,  -224,    64,    64,  -224,    71,    71,  -224,  -224,
     160,   149,    76,    76,   150,    -6,  -224,  -224,    64,   145,
    -224,  -224,    37,   151,  -224,  -224,  -224,  -224,  -224,  -224,
      76,  -224,   159,   163,  -224,    64,  -224,  -224,  -224,    76,
      64,    64,    64,    71,    71,    71,  -224,   222,    52,    64,
     157,   160,    47,    14,    29,   140,  -224,  -224,  -224,    64,
      76,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,    76,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,    64,    64,  -224,  -224,    64,    64,    64,    64,  -224,
     170,   147,   -33,   171,   160,    50,    14,  -224,    14,    60,
    -224,    64,  -224,    64,    76,  -224,  -224,  -224,   182,  -224,
    -224,   152,  -224,  -224,  -224,  -224,  -224,    65,   160,  -224,
    -224,  -224,    64,  -224,  -224,    64,    64,  -224,  -224,   170,
      67,  -224,  -224,    64,   160,  -224
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -224,  -224,  -224,  -224,  -224,  -224,  -224,   138,  -224,  -224,
     106,    41,   166,  -224,  -224,  -224,  -224,  -224,   104,  -224,
    -224,  -224,  -224,     0,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,   -99,  -224,  -224,  -224,  -224,  -224,  -223,    66,  -224,
     -43,    68,  -224,  -224,  -224,  -224,   -36,  -138,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,
    -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,  -224,    58,
      -9,  -224,  -224,  -224,  -224,  -224,    46,  -224,  -224,  -224,
    -224,  -112,   -67,    88,  -224
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -135
static const short yytable[] =
{
      21,    10,     6,  -127,     6,   126,   127,  -126,     6,   169,
      33,   137,   244,   253,    26,   254,   245,   125,    26,   246,
     122,     7,     1,     7,     6,    83,    84,     7,    34,     6,
      56,    57,   139,    98,    99,   100,   101,   102,   170,    26,
     125,    13,    14,     7,   123,   196,   197,   198,     7,    45,
      46,    74,   181,    12,    77,    24,   176,   103,     8,    11,
       8,    22,    81,  -134,     8,    18,    35,    20,   224,    42,
      41,    52,    18,   225,   180,   181,    91,  -134,  -133,   219,
       8,     6,  -133,    49,    42,     8,   220,    61,    25,   251,
     221,    26,   107,   221,   120,     6,     7,    55,    50,   255,
       7,   -26,   -26,   227,   264,    26,   272,    60,   265,    65,
     273,   164,   165,   166,     7,   -27,   -27,   141,   142,    58,
     143,    76,   250,    62,   151,    37,    38,    66,    68,    69,
     160,   161,    83,    84,   162,   163,    38,     8,   152,   153,
      72,    89,    90,    53,    54,    73,   267,    78,    79,   178,
     114,     8,   115,    82,    88,    86,   116,    93,   117,   118,
     119,    95,   275,   125,   128,   135,   190,   156,   171,   174,
     186,   192,   193,   195,   179,   188,   218,   240,   189,   216,
     217,   130,   131,   227,   132,   243,   249,   134,   260,    47,
     230,   150,   263,    71,   155,    97,   256,   159,   146,   252,
     147,   177,   271,   121,     0,     0,     0,     0,     0,     0,
     172,   173,   234,   235,     0,     0,   236,   237,   238,   239,
       0,     0,     0,     0,     0,     0,     0,     0,   187,     0,
       0,     0,   257,     0,   258,     0,     0,   191,     0,     0,
       0,     0,     0,     0,     0,     0,   215,     0,     0,     0,
       0,     0,     0,   268,     0,     0,   269,   270,   231,     0,
     199,     0,     0,     0,   274,     0,     0,   232,   200,   201,
     202,     0,   203,   204,     0,   205,   206,   207,   208,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   259
};

static const short yycheck[] =
{
       9,     1,     1,     0,     1,   117,   118,     0,     1,   147,
       1,    17,    45,   236,    11,   238,    49,     3,    11,    52,
       3,    20,     9,    20,     1,    31,    32,    20,    19,     1,
      39,    40,   131,    10,    11,    12,    13,    14,   150,    11,
       3,    21,    22,    20,    27,   183,   184,   185,    20,    21,
      22,    60,    38,     0,    63,    14,   155,    34,    57,     1,
      57,    28,    71,    19,    57,    20,    57,     9,    39,    20,
      19,    30,    20,    44,    37,    38,    85,    33,    33,   191,
      57,     1,    33,    25,    20,    57,    39,    46,     8,    39,
      43,    11,    92,    43,   103,     1,    20,    39,     3,    39,
      20,    21,    22,    43,    39,    11,    39,    30,    43,    51,
      43,    40,    41,    42,    20,    21,    22,    35,    36,     5,
      38,    63,   234,     3,   133,    23,    24,     4,    25,    26,
     139,   140,    31,    32,   143,   144,    24,    57,    31,    32,
      19,    83,    84,    37,    38,    19,   258,    19,    18,   158,
      92,    57,    94,    19,    19,    33,    98,    16,   100,   101,
     102,    15,   274,     3,     3,     3,   175,    19,    19,    19,
      19,   180,   181,   182,    29,    16,    19,     7,    15,   188,
     189,   123,   124,    43,   126,    38,    15,   129,     6,    23,
     199,   133,    40,    55,   136,    91,   239,   139,   132,   235,
     132,   155,   269,   115,    -1,    -1,    -1,    -1,    -1,    -1,
     152,   153,   221,   222,    -1,    -1,   225,   226,   227,   228,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,
      -1,    -1,   241,    -1,   243,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   188,    -1,    -1,    -1,
      -1,    -1,    -1,   262,    -1,    -1,   265,   266,   200,    -1,
      38,    -1,    -1,    -1,   273,    -1,    -1,   209,    46,    47,
      48,    -1,    50,    51,    -1,    53,    54,    55,    56,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   244
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     9,    59,    60,    61,    62,     1,    20,    57,    67,
      81,   127,     0,    21,    22,   124,    69,    83,    20,    63,
     127,   128,    28,   125,    69,     8,    11,    70,    71,    73,
      74,    81,   127,     1,    19,    57,    82,    23,    24,    64,
      65,    19,    20,    68,   128,    21,    22,    70,   126,   127,
       3,   137,    69,    68,    68,   127,   128,   128,     5,   130,
      30,    69,     3,   131,    72,   127,     4,   129,    25,    26,
      66,    65,    19,    19,   128,    75,   127,   128,    19,    18,
     138,   128,    19,    31,    32,    89,    33,    76,    19,   127,
     127,   128,    77,    16,   136,    15,   141,    76,    10,    11,
      12,    13,    14,    34,    78,    79,    80,    81,    84,    85,
      86,    92,   107,   120,   127,   127,   127,   127,   127,   127,
     128,   141,     3,    27,   132,     3,   139,   139,     3,   135,
     127,   127,   127,   108,   127,     3,   133,    17,    87,    89,
     134,    35,    36,    38,    93,    94,    96,    99,   102,   109,
     127,   128,    31,    32,   121,   127,    19,    90,    91,   127,
     128,   128,   128,   128,    40,    41,    42,   105,   106,   105,
     139,    19,   127,   127,    19,    88,    89,   134,   128,    29,
      37,    38,    95,    96,    99,   139,    19,   127,    16,    15,
     128,   127,   128,   128,   100,   128,   105,   105,   105,    38,
      46,    47,    48,    50,    51,    53,    54,    55,    56,   110,
     111,   112,   113,   114,   116,   127,   128,   128,    19,   139,
      39,    43,   103,   104,    39,    44,   101,    43,    97,    98,
     128,   127,   127,   122,   128,   128,   128,   128,   128,   128,
       7,   117,   140,    38,    45,    49,    52,   118,   119,    15,
     139,    39,   104,    95,    95,    39,    98,   128,   128,   127,
       6,   142,   123,    40,    39,    43,   115,   139,   128,   128,
     128,   140,    39,    43,   128,   139
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
        case 24:
#line 94 "xml_prolog.yy"
    { XmltvTags_PiContent(NULL); }
    break;

  case 31:
#line 102 "xml_prolog.yy"
    {XmltvTags_DocIntDtdClose();}
    break;

  case 32:
#line 103 "xml_prolog.yy"
    {XmltvTags_DocIntDtdClose();}
    break;

  case 45:
#line 122 "xml_prolog.yy"
    {yyerrok;}
    break;

  case 46:
#line 123 "xml_prolog.yy"
    {yyerrok;}
    break;

  case 121:
#line 189 "xml_prolog.yy"
    {XmltvTags_Notation(1, yyvsp[-1].str);}
    break;

  case 122:
#line 190 "xml_prolog.yy"
    {XmltvTags_Notation(2, yyvsp[-1].str);}
    break;

  case 123:
#line 190 "xml_prolog.yy"
    {XmltvTags_Notation(4, yyvsp[0].str);}
    break;

  case 125:
#line 191 "xml_prolog.yy"
    {XmltvTags_Notation(3, yyvsp[-1].str);}
    break;

  case 135:
#line 203 "xml_prolog.yy"
    { XmltvTags_Encoding(yyvsp[0].str); }
    break;

  case 136:
#line 204 "xml_prolog.yy"
    { XmltvTags_XmlVersion(yyvsp[0].str); }
    break;

  case 137:
#line 205 "xml_prolog.yy"
    { XmltvTags_DocType(yyvsp[0].str); }
    break;

  case 138:
#line 206 "xml_prolog.yy"
    { XmlScan_EntityDefName(yyvsp[0].str, 0); XmltvTags_CheckName(yyvsp[0].str); }
    break;

  case 139:
#line 207 "xml_prolog.yy"
    { XmlScan_EntityDefName(yyvsp[0].str, 1); XmltvTags_CheckName(yyvsp[0].str); }
    break;

  case 140:
#line 208 "xml_prolog.yy"
    { XmlScan_EntityDefValue(yyvsp[0].str); XmltvTags_CheckCharset(yyvsp[0].str); }
    break;

  case 141:
#line 210 "xml_prolog.yy"
    { XmltvTags_Notation(0, yyvsp[0].str); }
    break;

  case 142:
#line 211 "xml_prolog.yy"
    { }
    break;

  case 143:
#line 212 "xml_prolog.yy"
    { XmltvTags_PiTarget(yyvsp[0].str); }
    break;

  case 144:
#line 213 "xml_prolog.yy"
    { XmltvTags_PiContent(yyvsp[0].str); }
    break;

  case 145:
#line 215 "xml_prolog.yy"
    { XmltvTags_CheckName(yyvsp[0].str); }
    break;

  case 146:
#line 216 "xml_prolog.yy"
    { XmltvTags_CheckNmtoken(yyvsp[0].str); }
    break;

  case 147:
#line 217 "xml_prolog.yy"
    { XmltvTags_CheckSystemLiteral(yyvsp[0].str); }
    break;

  case 148:
#line 218 "xml_prolog.yy"
    { XmltvTags_CheckCharset(yyvsp[0].str); }
    break;


    }

/* Line 991 of yacc.c.  */
#line 1497 "xml_prolog.tab.c"

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


#line 220 "xml_prolog.yy"


void yyerror( const char * p_msg )
{
   Xmltv_SyntaxError("Prolog parse error", p_msg);
}



/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "grammar.y"

#include "./include/ast.h"
#include "./source/lex/lexer.h"
#include <stdio.h>
#include <stdlib.h>
ASTNode *root;
int yylex(void);
void yyerror(const char* s);

#line 81 "grammar.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "grammar.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENTIFIER = 3,                 /* IDENTIFIER  */
  YYSYMBOL_INT_LITERAL = 4,                /* INT_LITERAL  */
  YYSYMBOL_FLOAT_LITERAL = 5,              /* FLOAT_LITERAL  */
  YYSYMBOL_STRING_LITERAL = 6,             /* STRING_LITERAL  */
  YYSYMBOL_CHAR_LITERAL = 7,               /* CHAR_LITERAL  */
  YYSYMBOL_TRUE = 8,                       /* TRUE  */
  YYSYMBOL_FALSE = 9,                      /* FALSE  */
  YYSYMBOL_NULL_TOKEN = 10,                /* NULL_TOKEN  */
  YYSYMBOL_PROGRAM = 11,                   /* PROGRAM  */
  YYSYMBOL_INCLUDE = 12,                   /* INCLUDE  */
  YYSYMBOL_STRUCT = 13,                    /* STRUCT  */
  YYSYMBOL_FOR = 14,                       /* FOR  */
  YYSYMBOL_IF = 15,                        /* IF  */
  YYSYMBOL_WHILE = 16,                     /* WHILE  */
  YYSYMBOL_RETURN = 17,                    /* RETURN  */
  YYSYMBOL_BREAK = 18,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 19,                  /* CONTINUE  */
  YYSYMBOL_ELSE = 20,                      /* ELSE  */
  YYSYMBOL_INT = 21,                       /* INT  */
  YYSYMBOL_FLOAT = 22,                     /* FLOAT  */
  YYSYMBOL_BOOL = 23,                      /* BOOL  */
  YYSYMBOL_STRING = 24,                    /* STRING  */
  YYSYMBOL_CHAR = 25,                      /* CHAR  */
  YYSYMBOL_VOID = 26,                      /* VOID  */
  YYSYMBOL_OPEN_PAREN = 27,                /* OPEN_PAREN  */
  YYSYMBOL_CLOSE_PAREN = 28,               /* CLOSE_PAREN  */
  YYSYMBOL_OPEN_BRACE = 29,                /* OPEN_BRACE  */
  YYSYMBOL_CLOSE_BRACE = 30,               /* CLOSE_BRACE  */
  YYSYMBOL_SEMICOLON = 31,                 /* SEMICOLON  */
  YYSYMBOL_COMMA = 32,                     /* COMMA  */
  YYSYMBOL_PERIOD = 33,                    /* PERIOD  */
  YYSYMBOL_OPEN_BRACKET = 34,              /* OPEN_BRACKET  */
  YYSYMBOL_CLOSE_BRACKET = 35,             /* CLOSE_BRACKET  */
  YYSYMBOL_EQUAL = 36,                     /* EQUAL  */
  YYSYMBOL_EQUALITY = 37,                  /* EQUALITY  */
  YYSYMBOL_NOT = 38,                       /* NOT  */
  YYSYMBOL_NOT_EQUALS = 39,                /* NOT_EQUALS  */
  YYSYMBOL_TYPE_DECL = 40,                 /* TYPE_DECL  */
  YYSYMBOL_MUL = 41,                       /* MUL  */
  YYSYMBOL_DIV = 42,                       /* DIV  */
  YYSYMBOL_SUB = 43,                       /* SUB  */
  YYSYMBOL_ADD = 44,                       /* ADD  */
  YYSYMBOL_MOD = 45,                       /* MOD  */
  YYSYMBOL_EXPONENT = 46,                  /* EXPONENT  */
  YYSYMBOL_QUOTE = 47,                     /* QUOTE  */
  YYSYMBOL_APOSTROPHE = 48,                /* APOSTROPHE  */
  YYSYMBOL_ELLIPSIS = 49,                  /* ELLIPSIS  */
  YYSYMBOL_AND = 50,                       /* AND  */
  YYSYMBOL_OR = 51,                        /* OR  */
  YYSYMBOL_MUL_EQUALS = 52,                /* MUL_EQUALS  */
  YYSYMBOL_DIV_EQUALS = 53,                /* DIV_EQUALS  */
  YYSYMBOL_SUB_EQUALS = 54,                /* SUB_EQUALS  */
  YYSYMBOL_ADD_EQUALS = 55,                /* ADD_EQUALS  */
  YYSYMBOL_MOD_EQUALS = 56,                /* MOD_EQUALS  */
  YYSYMBOL_GREATER = 57,                   /* GREATER  */
  YYSYMBOL_LESS = 58,                      /* LESS  */
  YYSYMBOL_GREATER_EQUALS = 59,            /* GREATER_EQUALS  */
  YYSYMBOL_LESS_EQUALS = 60,               /* LESS_EQUALS  */
  YYSYMBOL_ADD_ADD = 61,                   /* ADD_ADD  */
  YYSYMBOL_SUB_SUB = 62,                   /* SUB_SUB  */
  YYSYMBOL_BITWISE_AND = 63,               /* BITWISE_AND  */
  YYSYMBOL_BITWISE_OR = 64,                /* BITWISE_OR  */
  YYSYMBOL_BITWISE_NOT = 65,               /* BITWISE_NOT  */
  YYSYMBOL_BITWISE_XOR = 66,               /* BITWISE_XOR  */
  YYSYMBOL_BITWISE_NAND = 67,              /* BITWISE_NAND  */
  YYSYMBOL_BITWISE_NOR = 68,               /* BITWISE_NOR  */
  YYSYMBOL_BITWISE_XNOR = 69,              /* BITWISE_XNOR  */
  YYSYMBOL_BITWISE_ZERO_FILL_LEFT_SHIFT = 70, /* BITWISE_ZERO_FILL_LEFT_SHIFT  */
  YYSYMBOL_BITWISE_SIGNED_RIGHT_SHIFT = 71, /* BITWISE_SIGNED_RIGHT_SHIFT  */
  YYSYMBOL_BITWISE_ZERO_FILL_RIGHT_SHIFT = 72, /* BITWISE_ZERO_FILL_RIGHT_SHIFT  */
  YYSYMBOL_YYACCEPT = 73,                  /* $accept  */
  YYSYMBOL_program = 74,                   /* program  */
  YYSYMBOL_allowed_top_level = 75,         /* allowed_top_level  */
  YYSYMBOL_includes = 76,                  /* includes  */
  YYSYMBOL_include = 77,                   /* include  */
  YYSYMBOL_include_tail = 78,              /* include_tail  */
  YYSYMBOL_top_level_keywords_seq = 79,    /* top_level_keywords_seq  */
  YYSYMBOL_top_level_keywords = 80,        /* top_level_keywords  */
  YYSYMBOL_function_def = 81,              /* function_def  */
  YYSYMBOL_param_list_opt = 82,            /* param_list_opt  */
  YYSYMBOL_param_list = 83,                /* param_list  */
  YYSYMBOL_param = 84,                     /* param  */
  YYSYMBOL_variable_def = 85,              /* variable_def  */
  YYSYMBOL_variable_init_opt = 86,         /* variable_init_opt  */
  YYSYMBOL_struct_def = 87,                /* struct_def  */
  YYSYMBOL_struct = 88,                    /* struct  */
  YYSYMBOL_struct_members = 89,            /* struct_members  */
  YYSYMBOL_struct_member = 90,             /* struct_member  */
  YYSYMBOL_type = 91,                      /* type  */
  YYSYMBOL_default_type = 92,              /* default_type  */
  YYSYMBOL_user_type = 93,                 /* user_type  */
  YYSYMBOL_pointer_type = 94,              /* pointer_type  */
  YYSYMBOL_pointer_stars = 95,             /* pointer_stars  */
  YYSYMBOL_stmts = 96,                     /* stmts  */
  YYSYMBOL_statement = 97,                 /* statement  */
  YYSYMBOL_code_block = 98,                /* code_block  */
  YYSYMBOL_if_stmt = 99,                   /* if_stmt  */
  YYSYMBOL_else_opt = 100,                 /* else_opt  */
  YYSYMBOL_while_stmt = 101,               /* while_stmt  */
  YYSYMBOL_for_stmt = 102,                 /* for_stmt  */
  YYSYMBOL_for_init = 103,                 /* for_init  */
  YYSYMBOL_for_cond_opt = 104,             /* for_cond_opt  */
  YYSYMBOL_for_iter_opt = 105,             /* for_iter_opt  */
  YYSYMBOL_return_stmt = 106,              /* return_stmt  */
  YYSYMBOL_expression_opt = 107,           /* expression_opt  */
  YYSYMBOL_break_stmt = 108,               /* break_stmt  */
  YYSYMBOL_continue_stmt = 109,            /* continue_stmt  */
  YYSYMBOL_expression_statement = 110,     /* expression_statement  */
  YYSYMBOL_expression = 111,               /* expression  */
  YYSYMBOL_assignment = 112,               /* assignment  */
  YYSYMBOL_assignment_op = 113,            /* assignment_op  */
  YYSYMBOL_logical_or = 114,               /* logical_or  */
  YYSYMBOL_logical_and = 115,              /* logical_and  */
  YYSYMBOL_equality = 116,                 /* equality  */
  YYSYMBOL_comparison = 117,               /* comparison  */
  YYSYMBOL_additive = 118,                 /* additive  */
  YYSYMBOL_multiplicative = 119,           /* multiplicative  */
  YYSYMBOL_unary = 120,                    /* unary  */
  YYSYMBOL_postfix = 121,                  /* postfix  */
  YYSYMBOL_primary = 122                   /* primary  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   164

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  119
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  181

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   327


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    34,    34,    38,    42,    43,    47,    51,    52,    56,
      57,    61,    62,    63,    67,    71,    72,    76,    77,    81,
      85,    89,    90,    94,    98,   102,   103,   107,   108,   112,
     113,   114,   115,   119,   120,   121,   122,   123,   127,   131,
     135,   136,   142,   143,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   159,   163,   167,   168,   169,   173,   177,
     181,   182,   186,   187,   191,   192,   196,   200,   201,   205,
     209,   213,   217,   221,   222,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   238,   239,   243,   244,   248,   249,
     250,   254,   255,   256,   257,   258,   262,   263,   264,   268,
     269,   270,   271,   275,   276,   277,   278,   279,   283,   284,
     285,   289,   290,   291,   292,   293,   294,   295,   296,   297
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFIER",
  "INT_LITERAL", "FLOAT_LITERAL", "STRING_LITERAL", "CHAR_LITERAL", "TRUE",
  "FALSE", "NULL_TOKEN", "PROGRAM", "INCLUDE", "STRUCT", "FOR", "IF",
  "WHILE", "RETURN", "BREAK", "CONTINUE", "ELSE", "INT", "FLOAT", "BOOL",
  "STRING", "CHAR", "VOID", "OPEN_PAREN", "CLOSE_PAREN", "OPEN_BRACE",
  "CLOSE_BRACE", "SEMICOLON", "COMMA", "PERIOD", "OPEN_BRACKET",
  "CLOSE_BRACKET", "EQUAL", "EQUALITY", "NOT", "NOT_EQUALS", "TYPE_DECL",
  "MUL", "DIV", "SUB", "ADD", "MOD", "EXPONENT", "QUOTE", "APOSTROPHE",
  "ELLIPSIS", "AND", "OR", "MUL_EQUALS", "DIV_EQUALS", "SUB_EQUALS",
  "ADD_EQUALS", "MOD_EQUALS", "GREATER", "LESS", "GREATER_EQUALS",
  "LESS_EQUALS", "ADD_ADD", "SUB_SUB", "BITWISE_AND", "BITWISE_OR",
  "BITWISE_NOT", "BITWISE_XOR", "BITWISE_NAND", "BITWISE_NOR",
  "BITWISE_XNOR", "BITWISE_ZERO_FILL_LEFT_SHIFT",
  "BITWISE_SIGNED_RIGHT_SHIFT", "BITWISE_ZERO_FILL_RIGHT_SHIFT", "$accept",
  "program", "allowed_top_level", "includes", "include", "include_tail",
  "top_level_keywords_seq", "top_level_keywords", "function_def",
  "param_list_opt", "param_list", "param", "variable_def",
  "variable_init_opt", "struct_def", "struct", "struct_members",
  "struct_member", "type", "default_type", "user_type", "pointer_type",
  "pointer_stars", "stmts", "statement", "code_block", "if_stmt",
  "else_opt", "while_stmt", "for_stmt", "for_init", "for_cond_opt",
  "for_iter_opt", "return_stmt", "expression_opt", "break_stmt",
  "continue_stmt", "expression_statement", "expression", "assignment",
  "assignment_op", "logical_or", "logical_and", "equality", "comparison",
  "additive", "multiplicative", "unary", "postfix", "primary", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-93)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-42)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -93,    11,   -93,    28,   -93,    49,   -93,    83,    29,    40,
      43,    82,   -93,   -93,   -93,   -93,   -93,    86,    61,   119,
     119,    66,    29,   -93,   -93,   -93,   -93,   -93,   -93,   -93,
     -93,    -8,   -93,   -93,   -93,    94,   -93,   -93,    41,    71,
      62,    78,    31,   -93,   -93,   -93,   -93,   -93,   -93,   -93,
     -93,    41,    41,    41,    41,    41,   -93,   -93,    65,    57,
      39,     7,   -23,   -10,   -93,   -93,   -25,   -93,   -93,   105,
      80,   -93,   -93,   -93,    81,   -93,   -93,   -93,   -93,   -93,
      41,   -93,   -93,   -93,   -93,   -93,   -93,   -93,   -93,    41,
      41,    41,    41,    41,    41,    41,    41,    41,    41,    41,
      41,    41,   -93,   -93,    72,   102,   100,   -93,   -93,   -93,
      57,   -93,    39,     7,     7,   -23,   -23,   -23,   -23,   -10,
     -10,   -93,   -93,   -93,   119,   104,   105,    95,   -93,   -93,
     -93,     0,    40,   108,   110,   111,    41,   115,   116,   -93,
     -93,   -93,   -93,   -93,   -93,   -93,   -93,   -93,   -93,   -93,
     117,    50,    41,    41,   118,   -93,   -93,   -93,   -93,   -93,
     120,   -93,   122,   124,   -93,    41,   104,   104,   123,   -93,
     133,   -93,    41,    -3,   -93,   127,   -93,   -93,   -93,   104,
     -93
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       5,     0,     2,    10,     1,     0,     4,     3,     8,     0,
       0,     0,     9,    11,    13,    12,    23,     0,     0,     0,
       0,     0,     8,     6,    38,    33,    34,    36,    35,    37,
      32,    22,    29,    30,    31,    41,    26,     7,     0,     0,
      39,     0,     0,   111,   112,   113,   114,   115,   116,   117,
     118,     0,     0,     0,     0,     0,    21,    72,    74,    85,
      87,    90,    95,    98,   102,   107,   110,    20,    40,    16,
       0,    28,    27,    25,     0,   103,   106,   104,   105,    75,
       0,    78,    79,    77,    76,    80,    81,    82,    83,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   108,   109,     0,     0,    15,    18,    24,   119,
      84,    73,    86,    88,    89,    92,    91,    94,    93,    97,
      96,    99,   100,   101,     0,     0,     0,    19,    43,    14,
      17,     0,   111,     0,     0,     0,    68,     0,     0,    53,
      52,    42,    51,    44,    45,    46,    47,    48,    49,    50,
       0,     0,     0,     0,     0,    67,    69,    70,    71,    60,
       0,    61,     0,     0,    66,    63,     0,     0,     0,    62,
      57,    58,    65,     0,    54,     0,    64,    56,    55,     0,
      59
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -93,   -93,   -93,   -93,   -93,   134,   -93,   -93,    97,   -93,
     -93,    32,   -41,   -93,   -93,   -93,   -93,   -93,   -18,   -93,
     -93,   -93,   -93,   -93,   -93,   -92,   -16,   -93,   -93,   -93,
     -93,   -93,   -93,   -93,   -93,   -93,   -93,     8,   -38,    73,
     -93,   -93,    84,    70,   -19,    30,     2,   -30,   -93,   -93
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,     2,     3,     6,    18,     7,    12,    13,   105,
     106,   107,    14,    39,    15,    16,    42,    73,    31,    32,
      33,    34,    40,   131,   141,   129,   143,   174,   144,   145,
     160,   168,   175,   146,   154,   147,   148,   149,   150,    57,
      89,    58,    59,    60,    61,    62,    63,    64,    65,    66
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      56,    72,    35,   132,    44,    45,    46,    47,    48,    49,
      50,     4,   134,    74,   133,   134,   135,   136,   137,   138,
      97,    98,    75,    76,    77,    78,   128,    51,    38,   128,
     139,    99,   100,   -41,     9,   101,   102,   103,    52,   142,
       5,    53,    10,    54,    43,    44,    45,    46,    47,    48,
      49,    50,     8,   132,    44,    45,    46,    47,    48,    49,
      50,    70,    17,    55,    93,    94,    95,    96,    51,   121,
     122,   123,   113,   114,   170,   171,    91,    51,    92,    52,
      19,   177,    53,    20,    54,    21,     9,   180,    52,    22,
     140,    53,    23,    54,    10,    36,    11,    41,   155,   119,
     120,    79,    67,    68,    55,    69,   127,    90,   104,   109,
     159,   108,   124,    55,   162,   163,    80,    81,    82,    83,
      84,    85,    24,   115,   116,   117,   118,   169,    86,    87,
     125,    88,   126,   128,   176,   151,   -41,   152,   153,    71,
      25,    26,    27,    28,    29,    30,   156,   157,   158,   164,
     166,   165,   167,   173,   172,   179,    37,   178,   130,   161,
     112,     0,   111,     0,   110
};

static const yytype_int16 yycheck[] =
{
      38,    42,    20,     3,     4,     5,     6,     7,     8,     9,
      10,     0,    15,    51,    14,    15,    16,    17,    18,    19,
      43,    44,    52,    53,    54,    55,    29,    27,    36,    29,
      30,    41,    42,    41,     3,    45,    61,    62,    38,   131,
      12,    41,    11,    43,     3,     4,     5,     6,     7,     8,
       9,    10,     3,     3,     4,     5,     6,     7,     8,     9,
      10,    30,    33,    63,    57,    58,    59,    60,    27,    99,
     100,   101,    91,    92,   166,   167,    37,    27,    39,    38,
      40,   173,    41,    40,    43,     3,     3,   179,    38,     3,
     131,    41,    31,    43,    11,    29,    13,     3,   136,    97,
      98,    36,    31,    41,    63,    27,   124,    50,     3,    28,
     151,    31,    40,    63,   152,   153,    51,    52,    53,    54,
      55,    56,     3,    93,    94,    95,    96,   165,    63,    64,
      28,    66,    32,    29,   172,    27,    41,    27,    27,    42,
      21,    22,    23,    24,    25,    26,    31,    31,    31,    31,
      28,    31,    28,    20,    31,    28,    22,   173,   126,   151,
      90,    -1,    89,    -1,    80
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    74,    75,    76,     0,    12,    77,    79,     3,     3,
      11,    13,    80,    81,    85,    87,    88,    33,    78,    40,
      40,     3,     3,    31,     3,    21,    22,    23,    24,    25,
      26,    91,    92,    93,    94,    91,    29,    78,    36,    86,
      95,     3,    89,     3,     4,     5,     6,     7,     8,     9,
      10,    27,    38,    41,    43,    63,   111,   112,   114,   115,
     116,   117,   118,   119,   120,   121,   122,    31,    41,    27,
      30,    81,    85,    90,   111,   120,   120,   120,   120,    36,
      51,    52,    53,    54,    55,    56,    63,    64,    66,   113,
      50,    37,    39,    57,    58,    59,    60,    43,    44,    41,
      42,    45,    61,    62,     3,    82,    83,    84,    31,    28,
     115,   112,   116,   117,   117,   118,   118,   118,   118,   119,
     119,   120,   120,   120,    40,    28,    32,    91,    29,    98,
      84,    96,     3,    14,    15,    16,    17,    18,    19,    30,
      85,    97,    98,    99,   101,   102,   106,   108,   109,   110,
     111,    27,    27,    27,   107,   111,    31,    31,    31,    85,
     103,   110,   111,   111,    31,    31,    28,    28,   104,   111,
      98,    98,    31,    20,   100,   105,   111,    98,    99,    28,
      98
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    73,    74,    75,    76,    76,    77,    78,    78,    79,
      79,    80,    80,    80,    81,    82,    82,    83,    83,    84,
      85,    86,    86,    87,    88,    89,    89,    90,    90,    91,
      91,    91,    91,    92,    92,    92,    92,    92,    93,    94,
      95,    95,    96,    96,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    98,    99,   100,   100,   100,   101,   102,
     103,   103,   104,   104,   105,   105,   106,   107,   107,   108,
     109,   110,   111,   112,   112,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   114,   114,   115,   115,   116,   116,
     116,   117,   117,   117,   117,   117,   118,   118,   118,   119,
     119,   119,   119,   120,   120,   120,   120,   120,   121,   121,
     121,   122,   122,   122,   122,   122,   122,   122,   122,   122
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     2,     0,     4,     3,     0,     2,
       0,     1,     1,     1,     8,     1,     0,     3,     1,     3,
       5,     2,     0,     1,     6,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     0,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     6,     2,     2,     0,     5,     9,
       1,     1,     1,     0,     1,     0,     3,     1,     0,     2,
       2,     2,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     1,     3,     3,     1,     3,
       3,     3,     1,     2,     2,     2,     2,     1,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: allowed_top_level  */
#line 34 "grammar.y"
                                       { root = (yyvsp[0].node); }
#line 1333 "grammar.tab.c"
    break;

  case 3: /* allowed_top_level: includes top_level_keywords_seq  */
#line 38 "grammar.y"
                                        { (yyval.node) = ast_allowed_top_level((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1339 "grammar.tab.c"
    break;

  case 4: /* includes: includes include  */
#line 42 "grammar.y"
                                       { (yyval.node) = ast_append_include((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1345 "grammar.tab.c"
    break;

  case 5: /* includes: %empty  */
#line 43 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1351 "grammar.tab.c"
    break;

  case 6: /* include: INCLUDE IDENTIFIER include_tail SEMICOLON  */
#line 47 "grammar.y"
                                                { (yyval.node) = ast_include((yyvsp[-2].sval), (yyvsp[-1].node)); }
#line 1357 "grammar.tab.c"
    break;

  case 7: /* include_tail: PERIOD IDENTIFIER include_tail  */
#line 51 "grammar.y"
                                       { (yyval.node) = ast_include_tail((yyvsp[-1].sval), (yyvsp[0].node)); }
#line 1363 "grammar.tab.c"
    break;

  case 8: /* include_tail: %empty  */
#line 52 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1369 "grammar.tab.c"
    break;

  case 9: /* top_level_keywords_seq: top_level_keywords_seq top_level_keywords  */
#line 56 "grammar.y"
                                                { (yyval.node) = ast_append_top_level((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1375 "grammar.tab.c"
    break;

  case 10: /* top_level_keywords_seq: %empty  */
#line 57 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1381 "grammar.tab.c"
    break;

  case 11: /* top_level_keywords: function_def  */
#line 61 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1387 "grammar.tab.c"
    break;

  case 12: /* top_level_keywords: struct_def  */
#line 62 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1393 "grammar.tab.c"
    break;

  case 13: /* top_level_keywords: variable_def  */
#line 63 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1399 "grammar.tab.c"
    break;

  case 14: /* function_def: PROGRAM TYPE_DECL type IDENTIFIER OPEN_PAREN param_list_opt CLOSE_PAREN code_block  */
#line 67 "grammar.y"
                                                                                         { (yyval.node) = ast_function_def((yyvsp[-5].node), (yyvsp[-4].sval), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1405 "grammar.tab.c"
    break;

  case 15: /* param_list_opt: param_list  */
#line 71 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1411 "grammar.tab.c"
    break;

  case 16: /* param_list_opt: %empty  */
#line 72 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1417 "grammar.tab.c"
    break;

  case 17: /* param_list: param_list COMMA param  */
#line 76 "grammar.y"
                                       { (yyval.node) = ast_append_param((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1423 "grammar.tab.c"
    break;

  case 18: /* param_list: param  */
#line 77 "grammar.y"
                                       { (yyval.node) = ast_single_param((yyvsp[0].node)); }
#line 1429 "grammar.tab.c"
    break;

  case 19: /* param: IDENTIFIER TYPE_DECL type  */
#line 81 "grammar.y"
                                       { (yyval.node) = ast_param((yyvsp[-2].sval), (yyvsp[0].node)); }
#line 1435 "grammar.tab.c"
    break;

  case 20: /* variable_def: IDENTIFIER TYPE_DECL type variable_init_opt SEMICOLON  */
#line 85 "grammar.y"
                                                            { (yyval.node) = ast_variable_def((yyvsp[-4].sval), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 1441 "grammar.tab.c"
    break;

  case 21: /* variable_init_opt: EQUAL expression  */
#line 89 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1447 "grammar.tab.c"
    break;

  case 22: /* variable_init_opt: %empty  */
#line 90 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1453 "grammar.tab.c"
    break;

  case 23: /* struct_def: struct  */
#line 94 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1459 "grammar.tab.c"
    break;

  case 24: /* struct: STRUCT IDENTIFIER OPEN_BRACE struct_members CLOSE_BRACE SEMICOLON  */
#line 98 "grammar.y"
                                                                        { (yyval.node) = ast_struct((yyvsp[-4].sval), (yyvsp[-2].node)); }
#line 1465 "grammar.tab.c"
    break;

  case 25: /* struct_members: struct_members struct_member  */
#line 102 "grammar.y"
                                        { (yyval.node) = ast_append_struct_member((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1471 "grammar.tab.c"
    break;

  case 26: /* struct_members: %empty  */
#line 103 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1477 "grammar.tab.c"
    break;

  case 27: /* struct_member: variable_def  */
#line 107 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1483 "grammar.tab.c"
    break;

  case 28: /* struct_member: function_def  */
#line 108 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1489 "grammar.tab.c"
    break;

  case 29: /* type: default_type  */
#line 112 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1495 "grammar.tab.c"
    break;

  case 30: /* type: user_type  */
#line 113 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1501 "grammar.tab.c"
    break;

  case 31: /* type: pointer_type  */
#line 114 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1507 "grammar.tab.c"
    break;

  case 32: /* type: VOID  */
#line 115 "grammar.y"
                                       { (yyval.node) = ast_type_void(); }
#line 1513 "grammar.tab.c"
    break;

  case 33: /* default_type: INT  */
#line 119 "grammar.y"
                                       { (yyval.node) = ast_type_int(); }
#line 1519 "grammar.tab.c"
    break;

  case 34: /* default_type: FLOAT  */
#line 120 "grammar.y"
                                       { (yyval.node) = ast_type_float(); }
#line 1525 "grammar.tab.c"
    break;

  case 35: /* default_type: STRING  */
#line 121 "grammar.y"
                                       { (yyval.node) = ast_type_string(); }
#line 1531 "grammar.tab.c"
    break;

  case 36: /* default_type: BOOL  */
#line 122 "grammar.y"
                                       { (yyval.node) = ast_type_bool(); }
#line 1537 "grammar.tab.c"
    break;

  case 37: /* default_type: CHAR  */
#line 123 "grammar.y"
                                       { (yyval.node) = ast_type_char(); }
#line 1543 "grammar.tab.c"
    break;

  case 38: /* user_type: IDENTIFIER  */
#line 127 "grammar.y"
                                       { (yyval.node) = ast_type_user((yyvsp[0].sval)); }
#line 1549 "grammar.tab.c"
    break;

  case 39: /* pointer_type: type pointer_stars  */
#line 131 "grammar.y"
                                       { (yyval.node) = ast_type_pointer((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1555 "grammar.tab.c"
    break;

  case 40: /* pointer_stars: pointer_stars MUL  */
#line 135 "grammar.y"
                                       { (yyval.node) = ast_append_pointer_star((yyvsp[-1].node)); }
#line 1561 "grammar.tab.c"
    break;

  case 41: /* pointer_stars: %empty  */
#line 136 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1567 "grammar.tab.c"
    break;

  case 42: /* stmts: stmts statement  */
#line 142 "grammar.y"
                                       { (yyval.node) = ast_append_stmt((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1573 "grammar.tab.c"
    break;

  case 43: /* stmts: %empty  */
#line 143 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1579 "grammar.tab.c"
    break;

  case 44: /* statement: if_stmt  */
#line 147 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1585 "grammar.tab.c"
    break;

  case 45: /* statement: while_stmt  */
#line 148 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1591 "grammar.tab.c"
    break;

  case 46: /* statement: for_stmt  */
#line 149 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1597 "grammar.tab.c"
    break;

  case 47: /* statement: return_stmt  */
#line 150 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1603 "grammar.tab.c"
    break;

  case 48: /* statement: break_stmt  */
#line 151 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1609 "grammar.tab.c"
    break;

  case 49: /* statement: continue_stmt  */
#line 152 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1615 "grammar.tab.c"
    break;

  case 50: /* statement: expression_statement  */
#line 153 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1621 "grammar.tab.c"
    break;

  case 51: /* statement: code_block  */
#line 154 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1627 "grammar.tab.c"
    break;

  case 52: /* statement: variable_def  */
#line 155 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1633 "grammar.tab.c"
    break;

  case 53: /* code_block: OPEN_BRACE stmts CLOSE_BRACE  */
#line 159 "grammar.y"
                                        { (yyval.node) = ast_code_block((yyvsp[-1].node)); }
#line 1639 "grammar.tab.c"
    break;

  case 54: /* if_stmt: IF OPEN_PAREN expression CLOSE_PAREN code_block else_opt  */
#line 163 "grammar.y"
                                                               { (yyval.node) = ast_if_stmt((yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 1645 "grammar.tab.c"
    break;

  case 55: /* else_opt: ELSE if_stmt  */
#line 167 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1651 "grammar.tab.c"
    break;

  case 56: /* else_opt: ELSE code_block  */
#line 168 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1657 "grammar.tab.c"
    break;

  case 57: /* else_opt: %empty  */
#line 169 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1663 "grammar.tab.c"
    break;

  case 58: /* while_stmt: WHILE OPEN_PAREN expression CLOSE_PAREN code_block  */
#line 173 "grammar.y"
                                                         { (yyval.node) = ast_while_stmt((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1669 "grammar.tab.c"
    break;

  case 59: /* for_stmt: FOR OPEN_PAREN for_init SEMICOLON for_cond_opt SEMICOLON for_iter_opt CLOSE_PAREN code_block  */
#line 177 "grammar.y"
                                                                                                   { (yyval.node) = ast_for_stmt((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1675 "grammar.tab.c"
    break;

  case 60: /* for_init: variable_def  */
#line 181 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1681 "grammar.tab.c"
    break;

  case 61: /* for_init: expression_statement  */
#line 182 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1687 "grammar.tab.c"
    break;

  case 62: /* for_cond_opt: expression  */
#line 186 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1693 "grammar.tab.c"
    break;

  case 63: /* for_cond_opt: %empty  */
#line 187 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1699 "grammar.tab.c"
    break;

  case 64: /* for_iter_opt: expression  */
#line 191 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1705 "grammar.tab.c"
    break;

  case 65: /* for_iter_opt: %empty  */
#line 192 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1711 "grammar.tab.c"
    break;

  case 66: /* return_stmt: RETURN expression_opt SEMICOLON  */
#line 196 "grammar.y"
                                        { (yyval.node) = ast_return_stmt((yyvsp[-1].node)); }
#line 1717 "grammar.tab.c"
    break;

  case 67: /* expression_opt: expression  */
#line 200 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1723 "grammar.tab.c"
    break;

  case 68: /* expression_opt: %empty  */
#line 201 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1729 "grammar.tab.c"
    break;

  case 69: /* break_stmt: BREAK SEMICOLON  */
#line 205 "grammar.y"
                                        { (yyval.node) = ast_break_stmt(); }
#line 1735 "grammar.tab.c"
    break;

  case 70: /* continue_stmt: CONTINUE SEMICOLON  */
#line 209 "grammar.y"
                                        { (yyval.node) = ast_continue_stmt(); }
#line 1741 "grammar.tab.c"
    break;

  case 71: /* expression_statement: expression SEMICOLON  */
#line 213 "grammar.y"
                                        { (yyval.node) = ast_expr_stmt((yyvsp[-1].node)); }
#line 1747 "grammar.tab.c"
    break;

  case 72: /* expression: assignment  */
#line 217 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1753 "grammar.tab.c"
    break;

  case 73: /* assignment: logical_or assignment_op assignment  */
#line 221 "grammar.y"
                                          { (yyval.node) = ast_assignment((yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 1759 "grammar.tab.c"
    break;

  case 74: /* assignment: logical_or  */
#line 222 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1765 "grammar.tab.c"
    break;

  case 75: /* assignment_op: EQUAL  */
#line 226 "grammar.y"
                                        { (yyval.node) = ast_assign_op("="); }
#line 1771 "grammar.tab.c"
    break;

  case 76: /* assignment_op: ADD_EQUALS  */
#line 227 "grammar.y"
                                        { (yyval.node) = ast_assign_op("+="); }
#line 1777 "grammar.tab.c"
    break;

  case 77: /* assignment_op: SUB_EQUALS  */
#line 228 "grammar.y"
                                        { (yyval.node) = ast_assign_op("-="); }
#line 1783 "grammar.tab.c"
    break;

  case 78: /* assignment_op: MUL_EQUALS  */
#line 229 "grammar.y"
                                        { (yyval.node) = ast_assign_op("*="); }
#line 1789 "grammar.tab.c"
    break;

  case 79: /* assignment_op: DIV_EQUALS  */
#line 230 "grammar.y"
                                        { (yyval.node) = ast_assign_op("/="); }
#line 1795 "grammar.tab.c"
    break;

  case 80: /* assignment_op: MOD_EQUALS  */
#line 231 "grammar.y"
                                        { (yyval.node) = ast_assign_op("%="); }
#line 1801 "grammar.tab.c"
    break;

  case 81: /* assignment_op: BITWISE_AND  */
#line 232 "grammar.y"
                                        { (yyval.node) = ast_assign_op("&="); }
#line 1807 "grammar.tab.c"
    break;

  case 82: /* assignment_op: BITWISE_OR  */
#line 233 "grammar.y"
                                        { (yyval.node) = ast_assign_op("|="); }
#line 1813 "grammar.tab.c"
    break;

  case 83: /* assignment_op: BITWISE_XOR  */
#line 234 "grammar.y"
                                        { (yyval.node) = ast_assign_op("^="); }
#line 1819 "grammar.tab.c"
    break;

  case 84: /* logical_or: logical_or OR logical_and  */
#line 238 "grammar.y"
                                        { (yyval.node) = ast_binary_op("||", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1825 "grammar.tab.c"
    break;

  case 85: /* logical_or: logical_and  */
#line 239 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1831 "grammar.tab.c"
    break;

  case 86: /* logical_and: logical_and AND equality  */
#line 243 "grammar.y"
                                        { (yyval.node) = ast_binary_op("&&", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1837 "grammar.tab.c"
    break;

  case 87: /* logical_and: equality  */
#line 244 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1843 "grammar.tab.c"
    break;

  case 88: /* equality: equality EQUALITY comparison  */
#line 248 "grammar.y"
                                        { (yyval.node) = ast_binary_op("==", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1849 "grammar.tab.c"
    break;

  case 89: /* equality: equality NOT_EQUALS comparison  */
#line 249 "grammar.y"
                                        { (yyval.node) = ast_binary_op("!=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1855 "grammar.tab.c"
    break;

  case 90: /* equality: comparison  */
#line 250 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1861 "grammar.tab.c"
    break;

  case 91: /* comparison: comparison LESS additive  */
#line 254 "grammar.y"
                                        { (yyval.node) = ast_binary_op("<", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1867 "grammar.tab.c"
    break;

  case 92: /* comparison: comparison GREATER additive  */
#line 255 "grammar.y"
                                        { (yyval.node) = ast_binary_op(">", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1873 "grammar.tab.c"
    break;

  case 93: /* comparison: comparison LESS_EQUALS additive  */
#line 256 "grammar.y"
                                        { (yyval.node) = ast_binary_op("<=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1879 "grammar.tab.c"
    break;

  case 94: /* comparison: comparison GREATER_EQUALS additive  */
#line 257 "grammar.y"
                                        { (yyval.node) = ast_binary_op(">=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1885 "grammar.tab.c"
    break;

  case 95: /* comparison: additive  */
#line 258 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1891 "grammar.tab.c"
    break;

  case 96: /* additive: additive ADD multiplicative  */
#line 262 "grammar.y"
                                        { (yyval.node) = ast_binary_op("+", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1897 "grammar.tab.c"
    break;

  case 97: /* additive: additive SUB multiplicative  */
#line 263 "grammar.y"
                                        { (yyval.node) = ast_binary_op("-", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1903 "grammar.tab.c"
    break;

  case 98: /* additive: multiplicative  */
#line 264 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1909 "grammar.tab.c"
    break;

  case 99: /* multiplicative: multiplicative MUL unary  */
#line 268 "grammar.y"
                                        { (yyval.node) = ast_binary_op("*", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1915 "grammar.tab.c"
    break;

  case 100: /* multiplicative: multiplicative DIV unary  */
#line 269 "grammar.y"
                                        { (yyval.node) = ast_binary_op("/", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1921 "grammar.tab.c"
    break;

  case 101: /* multiplicative: multiplicative MOD unary  */
#line 270 "grammar.y"
                                        { (yyval.node) = ast_binary_op("%", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1927 "grammar.tab.c"
    break;

  case 102: /* multiplicative: unary  */
#line 271 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1933 "grammar.tab.c"
    break;

  case 103: /* unary: NOT unary  */
#line 275 "grammar.y"
                                        { (yyval.node) = ast_unary_op("!", (yyvsp[0].node)); }
#line 1939 "grammar.tab.c"
    break;

  case 104: /* unary: SUB unary  */
#line 276 "grammar.y"
                                        { (yyval.node) = ast_unary_op("-", (yyvsp[0].node)); }
#line 1945 "grammar.tab.c"
    break;

  case 105: /* unary: BITWISE_AND unary  */
#line 277 "grammar.y"
                                        { (yyval.node) = ast_unary_op("&", (yyvsp[0].node)); }
#line 1951 "grammar.tab.c"
    break;

  case 106: /* unary: MUL unary  */
#line 278 "grammar.y"
                                        { (yyval.node) = ast_unary_op("*", (yyvsp[0].node)); }
#line 1957 "grammar.tab.c"
    break;

  case 107: /* unary: postfix  */
#line 279 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1963 "grammar.tab.c"
    break;

  case 108: /* postfix: primary ADD_ADD  */
#line 283 "grammar.y"
                                        { (yyval.node) = ast_postfix_op("++", (yyvsp[-1].node)); }
#line 1969 "grammar.tab.c"
    break;

  case 109: /* postfix: primary SUB_SUB  */
#line 284 "grammar.y"
                                        { (yyval.node) = ast_postfix_op("--", (yyvsp[-1].node)); }
#line 1975 "grammar.tab.c"
    break;

  case 110: /* postfix: primary  */
#line 285 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1981 "grammar.tab.c"
    break;

  case 111: /* primary: IDENTIFIER  */
#line 289 "grammar.y"
                                        { (yyval.node) = ast_identifier((yyvsp[0].sval)); }
#line 1987 "grammar.tab.c"
    break;

  case 112: /* primary: INT_LITERAL  */
#line 290 "grammar.y"
                                        { (yyval.node) = ast_int_literal((yyvsp[0].ival)); }
#line 1993 "grammar.tab.c"
    break;

  case 113: /* primary: FLOAT_LITERAL  */
#line 291 "grammar.y"
                                        { (yyval.node) = ast_float_literal((yyvsp[0].fval)); }
#line 1999 "grammar.tab.c"
    break;

  case 114: /* primary: STRING_LITERAL  */
#line 292 "grammar.y"
                                        { (yyval.node) = ast_string_literal((yyvsp[0].sval)); }
#line 2005 "grammar.tab.c"
    break;

  case 115: /* primary: CHAR_LITERAL  */
#line 293 "grammar.y"
                                        { (yyval.node) = ast_char_literal((yyvsp[0].sval)[0]); }
#line 2011 "grammar.tab.c"
    break;

  case 116: /* primary: TRUE  */
#line 294 "grammar.y"
                                        { (yyval.node) = ast_true(); }
#line 2017 "grammar.tab.c"
    break;

  case 117: /* primary: FALSE  */
#line 295 "grammar.y"
                                        { (yyval.node) = ast_false(); }
#line 2023 "grammar.tab.c"
    break;

  case 118: /* primary: NULL_TOKEN  */
#line 296 "grammar.y"
                                        { (yyval.node) = ast_null(); }
#line 2029 "grammar.tab.c"
    break;

  case 119: /* primary: OPEN_PAREN expression CLOSE_PAREN  */
#line 297 "grammar.y"
                                        { (yyval.node) = (yyvsp[-1].node); }
#line 2035 "grammar.tab.c"
    break;


#line 2039 "grammar.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 300 "grammar.y"



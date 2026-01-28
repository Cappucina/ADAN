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
  YYSYMBOL_array_type = 92,                /* array_type  */
  YYSYMBOL_default_type = 93,              /* default_type  */
  YYSYMBOL_user_type = 94,                 /* user_type  */
  YYSYMBOL_pointer_type = 95,              /* pointer_type  */
  YYSYMBOL_pointer_stars = 96,             /* pointer_stars  */
  YYSYMBOL_stmts = 97,                     /* stmts  */
  YYSYMBOL_statement = 98,                 /* statement  */
  YYSYMBOL_code_block = 99,                /* code_block  */
  YYSYMBOL_if_stmt = 100,                  /* if_stmt  */
  YYSYMBOL_else_opt = 101,                 /* else_opt  */
  YYSYMBOL_while_stmt = 102,               /* while_stmt  */
  YYSYMBOL_for_stmt = 103,                 /* for_stmt  */
  YYSYMBOL_for_init_opt = 104,             /* for_init_opt  */
  YYSYMBOL_for_init = 105,                 /* for_init  */
  YYSYMBOL_variable_def_no_semi = 106,     /* variable_def_no_semi  */
  YYSYMBOL_for_cond_opt = 107,             /* for_cond_opt  */
  YYSYMBOL_for_iter_opt = 108,             /* for_iter_opt  */
  YYSYMBOL_return_stmt = 109,              /* return_stmt  */
  YYSYMBOL_expression_opt = 110,           /* expression_opt  */
  YYSYMBOL_break_stmt = 111,               /* break_stmt  */
  YYSYMBOL_continue_stmt = 112,            /* continue_stmt  */
  YYSYMBOL_expression_statement = 113,     /* expression_statement  */
  YYSYMBOL_expression = 114,               /* expression  */
  YYSYMBOL_assignment = 115,               /* assignment  */
  YYSYMBOL_assignment_op = 116,            /* assignment_op  */
  YYSYMBOL_logical_or = 117,               /* logical_or  */
  YYSYMBOL_logical_and = 118,              /* logical_and  */
  YYSYMBOL_equality = 119,                 /* equality  */
  YYSYMBOL_comparison = 120,               /* comparison  */
  YYSYMBOL_additive = 121,                 /* additive  */
  YYSYMBOL_multiplicative = 122,           /* multiplicative  */
  YYSYMBOL_unary = 123,                    /* unary  */
  YYSYMBOL_postfix = 124,                  /* postfix  */
  YYSYMBOL_primary = 125,                  /* primary  */
  YYSYMBOL_array_literal = 126,            /* array_literal  */
  YYSYMBOL_expr_list = 127                 /* expr_list  */
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
#define YYLAST   184

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  55
/* YYNRULES -- Number of rules.  */
#define YYNRULES  131
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  200

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
      85,    89,    90,    94,    98,   102,   103,   107,   108,   113,
     114,   115,   116,   117,   121,   125,   126,   127,   128,   129,
     133,   137,   141,   142,   146,   147,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   163,   167,   171,   172,   173,
     177,   182,   186,   187,   192,   193,   197,   201,   202,   206,
     207,   211,   215,   216,   220,   224,   228,   232,   236,   237,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   253,
     254,   258,   259,   263,   264,   265,   269,   270,   271,   272,
     273,   277,   278,   279,   283,   284,   285,   286,   291,   292,
     293,   294,   295,   296,   297,   301,   302,   303,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   319,   323,
     324,   325
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
  "struct_member", "type", "array_type", "default_type", "user_type",
  "pointer_type", "pointer_stars", "stmts", "statement", "code_block",
  "if_stmt", "else_opt", "while_stmt", "for_stmt", "for_init_opt",
  "for_init", "variable_def_no_semi", "for_cond_opt", "for_iter_opt",
  "return_stmt", "expression_opt", "break_stmt", "continue_stmt",
  "expression_statement", "expression", "assignment", "assignment_op",
  "logical_or", "logical_and", "equality", "comparison", "additive",
  "multiplicative", "unary", "postfix", "primary", "array_literal",
  "expr_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-115)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-44)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -115,    36,  -115,    -8,  -115,    11,  -115,    64,     6,     7,
      33,    78,  -115,  -115,  -115,  -115,  -115,    82,    61,     2,
       2,    80,     6,  -115,  -115,  -115,  -115,  -115,  -115,  -115,
    -115,     4,  -115,  -115,  -115,  -115,     0,  -115,  -115,    91,
      45,   103,    72,   110,    65,  -115,  -115,  -115,  -115,  -115,
    -115,  -115,  -115,  -115,    45,    45,    45,    45,    45,    45,
      45,    45,  -115,  -115,    93,    88,    73,    40,    58,    48,
    -115,    43,  -115,  -115,  -115,  -115,   136,   109,  -115,  -115,
    -115,   113,  -115,    39,  -115,  -115,  -115,  -115,  -115,  -115,
    -115,    45,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,
      45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
      45,    45,    45,  -115,  -115,   102,   122,   119,  -115,  -115,
    -115,    45,  -115,    88,  -115,    73,    40,    40,    58,    58,
      58,    58,    48,    48,  -115,  -115,  -115,     2,   124,   136,
    -115,     1,  -115,  -115,  -115,     3,     7,   127,   128,   131,
      45,   129,   132,  -115,  -115,  -115,  -115,  -115,  -115,  -115,
    -115,  -115,  -115,  -115,   133,    53,    45,    45,   134,  -115,
    -115,  -115,  -115,   126,   137,  -115,  -115,  -115,   139,   141,
    -115,     2,    45,   124,   124,     4,   140,  -115,   142,  -115,
    -115,    45,    14,  -115,   147,  -115,  -115,  -115,   124,  -115
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       5,     0,     2,    10,     1,     0,     4,     3,     8,     0,
       0,     0,     9,    11,    13,    12,    23,     0,     0,     0,
       0,     0,     8,     6,    40,    35,    36,    38,    37,    39,
      33,    22,    32,    29,    30,    31,    43,    26,     7,     0,
       0,     0,    41,     0,     0,    34,   118,   119,   120,   121,
     122,   123,   124,   125,     0,   131,     0,     0,     0,     0,
       0,     0,    21,    77,    79,    90,    92,    95,   100,   103,
     107,   114,   117,   127,    20,    42,    16,     0,    28,    27,
      25,     0,   129,     0,   108,   111,   109,   112,   113,   110,
      80,     0,    83,    84,    82,    81,    85,    86,    87,    88,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   115,   116,     0,     0,    15,    18,    24,
     126,     0,   128,    89,    78,    91,    93,    94,    97,    96,
      99,    98,   102,   101,   104,   105,   106,     0,     0,     0,
     130,    19,    45,    14,    17,     0,   118,     0,     0,     0,
      73,     0,     0,    55,    54,    44,    53,    46,    47,    48,
      49,    50,    51,    52,     0,    63,     0,     0,     0,    72,
      74,    75,    76,   118,     0,    62,    64,    65,     0,     0,
      71,     0,    68,     0,     0,    22,     0,    67,    59,    60,
      66,    70,     0,    56,     0,    69,    58,    57,     0,    61
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -115,  -115,  -115,  -115,  -115,   148,  -115,  -115,   135,  -115,
    -115,    37,   -42,    -7,  -115,  -115,  -115,  -115,   -20,  -115,
    -115,  -115,  -115,  -115,  -115,  -115,  -114,   -15,  -115,  -115,
    -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,  -115,
      15,   -39,    81,  -115,  -115,    92,    83,    22,    26,    27,
      62,  -115,  -115,  -115,  -115
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,     2,     3,     6,    18,     7,    12,    13,   116,
     117,   118,    14,    41,    15,    16,    44,    80,    31,    32,
      33,    34,    35,    42,   145,   155,   143,   157,   193,   158,
     159,   174,   175,   176,   186,   194,   160,   168,   161,   162,
     163,   164,    63,   100,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    83
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      36,    62,    79,    43,     5,    24,   146,    47,    48,    49,
      50,    51,    52,    53,     8,    81,    82,   147,   148,   149,
     150,   151,   152,    25,    26,    27,    28,    29,    30,   148,
      54,   156,   142,   153,    39,    39,     4,    55,    39,    17,
      40,    56,   -43,   142,    57,   -43,    58,    19,    46,    47,
      48,    49,    50,    51,    52,    53,   173,    47,    48,    49,
      50,    51,    52,    53,    59,    60,    61,     9,     9,   188,
     189,   121,    54,    20,   122,    10,    10,    11,   196,    55,
      54,    21,   140,    56,   199,    22,    57,    55,    58,   110,
     111,    56,    23,   112,    57,    77,    58,   104,   105,   106,
     107,   108,   109,   154,   113,   114,    59,    60,    61,    37,
     102,   169,   103,    75,    59,    60,    61,   141,    84,    85,
      86,    87,    88,    89,   126,   127,    45,   178,   179,    90,
     128,   129,   130,   131,    74,   132,   133,    76,   101,   115,
     119,   120,   137,   187,    91,    92,    93,    94,    95,    96,
     138,   139,   195,   142,   165,   166,    97,    98,   167,    99,
     170,   185,   192,   171,   172,   180,   181,   183,   182,   184,
      38,   191,   134,   135,   136,   198,   144,   197,   190,    78,
     177,   124,     0,   123,   125
};

static const yytype_int16 yycheck[] =
{
      20,    40,    44,     3,    12,     3,     3,     4,     5,     6,
       7,     8,     9,    10,     3,    54,    55,    14,    15,    16,
      17,    18,    19,    21,    22,    23,    24,    25,    26,    15,
      27,   145,    29,    30,    34,    34,     0,    34,    34,    33,
      36,    38,    41,    29,    41,    41,    43,    40,     3,     4,
       5,     6,     7,     8,     9,    10,     3,     4,     5,     6,
       7,     8,     9,    10,    61,    62,    63,     3,     3,   183,
     184,    32,    27,    40,    35,    11,    11,    13,   192,    34,
      27,     3,   121,    38,   198,     3,    41,    34,    43,    41,
      42,    38,    31,    45,    41,    30,    43,    57,    58,    59,
      60,    43,    44,   145,    61,    62,    61,    62,    63,    29,
      37,   150,    39,    41,    61,    62,    63,   137,    56,    57,
      58,    59,    60,    61,   102,   103,    35,   166,   167,    36,
     104,   105,   106,   107,    31,   108,   109,    27,    50,     3,
      31,    28,    40,   182,    51,    52,    53,    54,    55,    56,
      28,    32,   191,    29,    27,    27,    63,    64,    27,    66,
      31,   181,    20,    31,    31,    31,    40,    28,    31,    28,
      22,    31,   110,   111,   112,    28,   139,   192,   185,    44,
     165,   100,    -1,    91,   101
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    74,    75,    76,     0,    12,    77,    79,     3,     3,
      11,    13,    80,    81,    85,    87,    88,    33,    78,    40,
      40,     3,     3,    31,     3,    21,    22,    23,    24,    25,
      26,    91,    92,    93,    94,    95,    91,    29,    78,    34,
      36,    86,    96,     3,    89,    35,     3,     4,     5,     6,
       7,     8,     9,    10,    27,    34,    38,    41,    43,    61,
      62,    63,   114,   115,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,    31,    41,    27,    30,    81,    85,
      90,   114,   114,   127,   123,   123,   123,   123,   123,   123,
      36,    51,    52,    53,    54,    55,    56,    63,    64,    66,
     116,    50,    37,    39,    57,    58,    59,    60,    43,    44,
      41,    42,    45,    61,    62,     3,    82,    83,    84,    31,
      28,    32,    35,   118,   115,   119,   120,   120,   121,   121,
     121,   121,   122,   122,   123,   123,   123,    40,    28,    32,
     114,    91,    29,    99,    84,    97,     3,    14,    15,    16,
      17,    18,    19,    30,    85,    98,    99,   100,   102,   103,
     109,   111,   112,   113,   114,    27,    27,    27,   110,   114,
      31,    31,    31,     3,   104,   105,   106,   113,   114,   114,
      31,    40,    31,    28,    28,    91,   107,   114,    99,    99,
      86,    31,    20,   101,   108,   114,    99,   100,    28,    99
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    73,    74,    75,    76,    76,    77,    78,    78,    79,
      79,    80,    80,    80,    81,    82,    82,    83,    83,    84,
      85,    86,    86,    87,    88,    89,    89,    90,    90,    91,
      91,    91,    91,    91,    92,    93,    93,    93,    93,    93,
      94,    95,    96,    96,    97,    97,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    99,   100,   101,   101,   101,
     102,   103,   104,   104,   105,   105,   106,   107,   107,   108,
     108,   109,   110,   110,   111,   112,   113,   114,   115,   115,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   117,
     117,   118,   118,   119,   119,   119,   120,   120,   120,   120,
     120,   121,   121,   121,   122,   122,   122,   122,   123,   123,
     123,   123,   123,   123,   123,   124,   124,   124,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   125,   126,   127,
     127,   127
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     2,     0,     4,     3,     0,     2,
       0,     1,     1,     1,     8,     1,     0,     3,     1,     3,
       5,     2,     0,     1,     6,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     2,     2,     0,     2,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     6,     2,     2,     0,
       5,     9,     1,     0,     1,     1,     4,     1,     0,     1,
       0,     3,     1,     0,     2,     2,     2,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     1,     3,     3,     1,     3,     3,     3,     3,
       1,     3,     3,     1,     3,     3,     3,     1,     2,     2,
       2,     2,     2,     2,     1,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     3,     1,
       3,     0
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
#line 1355 "grammar.tab.c"
    break;

  case 3: /* allowed_top_level: includes top_level_keywords_seq  */
#line 38 "grammar.y"
                                        { (yyval.node) = ast_allowed_top_level((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1361 "grammar.tab.c"
    break;

  case 4: /* includes: includes include  */
#line 42 "grammar.y"
                                       { (yyval.node) = ast_append_include((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1367 "grammar.tab.c"
    break;

  case 5: /* includes: %empty  */
#line 43 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1373 "grammar.tab.c"
    break;

  case 6: /* include: INCLUDE IDENTIFIER include_tail SEMICOLON  */
#line 47 "grammar.y"
                                                { (yyval.node) = ast_include((yyvsp[-2].sval), (yyvsp[-1].node)); }
#line 1379 "grammar.tab.c"
    break;

  case 7: /* include_tail: PERIOD IDENTIFIER include_tail  */
#line 51 "grammar.y"
                                       { (yyval.node) = ast_include_tail((yyvsp[-1].sval), (yyvsp[0].node)); }
#line 1385 "grammar.tab.c"
    break;

  case 8: /* include_tail: %empty  */
#line 52 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1391 "grammar.tab.c"
    break;

  case 9: /* top_level_keywords_seq: top_level_keywords_seq top_level_keywords  */
#line 56 "grammar.y"
                                                { (yyval.node) = ast_append_top_level((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1397 "grammar.tab.c"
    break;

  case 10: /* top_level_keywords_seq: %empty  */
#line 57 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1403 "grammar.tab.c"
    break;

  case 11: /* top_level_keywords: function_def  */
#line 61 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1409 "grammar.tab.c"
    break;

  case 12: /* top_level_keywords: struct_def  */
#line 62 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1415 "grammar.tab.c"
    break;

  case 13: /* top_level_keywords: variable_def  */
#line 63 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1421 "grammar.tab.c"
    break;

  case 14: /* function_def: PROGRAM TYPE_DECL type IDENTIFIER OPEN_PAREN param_list_opt CLOSE_PAREN code_block  */
#line 67 "grammar.y"
                                                                                         { (yyval.node) = ast_function_def((yyvsp[-5].node), (yyvsp[-4].sval), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1427 "grammar.tab.c"
    break;

  case 15: /* param_list_opt: param_list  */
#line 71 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1433 "grammar.tab.c"
    break;

  case 16: /* param_list_opt: %empty  */
#line 72 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1439 "grammar.tab.c"
    break;

  case 17: /* param_list: param_list COMMA param  */
#line 76 "grammar.y"
                                       { (yyval.node) = ast_append_param((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1445 "grammar.tab.c"
    break;

  case 18: /* param_list: param  */
#line 77 "grammar.y"
                                       { (yyval.node) = ast_single_param((yyvsp[0].node)); }
#line 1451 "grammar.tab.c"
    break;

  case 19: /* param: IDENTIFIER TYPE_DECL type  */
#line 81 "grammar.y"
                                       { (yyval.node) = ast_param((yyvsp[-2].sval), (yyvsp[0].node)); }
#line 1457 "grammar.tab.c"
    break;

  case 20: /* variable_def: IDENTIFIER TYPE_DECL type variable_init_opt SEMICOLON  */
#line 85 "grammar.y"
                                                            { (yyval.node) = ast_variable_def((yyvsp[-4].sval), (yyvsp[-2].node), (yyvsp[-1].node)); }
#line 1463 "grammar.tab.c"
    break;

  case 21: /* variable_init_opt: EQUAL expression  */
#line 89 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1469 "grammar.tab.c"
    break;

  case 22: /* variable_init_opt: %empty  */
#line 90 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1475 "grammar.tab.c"
    break;

  case 23: /* struct_def: struct  */
#line 94 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1481 "grammar.tab.c"
    break;

  case 24: /* struct: STRUCT IDENTIFIER OPEN_BRACE struct_members CLOSE_BRACE SEMICOLON  */
#line 98 "grammar.y"
                                                                        { (yyval.node) = ast_struct((yyvsp[-4].sval), (yyvsp[-2].node)); }
#line 1487 "grammar.tab.c"
    break;

  case 25: /* struct_members: struct_members struct_member  */
#line 102 "grammar.y"
                                        { (yyval.node) = ast_append_struct_member((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1493 "grammar.tab.c"
    break;

  case 26: /* struct_members: %empty  */
#line 103 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1499 "grammar.tab.c"
    break;

  case 27: /* struct_member: variable_def  */
#line 107 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1505 "grammar.tab.c"
    break;

  case 28: /* struct_member: function_def  */
#line 108 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1511 "grammar.tab.c"
    break;

  case 29: /* type: default_type  */
#line 113 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1517 "grammar.tab.c"
    break;

  case 30: /* type: user_type  */
#line 114 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1523 "grammar.tab.c"
    break;

  case 31: /* type: pointer_type  */
#line 115 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1529 "grammar.tab.c"
    break;

  case 32: /* type: array_type  */
#line 116 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1535 "grammar.tab.c"
    break;

  case 33: /* type: VOID  */
#line 117 "grammar.y"
                                       { (yyval.node) = ast_type_void(); }
#line 1541 "grammar.tab.c"
    break;

  case 34: /* array_type: type OPEN_BRACKET CLOSE_BRACKET  */
#line 121 "grammar.y"
                                       { (yyval.node) = ast_type_array((yyvsp[-2].node), -1); }
#line 1547 "grammar.tab.c"
    break;

  case 35: /* default_type: INT  */
#line 125 "grammar.y"
                                       { (yyval.node) = ast_type_int(); }
#line 1553 "grammar.tab.c"
    break;

  case 36: /* default_type: FLOAT  */
#line 126 "grammar.y"
                                       { (yyval.node) = ast_type_float(); }
#line 1559 "grammar.tab.c"
    break;

  case 37: /* default_type: STRING  */
#line 127 "grammar.y"
                                       { (yyval.node) = ast_type_string(); }
#line 1565 "grammar.tab.c"
    break;

  case 38: /* default_type: BOOL  */
#line 128 "grammar.y"
                                       { (yyval.node) = ast_type_bool(); }
#line 1571 "grammar.tab.c"
    break;

  case 39: /* default_type: CHAR  */
#line 129 "grammar.y"
                                       { (yyval.node) = ast_type_char(); }
#line 1577 "grammar.tab.c"
    break;

  case 40: /* user_type: IDENTIFIER  */
#line 133 "grammar.y"
                                       { (yyval.node) = ast_type_user((yyvsp[0].sval)); }
#line 1583 "grammar.tab.c"
    break;

  case 41: /* pointer_type: type pointer_stars  */
#line 137 "grammar.y"
                                       { (yyval.node) = ast_type_pointer((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1589 "grammar.tab.c"
    break;

  case 42: /* pointer_stars: pointer_stars MUL  */
#line 141 "grammar.y"
                                       { (yyval.node) = ast_append_pointer_star((yyvsp[-1].node)); }
#line 1595 "grammar.tab.c"
    break;

  case 43: /* pointer_stars: %empty  */
#line 142 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1601 "grammar.tab.c"
    break;

  case 44: /* stmts: stmts statement  */
#line 146 "grammar.y"
                                       { (yyval.node) = ast_append_stmt((yyvsp[-1].node), (yyvsp[0].node)); }
#line 1607 "grammar.tab.c"
    break;

  case 45: /* stmts: %empty  */
#line 147 "grammar.y"
                                       { (yyval.node) = ast_empty(); }
#line 1613 "grammar.tab.c"
    break;

  case 46: /* statement: if_stmt  */
#line 151 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1619 "grammar.tab.c"
    break;

  case 47: /* statement: while_stmt  */
#line 152 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1625 "grammar.tab.c"
    break;

  case 48: /* statement: for_stmt  */
#line 153 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1631 "grammar.tab.c"
    break;

  case 49: /* statement: return_stmt  */
#line 154 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1637 "grammar.tab.c"
    break;

  case 50: /* statement: break_stmt  */
#line 155 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1643 "grammar.tab.c"
    break;

  case 51: /* statement: continue_stmt  */
#line 156 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1649 "grammar.tab.c"
    break;

  case 52: /* statement: expression_statement  */
#line 157 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1655 "grammar.tab.c"
    break;

  case 53: /* statement: code_block  */
#line 158 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1661 "grammar.tab.c"
    break;

  case 54: /* statement: variable_def  */
#line 159 "grammar.y"
                                       { (yyval.node) = (yyvsp[0].node); }
#line 1667 "grammar.tab.c"
    break;

  case 55: /* code_block: OPEN_BRACE stmts CLOSE_BRACE  */
#line 163 "grammar.y"
                                        { (yyval.node) = ast_code_block((yyvsp[-1].node)); }
#line 1673 "grammar.tab.c"
    break;

  case 56: /* if_stmt: IF OPEN_PAREN expression CLOSE_PAREN code_block else_opt  */
#line 167 "grammar.y"
                                                               { (yyval.node) = ast_if_stmt((yyvsp[-3].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 1679 "grammar.tab.c"
    break;

  case 57: /* else_opt: ELSE if_stmt  */
#line 171 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1685 "grammar.tab.c"
    break;

  case 58: /* else_opt: ELSE code_block  */
#line 172 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1691 "grammar.tab.c"
    break;

  case 59: /* else_opt: %empty  */
#line 173 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1697 "grammar.tab.c"
    break;

  case 60: /* while_stmt: WHILE OPEN_PAREN expression CLOSE_PAREN code_block  */
#line 177 "grammar.y"
                                                         { (yyval.node) = ast_while_stmt((yyvsp[-2].node), (yyvsp[0].node)); }
#line 1703 "grammar.tab.c"
    break;

  case 61: /* for_stmt: FOR OPEN_PAREN for_init_opt SEMICOLON for_cond_opt SEMICOLON for_iter_opt CLOSE_PAREN code_block  */
#line 182 "grammar.y"
                                                                                                       { (yyval.node) = ast_for_stmt((yyvsp[-6].node), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1709 "grammar.tab.c"
    break;

  case 62: /* for_init_opt: for_init  */
#line 186 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1715 "grammar.tab.c"
    break;

  case 63: /* for_init_opt: %empty  */
#line 187 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1721 "grammar.tab.c"
    break;

  case 64: /* for_init: variable_def_no_semi  */
#line 192 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1727 "grammar.tab.c"
    break;

  case 65: /* for_init: expression_statement  */
#line 193 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1733 "grammar.tab.c"
    break;

  case 66: /* variable_def_no_semi: IDENTIFIER TYPE_DECL type variable_init_opt  */
#line 197 "grammar.y"
                                                  { (yyval.node) = ast_variable_def((yyvsp[-3].sval), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 1739 "grammar.tab.c"
    break;

  case 67: /* for_cond_opt: expression  */
#line 201 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1745 "grammar.tab.c"
    break;

  case 68: /* for_cond_opt: %empty  */
#line 202 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1751 "grammar.tab.c"
    break;

  case 69: /* for_iter_opt: expression  */
#line 206 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1757 "grammar.tab.c"
    break;

  case 70: /* for_iter_opt: %empty  */
#line 207 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1763 "grammar.tab.c"
    break;

  case 71: /* return_stmt: RETURN expression_opt SEMICOLON  */
#line 211 "grammar.y"
                                        { (yyval.node) = ast_return_stmt((yyvsp[-1].node)); }
#line 1769 "grammar.tab.c"
    break;

  case 72: /* expression_opt: expression  */
#line 215 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1775 "grammar.tab.c"
    break;

  case 73: /* expression_opt: %empty  */
#line 216 "grammar.y"
                                        { (yyval.node) = ast_empty(); }
#line 1781 "grammar.tab.c"
    break;

  case 74: /* break_stmt: BREAK SEMICOLON  */
#line 220 "grammar.y"
                                        { (yyval.node) = ast_break_stmt(); }
#line 1787 "grammar.tab.c"
    break;

  case 75: /* continue_stmt: CONTINUE SEMICOLON  */
#line 224 "grammar.y"
                                        { (yyval.node) = ast_continue_stmt(); }
#line 1793 "grammar.tab.c"
    break;

  case 76: /* expression_statement: expression SEMICOLON  */
#line 228 "grammar.y"
                                        { (yyval.node) = ast_expr_stmt((yyvsp[-1].node)); }
#line 1799 "grammar.tab.c"
    break;

  case 77: /* expression: assignment  */
#line 232 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1805 "grammar.tab.c"
    break;

  case 78: /* assignment: logical_or assignment_op assignment  */
#line 236 "grammar.y"
                                          { (yyval.node) = ast_assignment((yyvsp[-2].node), (yyvsp[-1].node), (yyvsp[0].node)); }
#line 1811 "grammar.tab.c"
    break;

  case 79: /* assignment: logical_or  */
#line 237 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1817 "grammar.tab.c"
    break;

  case 80: /* assignment_op: EQUAL  */
#line 241 "grammar.y"
                                        { (yyval.node) = ast_assign_op("="); }
#line 1823 "grammar.tab.c"
    break;

  case 81: /* assignment_op: ADD_EQUALS  */
#line 242 "grammar.y"
                                        { (yyval.node) = ast_assign_op("+="); }
#line 1829 "grammar.tab.c"
    break;

  case 82: /* assignment_op: SUB_EQUALS  */
#line 243 "grammar.y"
                                        { (yyval.node) = ast_assign_op("-="); }
#line 1835 "grammar.tab.c"
    break;

  case 83: /* assignment_op: MUL_EQUALS  */
#line 244 "grammar.y"
                                        { (yyval.node) = ast_assign_op("*="); }
#line 1841 "grammar.tab.c"
    break;

  case 84: /* assignment_op: DIV_EQUALS  */
#line 245 "grammar.y"
                                        { (yyval.node) = ast_assign_op("/="); }
#line 1847 "grammar.tab.c"
    break;

  case 85: /* assignment_op: MOD_EQUALS  */
#line 246 "grammar.y"
                                        { (yyval.node) = ast_assign_op("%="); }
#line 1853 "grammar.tab.c"
    break;

  case 86: /* assignment_op: BITWISE_AND  */
#line 247 "grammar.y"
                                        { (yyval.node) = ast_assign_op("&="); }
#line 1859 "grammar.tab.c"
    break;

  case 87: /* assignment_op: BITWISE_OR  */
#line 248 "grammar.y"
                                        { (yyval.node) = ast_assign_op("|="); }
#line 1865 "grammar.tab.c"
    break;

  case 88: /* assignment_op: BITWISE_XOR  */
#line 249 "grammar.y"
                                        { (yyval.node) = ast_assign_op("^="); }
#line 1871 "grammar.tab.c"
    break;

  case 89: /* logical_or: logical_or OR logical_and  */
#line 253 "grammar.y"
                                        { (yyval.node) = ast_binary_op("||", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1877 "grammar.tab.c"
    break;

  case 90: /* logical_or: logical_and  */
#line 254 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1883 "grammar.tab.c"
    break;

  case 91: /* logical_and: logical_and AND equality  */
#line 258 "grammar.y"
                                        { (yyval.node) = ast_binary_op("&&", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1889 "grammar.tab.c"
    break;

  case 92: /* logical_and: equality  */
#line 259 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1895 "grammar.tab.c"
    break;

  case 93: /* equality: equality EQUALITY comparison  */
#line 263 "grammar.y"
                                        { (yyval.node) = ast_binary_op("==", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1901 "grammar.tab.c"
    break;

  case 94: /* equality: equality NOT_EQUALS comparison  */
#line 264 "grammar.y"
                                        { (yyval.node) = ast_binary_op("!=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1907 "grammar.tab.c"
    break;

  case 95: /* equality: comparison  */
#line 265 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1913 "grammar.tab.c"
    break;

  case 96: /* comparison: comparison LESS additive  */
#line 269 "grammar.y"
                                        { (yyval.node) = ast_binary_op("<", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1919 "grammar.tab.c"
    break;

  case 97: /* comparison: comparison GREATER additive  */
#line 270 "grammar.y"
                                        { (yyval.node) = ast_binary_op(">", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1925 "grammar.tab.c"
    break;

  case 98: /* comparison: comparison LESS_EQUALS additive  */
#line 271 "grammar.y"
                                        { (yyval.node) = ast_binary_op("<=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1931 "grammar.tab.c"
    break;

  case 99: /* comparison: comparison GREATER_EQUALS additive  */
#line 272 "grammar.y"
                                        { (yyval.node) = ast_binary_op(">=", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1937 "grammar.tab.c"
    break;

  case 100: /* comparison: additive  */
#line 273 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1943 "grammar.tab.c"
    break;

  case 101: /* additive: additive ADD multiplicative  */
#line 277 "grammar.y"
                                        { (yyval.node) = ast_binary_op("+", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1949 "grammar.tab.c"
    break;

  case 102: /* additive: additive SUB multiplicative  */
#line 278 "grammar.y"
                                        { (yyval.node) = ast_binary_op("-", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1955 "grammar.tab.c"
    break;

  case 103: /* additive: multiplicative  */
#line 279 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1961 "grammar.tab.c"
    break;

  case 104: /* multiplicative: multiplicative MUL unary  */
#line 283 "grammar.y"
                                        { (yyval.node) = ast_binary_op("*", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1967 "grammar.tab.c"
    break;

  case 105: /* multiplicative: multiplicative DIV unary  */
#line 284 "grammar.y"
                                        { (yyval.node) = ast_binary_op("/", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1973 "grammar.tab.c"
    break;

  case 106: /* multiplicative: multiplicative MOD unary  */
#line 285 "grammar.y"
                                        { (yyval.node) = ast_binary_op("%", (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1979 "grammar.tab.c"
    break;

  case 107: /* multiplicative: unary  */
#line 286 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 1985 "grammar.tab.c"
    break;

  case 108: /* unary: NOT unary  */
#line 291 "grammar.y"
                                        { (yyval.node) = ast_unary_op("!", (yyvsp[0].node)); }
#line 1991 "grammar.tab.c"
    break;

  case 109: /* unary: SUB unary  */
#line 292 "grammar.y"
                                        { (yyval.node) = ast_unary_op("-", (yyvsp[0].node)); }
#line 1997 "grammar.tab.c"
    break;

  case 110: /* unary: BITWISE_AND unary  */
#line 293 "grammar.y"
                                        { (yyval.node) = ast_unary_op("&", (yyvsp[0].node)); }
#line 2003 "grammar.tab.c"
    break;

  case 111: /* unary: MUL unary  */
#line 294 "grammar.y"
                                        { (yyval.node) = ast_unary_op("*", (yyvsp[0].node)); }
#line 2009 "grammar.tab.c"
    break;

  case 112: /* unary: ADD_ADD unary  */
#line 295 "grammar.y"
                                        { (yyval.node) = ast_unary_op("++", (yyvsp[0].node)); }
#line 2015 "grammar.tab.c"
    break;

  case 113: /* unary: SUB_SUB unary  */
#line 296 "grammar.y"
                                        { (yyval.node) = ast_unary_op("--", (yyvsp[0].node)); }
#line 2021 "grammar.tab.c"
    break;

  case 114: /* unary: postfix  */
#line 297 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 2027 "grammar.tab.c"
    break;

  case 115: /* postfix: postfix ADD_ADD  */
#line 301 "grammar.y"
                                        { (yyval.node) = ast_postfix_op("++", (yyvsp[-1].node)); }
#line 2033 "grammar.tab.c"
    break;

  case 116: /* postfix: postfix SUB_SUB  */
#line 302 "grammar.y"
                                        { (yyval.node) = ast_postfix_op("--", (yyvsp[-1].node)); }
#line 2039 "grammar.tab.c"
    break;

  case 117: /* postfix: primary  */
#line 303 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 2045 "grammar.tab.c"
    break;

  case 118: /* primary: IDENTIFIER  */
#line 306 "grammar.y"
                                        { (yyval.node) = ast_identifier((yyvsp[0].sval)); }
#line 2051 "grammar.tab.c"
    break;

  case 119: /* primary: INT_LITERAL  */
#line 307 "grammar.y"
                                        { (yyval.node) = ast_int_literal((yyvsp[0].ival)); }
#line 2057 "grammar.tab.c"
    break;

  case 120: /* primary: FLOAT_LITERAL  */
#line 308 "grammar.y"
                                        { (yyval.node) = ast_float_literal((yyvsp[0].fval)); }
#line 2063 "grammar.tab.c"
    break;

  case 121: /* primary: STRING_LITERAL  */
#line 309 "grammar.y"
                                        { (yyval.node) = ast_string_literal((yyvsp[0].sval)); }
#line 2069 "grammar.tab.c"
    break;

  case 122: /* primary: CHAR_LITERAL  */
#line 310 "grammar.y"
                                        { (yyval.node) = ast_char_literal((yyvsp[0].sval)[0]); }
#line 2075 "grammar.tab.c"
    break;

  case 123: /* primary: TRUE  */
#line 311 "grammar.y"
                                        { (yyval.node) = ast_true(); }
#line 2081 "grammar.tab.c"
    break;

  case 124: /* primary: FALSE  */
#line 312 "grammar.y"
                                        { (yyval.node) = ast_false(); }
#line 2087 "grammar.tab.c"
    break;

  case 125: /* primary: NULL_TOKEN  */
#line 313 "grammar.y"
                                        { (yyval.node) = ast_null(); }
#line 2093 "grammar.tab.c"
    break;

  case 126: /* primary: OPEN_PAREN expression CLOSE_PAREN  */
#line 314 "grammar.y"
                                        { (yyval.node) = (yyvsp[-1].node); }
#line 2099 "grammar.tab.c"
    break;

  case 127: /* primary: array_literal  */
#line 315 "grammar.y"
                                        { (yyval.node) = (yyvsp[0].node); }
#line 2105 "grammar.tab.c"
    break;

  case 128: /* array_literal: OPEN_BRACKET expr_list CLOSE_BRACKET  */
#line 319 "grammar.y"
                                           { (yyval.node) = ast_list((yyvsp[-1].node)); }
#line 2111 "grammar.tab.c"
    break;

  case 129: /* expr_list: expression  */
#line 323 "grammar.y"
                                        { (yyval.node) = ast_single_stmt((yyvsp[0].node)); }
#line 2117 "grammar.tab.c"
    break;

  case 130: /* expr_list: expr_list COMMA expression  */
#line 324 "grammar.y"
                                         { (yyval.node) = ast_append_stmt((yyvsp[-2].node), (yyvsp[0].node)); }
#line 2123 "grammar.tab.c"
    break;

  case 131: /* expr_list: %empty  */
#line 325 "grammar.y"
                                         { (yyval.node) = ast_empty(); }
#line 2129 "grammar.tab.c"
    break;


#line 2133 "grammar.tab.c"

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

#line 328 "grammar.y"



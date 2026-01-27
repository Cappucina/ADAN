/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_GRAMMAR_TAB_H_INCLUDED
# define YY_YY_GRAMMAR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENTIFIER = 258,              /* IDENTIFIER  */
    INT_LITERAL = 259,             /* INT_LITERAL  */
    FLOAT_LITERAL = 260,           /* FLOAT_LITERAL  */
    STRING_LITERAL = 261,          /* STRING_LITERAL  */
    CHAR_LITERAL = 262,            /* CHAR_LITERAL  */
    TRUE = 263,                    /* TRUE  */
    FALSE = 264,                   /* FALSE  */
    NULL_TOKEN = 265,              /* NULL_TOKEN  */
    PROGRAM = 266,                 /* PROGRAM  */
    INCLUDE = 267,                 /* INCLUDE  */
    STRUCT = 268,                  /* STRUCT  */
    FOR = 269,                     /* FOR  */
    IF = 270,                      /* IF  */
    WHILE = 271,                   /* WHILE  */
    RETURN = 272,                  /* RETURN  */
    BREAK = 273,                   /* BREAK  */
    CONTINUE = 274,                /* CONTINUE  */
    ELSE = 275,                    /* ELSE  */
    INT = 276,                     /* INT  */
    FLOAT = 277,                   /* FLOAT  */
    BOOL = 278,                    /* BOOL  */
    STRING = 279,                  /* STRING  */
    CHAR = 280,                    /* CHAR  */
    VOID = 281,                    /* VOID  */
    OPEN_PAREN = 282,              /* OPEN_PAREN  */
    CLOSE_PAREN = 283,             /* CLOSE_PAREN  */
    OPEN_BRACE = 284,              /* OPEN_BRACE  */
    CLOSE_BRACE = 285,             /* CLOSE_BRACE  */
    SEMICOLON = 286,               /* SEMICOLON  */
    COMMA = 287,                   /* COMMA  */
    PERIOD = 288,                  /* PERIOD  */
    OPEN_BRACKET = 289,            /* OPEN_BRACKET  */
    CLOSE_BRACKET = 290,           /* CLOSE_BRACKET  */
    EQUAL = 291,                   /* EQUAL  */
    EQUALITY = 292,                /* EQUALITY  */
    NOT = 293,                     /* NOT  */
    NOT_EQUALS = 294,              /* NOT_EQUALS  */
    TYPE_DECL = 295,               /* TYPE_DECL  */
    MUL = 296,                     /* MUL  */
    DIV = 297,                     /* DIV  */
    SUB = 298,                     /* SUB  */
    ADD = 299,                     /* ADD  */
    MOD = 300,                     /* MOD  */
    EXPONENT = 301,                /* EXPONENT  */
    QUOTE = 302,                   /* QUOTE  */
    APOSTROPHE = 303,              /* APOSTROPHE  */
    ELLIPSIS = 304,                /* ELLIPSIS  */
    AND = 305,                     /* AND  */
    OR = 306,                      /* OR  */
    MUL_EQUALS = 307,              /* MUL_EQUALS  */
    DIV_EQUALS = 308,              /* DIV_EQUALS  */
    SUB_EQUALS = 309,              /* SUB_EQUALS  */
    ADD_EQUALS = 310,              /* ADD_EQUALS  */
    MOD_EQUALS = 311,              /* MOD_EQUALS  */
    GREATER = 312,                 /* GREATER  */
    LESS = 313,                    /* LESS  */
    GREATER_EQUALS = 314,          /* GREATER_EQUALS  */
    LESS_EQUALS = 315,             /* LESS_EQUALS  */
    ADD_ADD = 316,                 /* ADD_ADD  */
    SUB_SUB = 317,                 /* SUB_SUB  */
    BITWISE_AND = 318,             /* BITWISE_AND  */
    BITWISE_OR = 319,              /* BITWISE_OR  */
    BITWISE_NOT = 320,             /* BITWISE_NOT  */
    BITWISE_XOR = 321,             /* BITWISE_XOR  */
    BITWISE_NAND = 322,            /* BITWISE_NAND  */
    BITWISE_NOR = 323,             /* BITWISE_NOR  */
    BITWISE_XNOR = 324,            /* BITWISE_XNOR  */
    BITWISE_ZERO_FILL_LEFT_SHIFT = 325, /* BITWISE_ZERO_FILL_LEFT_SHIFT  */
    BITWISE_SIGNED_RIGHT_SHIFT = 326, /* BITWISE_SIGNED_RIGHT_SHIFT  */
    BITWISE_ZERO_FILL_RIGHT_SHIFT = 327 /* BITWISE_ZERO_FILL_RIGHT_SHIFT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 12 "grammar.y"

    int ival;
    float fval;
    char *sval;
    ASTNode *node;

#line 143 "grammar.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_GRAMMAR_TAB_H_INCLUDED  */

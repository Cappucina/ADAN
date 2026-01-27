// clang-format off
#include <stdlib.h>
#include <string.h>
#ifndef HAVE_STRNDUP
static char* portable_strndup(const char* s, size_t n)
{
    char* p = (char*)malloc(n + 1);
    if (p)
    {
        memcpy(p, s, n);
        p[n] = '\0';
    }
    return p;
}
#define strndup portable_strndup
#endif

// It's VERY important that these includes come in this order.
// The AST MUST be included before grammar.tab.h because grammar.tab.h
// uses ASTNode in the YYSTYPE union.
#include "../../include/ast.h"
#include "../../grammar.tab.h"

#include "lexer.h"
// clang-format on

Lexer* current_lexer = NULL;

extern YYSTYPE yylval;

int yylex(void)
{
    Token tk = lex(current_lexer);
    switch (tk.type)
    {
        case TOKEN_IDENTIFIER:
            yylval.sval = strndup(tk.lexeme, tk.length);
            return IDENTIFIER;
        case TOKEN_INT_LITERAL:
            yylval.ival = atoi(tk.lexeme);
            return INT_LITERAL;
        case TOKEN_FLOAT_LITERAL:
            yylval.fval = (float)atof(tk.lexeme);
            return FLOAT_LITERAL;
        case TOKEN_STRING:
            yylval.sval = strndup(tk.lexeme, tk.length);
            return STRING_LITERAL;
        case TOKEN_CHAR:
            yylval.sval = strndup(tk.lexeme, tk.length);
            return CHAR_LITERAL;
        case TOKEN_TRUE:
            return TRUE;
        case TOKEN_FALSE:
            return FALSE;
        case TOKEN_NULL:
            return NULL_TOKEN;
        case TOKEN_PROGRAM:
            return PROGRAM;
        case TOKEN_INCLUDE:
            return INCLUDE;
        case TOKEN_STRUCT:
            return STRUCT;
        case TOKEN_FOR:
            return FOR;
        case TOKEN_IF:
            return IF;
        case TOKEN_WHILE:
            return WHILE;
        case TOKEN_RETURN:
            return RETURN;
        case TOKEN_BREAK:
            return BREAK;
        case TOKEN_CONTINUE:
            return CONTINUE;
        case TOKEN_ELSE:
            return ELSE;
        case TOKEN_INT:
            return INT;
        case TOKEN_FLOAT:
            return FLOAT;
        case TOKEN_BOOL:
            return BOOL;
        case TOKEN_VOID:
            return VOID;
        case TOKEN_OPEN_PAREN:
            return OPEN_PAREN;
        case TOKEN_CLOSE_PAREN:
            return CLOSE_PAREN;
        case TOKEN_OPEN_BRACE:
            return OPEN_BRACE;
        case TOKEN_CLOSE_BRACE:
            return CLOSE_BRACE;
        case TOKEN_SEMICOLON:
            return SEMICOLON;
        case TOKEN_EQUAL:
            return EQUAL;
        case TOKEN_EQUALITY:
            return EQUALITY;
        case TOKEN_NOT:
            return NOT;
        case TOKEN_NOT_EQUALS:
            return NOT_EQUALS;
        case TOKEN_TYPE_DECL:
            return TYPE_DECL;
        case TOKEN_MUL:
            return MUL;
        case TOKEN_DIV:
            return DIV;
        case TOKEN_SUB:
            return SUB;
        case TOKEN_ADD:
            return ADD;
        case TOKEN_MOD:
            return MOD;
        case TOKEN_EXPONENT:
            return EXPONENT;
        case TOKEN_QUOTE:
            return QUOTE;
        case TOKEN_APOSTROPHE:
            return APOSTROPHE;
        case TOKEN_ELLIPSIS:
            return ELLIPSIS;
        case TOKEN_AND:
            return AND;
        case TOKEN_OR:
            return OR;
        case TOKEN_MUL_EQUALS:
            return MUL_EQUALS;
        case TOKEN_DIV_EQUALS:
            return DIV_EQUALS;
        case TOKEN_SUB_EQUALS:
            return SUB_EQUALS;
        case TOKEN_ADD_EQUALS:
            return ADD_EQUALS;
        case TOKEN_MOD_EQUALS:
            return MOD_EQUALS;
        case TOKEN_COMMA:
            return COMMA;
        case TOKEN_PERIOD:
            return PERIOD;
        case TOKEN_GREATER:
            return GREATER;
        case TOKEN_LESS:
            return LESS;
        case TOKEN_GREATER_EQUALS:
            return GREATER_EQUALS;
        case TOKEN_LESS_EQUALS:
            return LESS_EQUALS;
        case TOKEN_ADD_ADD:
            return ADD_ADD;
        case TOKEN_SUB_SUB:
            return SUB_SUB;
        case TOKEN_OPEN_BRACKET:
            return OPEN_BRACKET;
        case TOKEN_CLOSE_BRACKET:
            return CLOSE_BRACKET;
        case TOKEN_BITWISE_AND:
            return BITWISE_AND;
        case TOKEN_BITWISE_OR:
            return BITWISE_OR;
        case TOKEN_BITWISE_NOT:
            return BITWISE_NOT;
        case TOKEN_BITWISE_XOR:
            return BITWISE_XOR;
        case TOKEN_BITWISE_NAND:
            return BITWISE_NAND;
        case TOKEN_BITWISE_NOR:
            return BITWISE_NOR;
        case TOKEN_BITWISE_XNOR:
            return BITWISE_XNOR;
        case TOKEN_BITWISE_ZERO_FILL_LEFT_SHIFT:
            return BITWISE_ZERO_FILL_LEFT_SHIFT;
        case TOKEN_BITWISE_SIGNED_RIGHT_SHIFT:
            return BITWISE_SIGNED_RIGHT_SHIFT;
        case TOKEN_BITWISE_ZERO_FILL_RIGHT_SHIFT:
            return BITWISE_ZERO_FILL_RIGHT_SHIFT;
        case TOKEN_EOF:
            return 0;  // Bison expects 0 for EOF
        case TOKEN_ERROR:
            return -1;
        default:
            return tk.type;
    }
}

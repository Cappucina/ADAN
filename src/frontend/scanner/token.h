#include <stdio.h>

typedef enum TokenType {
    // Special
    TOKEN_EOF,
    TOKEN_IDENT,

    // Keywords
    TOKEN_FUN,
    TOKEN_IMPORT,
    TOKEN_CONST,

    // Types
    TOKEN_STRING,
    TOKEN_I32,
    TOKEN_I64,
    TOKEN_U32,
    TOKEN_U64,
    TOKEN_ISIZE,
    TOKEN_USIZE,

    // Symbols
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_EQUALS,
    TOKEN_QUOTE,
} TokenType;

typedef struct Token
{
    TokenType* type;
    size_t column;
    size_t line;
    char* lexeme; // Literal value of the token.
    size_t length;
} Token;

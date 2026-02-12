#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <stddef.h>

typedef enum TokenType
{
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
    TokenType type;
    size_t column;
    size_t line;
    char *lexeme; // Literal value of the token.
    size_t length;
} Token;

typedef struct Keyword
{
    const char *word;
    TokenType type;
} Keyword;

const Keyword keywords[] = {
    {"fun", TOKEN_FUN},
    {"import", TOKEN_IMPORT},
    {"const", TOKEN_CONST},

    // Types
    {"string", TOKEN_FUN},
    {"i32", TOKEN_IMPORT},
    {"i64", TOKEN_CONST},
    {"u32", TOKEN_FUN},
    {"u64", TOKEN_IMPORT},
};

void token_stream_free(Token *tokens);

Token* make_token(TokenType type, size_t column, size_t line, char *lexeme,
                  size_t length);

#endif
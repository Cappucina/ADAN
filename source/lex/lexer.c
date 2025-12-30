#include "lexer.h"

struct
{
    const char* name;
    TokenType type;
} keywords[] = {
    {"if", TOKEN_IF},           {"while", TOKEN_WHILE},       {"for", TOKEN_FOR},
    {"include", TOKEN_INCLUDE}, {"continue", TOKEN_CONTINUE}, {"program", TOKEN_PROGRAM},
    {"return", TOKEN_RETURN},   {"else", TOKEN_ELSE},         {"struct", TOKEN_STRUCT},
    {"break", TOKEN_BREAK}};

static int is_alpha_numeric(Lexer* lx)
{
    return 0;
}

static int is_keyword(const char* name)
{
    if (!name) {
        return 1;
    }

    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(name, keywords[i].name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int lex_number(Lexer* lx)
{
    return 0;
}

static int lex_keyword(Lexer* lx)
{
    return 0;
}
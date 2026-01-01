#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>

static ErrorList* error_list;

struct
{
    const char* name;
    TokenType type;
} keywords[] = {
    {"if", TOKEN_IF},           {"while", TOKEN_WHILE},       {"for", TOKEN_FOR},
    {"include", TOKEN_INCLUDE}, {"continue", TOKEN_CONTINUE}, {"program", TOKEN_PROGRAM},
    {"return", TOKEN_RETURN},   {"else", TOKEN_ELSE},         {"struct", TOKEN_STRUCT},
    {"break", TOKEN_BREAK}};

static bool is_alpha_numeric(char c)
{
    return (isalnum(c) || c == '_');
}

static bool is_keyword(const char* name)
{
    if (!name) return true;
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(name, keywords[i].name) == 0) {
            return true;
        }
    }

    return false;
}

Lexer* create_lexer(const char* source, ErrorList* el)
{
    Lexer* new = (Lexer*)malloc(sizeof(Lexer));
    if (!new) return NULL;

    new->source = source;
    new->position = 0;
    new->line = 1;
    new->column = 1;
    new->length = strlen(source);
    error_list = el;
    return new;
}

void free_lexer(Lexer* lex)
{
    if (lex) free(lex);
    return;
}

char peek_char(Lexer* lex)
{
    if (lex->position >= lex->length) {
        return '\0';
    }
    return lex->source[lex->position];
}

char next_char(Lexer* lex)
{
    char current = lex->source[lex->position++];
    if (current == '\n') {
        lex->line++;
        lex->column = 1;
    }
    else {
        lex->column++;
    }

    return current;
}

Token create_token(Lexer* lex, const char* lexeme, size_t start, size_t length, TokenType type)
{
    Token tk = {.lexeme = lexeme,
                .start = start,
                .length = length,
                .line = lex->line,
                .column = lex->column,
                .type = type};
    return tk;
}

Token next_token(Lexer* lex)
{

}

/**
 *
 * Tokenize keywords ("include", "if", "program", etc.),
 *  symbols ("::", "&&", "||", etc.), and more.
 *
 * Turning literal forms into their token version, then
 *  group them into an array to be handled by the parser.
 */
static Token lex()
{
}
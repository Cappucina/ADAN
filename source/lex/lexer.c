#include "lexer.h"

#include <stdlib.h>

void lexer_init(Lexer* lexer)
{
    lexer->token = TOKEN_EOF;
    lexer->line = 1;
    lexer->column = 1;
    lexer->start = NULL;
    lexer->length = 0;
}

static void skip_whitespace(const char** input, size_t* line, size_t* column)
{
    while (**input == ' ' || **input == '\t' || **input == '\n' || **input == '\r')
    {
        if (**input == '\n')
        {
            (*line)++;
            *column = 1;
        }
        else
        {
            (*column)++;
        }
        (*input)++;
    }
}

/**
 * 
 * @todo Implement lexer_advance to return the next token from the input.
 */
Token* lexer_advance(Lexer* lexer, const char** input)
{
    return NULL;
}

void lexer_free(Lexer* lexer)
{
    if (!lexer)
        return;
    free(lexer);
}
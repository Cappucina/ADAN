#ifndef SYNTAX_H
#define SYNTAX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ast.h"
#include "diagnostic.h"
#include "../lex/lexer.h"

typedef struct
{
    Token* tokens;
    uint32_t current;
    uint32_t count;
    ErrorList* errors;
    bool panic;
} TokenStream;

typedef struct
{
    ASTNode* root;
    ErrorList* errors;
} ParseResult;

TokenStream* token_stream_create(Token* tokens, uint32_t count, ErrorList* errors);

void token_stream_free(TokenStream* stream);

ParseResult* syntax_analyze(TokenStream* stream);

#endif
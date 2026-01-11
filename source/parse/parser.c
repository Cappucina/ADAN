#include "parser.h"

#include <stdlib.h>

TokenStream* token_stream_create(Token* tokens, uint32_t count, ErrorList* errors)
{
    TokenStream* stream = (TokenStream*)malloc(sizeof(TokenStream));
    if (!stream) return NULL;

    stream->current = 0;
    stream->count = count;
    stream->tokens = tokens;
    stream->errors = errors;
    stream->panic = false;

    return stream;
}

void token_stream_free(TokenStream* stream)
{
    if (!stream) return;
    free(stream);
}

ParseResult* syntax_analyze(TokenStream* stream)
{
    if (!stream) return NULL;

    ParseResult* result = (ParseResult*)malloc(sizeof(ParseResult));
    if (!result) return NULL;

    result->root = NULL;
    result->errors = stream->errors;

    return result;
}
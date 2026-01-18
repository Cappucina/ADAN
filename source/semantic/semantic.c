#include "semantic.h"

#include <stdlib.h>

Parser* create_semantic(Buffer* token_buffer, ErrorList* errors)
{
    if (!token_buffer || !token_buffer->data)
    {
        return NULL;
    }

    Parser* parser = (Parser*)malloc(sizeof(Parser));

    if (!parser)
    {
        return NULL;
    }

    parser->current = 0;
    parser->count = token_buffer->count;
    parser->tokens = token_buffer->data;
    parser->errors = errors;
    parser->panic = false;

    return parser;
}

void free_semantic(Parser* parser)
{
    if (parser) free(parser);
}

void semantic_analysis(Parser* parser)
{
}

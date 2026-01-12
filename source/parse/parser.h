#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../lex/lexer.h"
#include "ast.h"
#include "buffer.h"
#include "diagnostic.h"

typedef struct Parser
{
    Token* tokens;
    size_t current;
    size_t count;
    ErrorList* errors;
    bool panic;
} Parser;

Token peek(Parser* parser);
Token advance(Parser* parser);
Token current_token(Parser* parser);
bool match(Parser* parser, uint32_t ct, ...);
Parser* create_parser(Buffer* token_buffer, ErrorList* errors);

void free_parser(Parser* parser);

#endif

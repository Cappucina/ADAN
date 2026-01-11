#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ast.h"
#include "diagnostic.h"
#include "lexer.h"

typedef struct Parser
{
    Token* tokens;
    uint32_t current;
    uint32_t count;
    ErrorList* errors;
    bool panic;
} Parser;

Parser* create_parser(Token* tokens, uint32_t count, ErrorList* errors);

void free_parser();

#endif
#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ast.h"
#include "buffer.h"
#include "common.h"
#include "diagnostic.h"
#include "lex/lexer.h"

Token peek(Analyzer* parser);

Token advance(Analyzer* parser);

Token current_token(Analyzer* parser);

bool match(Analyzer* parser, TokenType expected);

Analyzer* create_parser(Buffer* token_buffer, ErrorList* errors);

void parse(Analyzer* analyzer);

void free_parser(Analyzer* analyzer);

void free_ast(ASTNode* node);

#endif

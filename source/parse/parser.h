#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../lex/lexer.h"
#include "ast.h"
#include "buffer.h"
#include "common.h"
#include "diagnostic.h"

void synchronize(Parser* parser);

Token peek(Parser* parser);

Token advance(Parser* parser);

Token current_token(Parser* parser);

bool match(Parser* parser, TokenType expected);

Parser* create_parser(Buffer* token_buffer, ErrorList* errors);

ASTNode* parse(Parser* parser);

void free_parser(Parser* parser);

void free_ast(ASTNode* node);

ASTNode* parse_declaration(Parser* parser);

#endif

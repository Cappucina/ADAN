#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lex/lexer.h"

#include "ast.h"
#include "buffer.h"
#include "diagnostic.h"
#include "common.h"

Token peek(Analyzer* parser);
Token advance(Analyzer* parser);
Token current_token(Analyzer* parser);
// bool match(Analyzer* parser, uint32_t ct, ...);
bool match(Analyzer* parser, TokenType expected);


Analyzer* create_parser(Buffer* token_buffer, ErrorList* errors);
void free_parser(Analyzer* analyzer);

Token peek(Analyzer* parser);

Token advance(Analyzer* parser);

Token current_token(Analyzer* parser);

bool match(Analyzer* parser, TokenType expected);

Token* expect(Analyzer* parser, TokenType expected, const char* message);

#endif


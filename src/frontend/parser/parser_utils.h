#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include "parser.h"

// Symbol Table stuff

void parser_declare_variable(Parser* parser, const char* name, const char* type, unsigned int size);

void parser_declare_function(Parser* parser, const char* name, const char* return_type);

void parser_use_symbol(Parser* parser, const char* name);

void parser_enter_scope(Parser* parser);

void parser_exit_scope(Parser* parser);

bool parser_symbol_exists(Parser* parser, const char* name);

// Token lookahead stuff

void advance_token(Parser* parser);

Token* peek_current(Parser* parser);

Token* peek_lookahead1(Parser* parser);

Token* peek_lookahead2(Parser* parser);

bool match_current(Parser* parser, TokenType type);

// Error handling and recovery stuff

void error_expected(Parser* parser, const char* expected);

void error_undefined_symbol(Parser* parser, const char* name);

void enter_recovery_mode(Parser* parser);

void exit_recovery_mode(Parser* parser);

#endif
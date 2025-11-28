#ifndef LEXER_SRC_H
#define LEXER_SRC_H

#include "lexer.h"

Lexer* create_lexer(const char *src);

Token* make_token(Lexer *lexer, TokenType type, const char *texts[], int count);

char* capture_word(Lexer *lexer);

Token* next_token(Lexer *lexer);

void free_token(Token *token);

void print_token(Token *token);

#endif
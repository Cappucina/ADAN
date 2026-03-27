#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"
#include <stdbool.h>

typedef struct Scanner
{
	char* source;
	size_t start;
	size_t position;
	size_t length;
	size_t column;
	size_t line;
	int interp_depth;
	int brace_depth;
	char in_string_quote;
	bool emit_interp_start;
	char quote_stack[8];
	int quote_depth;
	int interp_brace_stack[8];
} Scanner;

Scanner* scanner_init(char* source);

void scanner_free(Scanner* scanner);

Token* scan_next_token(Scanner* scanner);

#endif
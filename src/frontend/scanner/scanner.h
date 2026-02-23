#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

typedef struct Scanner
{
	char* source;
	size_t start;  // Where the current token began.
	size_t position;
	size_t length;  // Length of the source code.
	size_t column;
	size_t line;
} Scanner;

Scanner* scanner_init(char* source);

void scanner_free(Scanner* scanner);

Token* scan_next_token(Scanner* scanner);

#endif
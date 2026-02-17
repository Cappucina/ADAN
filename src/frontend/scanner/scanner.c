#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "../../macros.h"
#include "scanner.h"

const Keyword keywords[] = {
    {"fun", TOKEN_FUN},
    {"import", TOKEN_IMPORT},
    {"const", TOKEN_CONST},

    // Types
    {"string", TOKEN_FUN},
    {"i32", TOKEN_IMPORT},
    {"i64", TOKEN_CONST},
    {"u32", TOKEN_FUN},
    {"u64", TOKEN_IMPORT},
};

Scanner* scanner_init(char* source)
{
	Scanner* scanner = (Scanner*)calloc(1, sizeof(Scanner));
	if (!scanner) {
		printf("No memory left to allocate for a new scanner! (Error)");
		return NULL;
	}
	scanner->position = 0;
	scanner->source = source;
	scanner->length = strlen(source);
	scanner->start = 0;
	scanner->column = 1;
	scanner->line = 1;
	return scanner;
}

void scanner_free(Scanner* scanner)
{
	if (!scanner) {
		printf("No scanner provided; nothing to free! (Error)");
		return;
	}
	free(scanner);
}

// Inline functions and stuff

char peek(Scanner* scanner)
{
	if (scanner->position >= scanner->length)
		return '\0';
	return scanner->source[scanner->position];
}

char peek_next(Scanner* scanner)
{
	if (scanner->position + 1 >= scanner->length)
		return '\0';
	return scanner->source[scanner->position + 1];
}

char advance(Scanner* scanner)
{
	if (scanner->position >= scanner->length)
		return '\0';
	char curr = scanner->source[scanner->position++];
	if (curr == '\n') {
		scanner->line++;
		scanner->column = 1;
	} else {
		scanner->column++;
	}
	return curr;
}

bool is_at_end(Scanner* scanner)
{
	return scanner->position >= scanner->length;
}

//
// @todo Implement:
//                  `scan_next_token`
//                  `make_token`,    `scan_next_token`,
//

bool is_alpha(const char c)
{
	return isalpha(c) != 0;
}

bool is_digit(const char c)
{
	return isdigit(c) != 0;
}

bool is_alphanumeric(const char c)
{
	return isalnum(c) != 0;
}

bool is_whitespace(const char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

TokenType is_keyword(const char* keyword)
{
	for (size_t i = 0; i < ARRAY_LENGTH(keywords); i++) {
		if (strcmp(keyword, keywords[i].word) == 0) {
			return keywords[i].type;
		}
	}
	return TOKEN_IDENT;
}

void skip_spaces(Scanner* scanner)
{
	while (!is_at_end(scanner) && is_whitespace(peek(scanner))) {
		advance(scanner);
	}
}

// Actual scanning logic

Token* scanner_scan(Scanner* scanner)
{
	return NULL;
}
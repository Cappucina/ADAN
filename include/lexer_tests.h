#ifndef LEXER_TEST_H
#define LEXER_TEST_H

#include "lexer.h"

typedef struct {
	const char *input;
	TokenType expected_types[32];
	const char *expected_texts[32];
	int expected_count;
} LexerTest;

int run_lexer_test(LexerTest test);

int create_lexer_tests();

#endif

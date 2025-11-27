#ifndef LEXER_TEST_H
#define LEXER_TEST_H

#include "lexer.h"

typedef struct {
    const char *input;
    TokenType expected_types[32];
    const char *expected_texts[32];
    int expected_count;
} LexerTest;

void run_lexer_test(LexerTest test);

void create_lexer_tests();

#endif

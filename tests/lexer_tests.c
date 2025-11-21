#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "lexer_src.h"
#include "lexer_tests.h"

void run_lexer_test(LexerTest test) {
    Lexer *lexer = create_lexer(test.input);

    for (int i = 0; i < test.expected_count; i++) {
        Token *token = next_token(lexer);
        
        assert(token != NULL);
        if (token->type != test.expected_types[i]) {
            printf("FAIL: Token %d type mismatch: found %d, expected %d\n", i, token->type, test.expected_types[i]);
        }

        assert(token->type == test.expected_types[i]);
        if (strcmp(token->text, test.expected_texts[i]) != 0) {
            printf("FAIL: Token %d text mismatch: found '%s', expected '%s'\n", i, token->text, test.expected_texts[i]);
        }

        assert(strcmp(token->text, test.expected_texts[i]) == 0);
        free_token(token);
    }

    free(lexer);
    printf("PASS: '%s'\n", test.input);
}
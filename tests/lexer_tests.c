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
            printf("LEXER FAIL: Token %d type mismatch: found %d, expected %d\n", i, token->type, test.expected_types[i]);
        }

        assert(token->type == test.expected_types[i]);
        if (strcmp(token->text, test.expected_texts[i]) != 0) {
            printf("LEXER FAIL: Token %d text mismatch: found '%s', expected '%s'\n", i, token->text, test.expected_texts[i]);
        }

        assert(strcmp(token->text, test.expected_texts[i]) == 0);
        free_token(token);
    }

    free(lexer);
    // printf("LEXER PASS: '%s'\n", test.input);
}

void create_lexer_tests() {
    LexerTest tests[] = {
        { "x::int;",
            { 
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_SEMICOLON
            },
            {
                "x", "::", "int", ";"
            },
            3
        },
        { "y::float = 3.14;",
            {
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_FLOAT, TOKEN_ASSIGN,
                TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON
            },
            {
                "y", "::", "float", "=", "3.14", ";"
            },
            5
        },
        { "if (x > 0) { y = x; }",
            {
                TOKEN_IF, TOKEN_LPAREN, TOKEN_IDENTIFIER, TOKEN_GREATER,
                TOKEN_INT_LITERAL, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_IDENTIFIER,
                TOKEN_ASSIGN, TOKEN_IDENTIFIER, TOKEN_SEMICOLON, TOKEN_RBRACE
            },
            {
                "if","(","x",">","0",")","{",
                    "y","=","x",";",
                "}"
            },
            12
        },
        { "program::void test() { a::int = 5; b::float = 2.2; }",
            {
                TOKEN_PROGRAM, TOKEN_TYPE_DECL, TOKEN_VOID, TOKEN_IDENTIFIER,
                TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_IDENTIFIER,
                TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_ASSIGN, TOKEN_INT_LITERAL,
                TOKEN_SEMICOLON, TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_FLOAT,
                TOKEN_ASSIGN, TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON, TOKEN_RBRACE
            },
            {
                "program","::","void","test","(",")","{",
                    "a","::","int","=","5",";",
                    "b","::","float","=","2.2",";",
                "}"
            },
            17
        },
        { "a :: int = 10;",
            {
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_ASSIGN,
                TOKEN_INT_LITERAL, TOKEN_SEMICOLON
            },
            {
                "a","::","int","=","10",";"
            },
            6
        },
        { "b::float=0.001;",
            {
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_FLOAT, TOKEN_ASSIGN,
                TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON
            },
            {
                "b","::","float","=","0.001",";"
            },
            6
        },
        { "sum = a;",
            {
                TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_IDENTIFIER, TOKEN_SEMICOLON
            },
            {
                "sum","=","a",";"
            },
            4
        },
        { "program :: void main() { }",
            {
                TOKEN_PROGRAM, TOKEN_TYPE_DECL, TOKEN_VOID, TOKEN_IDENTIFIER,
                TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE
            },
            {
                "program","::","void","main","(",")","{","}"
            },
            8
        },
        { "x::int; y::int;",
            {
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_SEMICOLON,
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_SEMICOLON
            },
            {
                "x","::","int",";","y","::","int",";"
            },
            8
        },
        { "pi::float = 3.14159;",
            {
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_FLOAT, TOKEN_ASSIGN,
                TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON
            },
            {
                "pi","::","float","=","3.14159",";"
            },
            6
        },
        { "if (a > 0) a::int = 1;",
            {
                TOKEN_IF, TOKEN_LPAREN, TOKEN_IDENTIFIER, TOKEN_GREATER,
                TOKEN_INT_LITERAL, TOKEN_RPAREN, TOKEN_IDENTIFIER, TOKEN_TYPE_DECL,
                TOKEN_INT, TOKEN_ASSIGN, TOKEN_INT_LITERAL, TOKEN_SEMICOLON
            },
            {
                "if","(","a",">","0",")","a","::","int","=","1",";"
            },
            12
        },
        { "empty();",
            {
                TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_SEMICOLON
            },
            {
                "empty","(",")",";"
            },
            4
        },
        { "num::int = 0; val::float = 1.0;",
            {
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_ASSIGN,
                TOKEN_INT_LITERAL, TOKEN_SEMICOLON, TOKEN_IDENTIFIER, TOKEN_TYPE_DECL,
                TOKEN_FLOAT, TOKEN_ASSIGN, TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON
            },
            {
                "num","::","int","=","0",";","val","::","float","=","1.0",";"
            },
            12
        },
        { "a_b::int = 7;",
            {
                TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_ASSIGN,
                TOKEN_INT_LITERAL, TOKEN_SEMICOLON
            },
            {
                "a_b","::","int","=","7",";"
            },
            6
        }
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < num_tests; i++) {
        run_lexer_test(tests[i]);
    }
}
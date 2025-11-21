#include <stdio.h>
#include "lexer_tests.h"
#include "lexer.h"

int main() {
    LexerTest tests[] = {
        { "int x;", 
          { TOKEN_INT, TOKEN_IDENTIFIER, TOKEN_SEMICOLON },
          { "int", "x", ";" },
          3
        },
        { "float y -> 3.14;",
          { TOKEN_FLOAT, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON },
          { "float", "y", "->", "3.14", ";" },
          5
        },
        { "if (x > 0) { y -> x; }",
          { TOKEN_IF, TOKEN_LPAREN, TOKEN_IDENTIFIER, TOKEN_GREATER, TOKEN_INT_LITERAL, TOKEN_RPAREN,
            TOKEN_LBRACE, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_IDENTIFIER, TOKEN_SEMICOLON, TOKEN_RBRACE },
          { "if","(","x",">","0",")","{","y","->","x",";","}" },
          12
        },
        { "int program test() -> { int a -> 5; float b -> 2.2; }",
          { TOKEN_INT, TOKEN_PROGRAM, TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_ASSIGN, TOKEN_LBRACE,
            TOKEN_INT, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_INT_LITERAL, TOKEN_SEMICOLON,
            TOKEN_FLOAT, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON,
            TOKEN_RBRACE },
          { "int","program","test","(",")","->","{",
            "int","a","->","5",";",
            "float","b","->","2.2",";",
            "}" },
          17
        }
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < num_tests; i++) {
        run_lexer_test(tests[i]);
    }

    return 0;
}
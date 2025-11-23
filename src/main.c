#include <stdio.h>
#include "lexer_tests.h"
#include "lexer.h"

int main() {
    LexerTest tests[] = {
        { "x::int;", 
          { TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_SEMICOLON },
          { "x", "::", "int", ";" },
          3
        },
        { "y::float = 3.14;",
          { TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_FLOAT, TOKEN_ASSIGN, TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON },
          { "y", "::", "float", "=", "3.14", ";" },
          5
        },
        { "if (x > 0) { y = x; }",
          { TOKEN_IF, TOKEN_LPAREN, TOKEN_IDENTIFIER, TOKEN_GREATER, TOKEN_INT_LITERAL, TOKEN_RPAREN,
            TOKEN_LBRACE, TOKEN_IDENTIFIER, TOKEN_ASSIGN, TOKEN_IDENTIFIER, TOKEN_SEMICOLON, TOKEN_RBRACE },
          { "if","(","x",">","0",")","{","y","=","x",";","}" },
          12
        },
        { "program::void test() { a::int = 5; b::float = 2.2; }",
          { TOKEN_PROGRAM, TOKEN_TYPE_DECL, TOKEN_VOID, TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE,
            TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_INT, TOKEN_ASSIGN, TOKEN_INT_LITERAL, TOKEN_SEMICOLON,
            TOKEN_IDENTIFIER, TOKEN_TYPE_DECL, TOKEN_FLOAT, TOKEN_ASSIGN, TOKEN_FLOAT_LITERAL, TOKEN_SEMICOLON,
            TOKEN_RBRACE },
          { "program","::","void","test","(",")","{",
            "a","::","int","=","5",";",
            "b","::","float","=","2.2",";",
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
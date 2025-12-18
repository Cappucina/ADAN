#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "lexer_src.h"
#include "lexer_tests.h"
#include "logs.h"
#include "flags.h"

void run_lexer_test(LexerTest test) {
	Lexer *lexer = create_lexer(test.input);

	for (int i = 0; i < test.expected_count; i++) {
		Token *token = next_token(lexer);
		
		if (!token) {
			printf("LEXER FAIL: token %d is NULL\n", i);
			assert(token != NULL);
		}

		if (token->type != test.expected_types[i]) {
			printf(LexerErrorMessages[LEXER_TYPE_MISMATCH], i, token->type, test.expected_types[i]);
		}
		assert(token->type == test.expected_types[i]);

		if (strcmp(token->text, test.expected_texts[i]) != 0) {
			printf(LexerErrorMessages[LEXER_TEXT_MISMATCH], i, token->text, test.expected_texts[i]);
		}
		assert(strcmp(token->text, test.expected_texts[i]) == 0);

		free_token(token);
	}

	free(lexer);
}

void create_lexer_tests(CompilorFlags* flags) {
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
		{ "\"Hello ${test()}\";",
			{
				TOKEN_STRING, TOKEN_SEMICOLON
			},
			{
				"Hello ${test()}", ";"
			},
			2
		},
		{ "\"hello\\nworld\";",
			{
				TOKEN_STRING, TOKEN_SEMICOLON
			},
			{
				"hello\nworld", ";"
			},
			2
		},
		{ "\"a\\\"b\\\\c\";",
			{
				TOKEN_STRING, TOKEN_SEMICOLON
			},
			{
				"a\"b\\c", ";"
			},
			2
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
		,
		{ "print(\"${test()}\");",
			{
				TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_STRING, TOKEN_RPAREN, TOKEN_SEMICOLON
			},
			{
				"print", "(", "${test()}", ")", ";"
			},
			5
		},
	};

	int num_tests = sizeof(tests) / sizeof(tests[0]);
	for (int i = 0; i < num_tests; i++) {
		run_lexer_test(tests[i]);
	}
	
	if (flags->verbose) {
		FILE* fp = fopen("examples/stack-overflow-test.adn", "r");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			long sz = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			char* buf = malloc(sz + 1);
			fread(buf, 1, sz, fp);
			buf[sz] = '\0';
			fclose(fp);
			Lexer* lex = create_lexer(buf);
			Token* t;
			while ((t = next_token(lex)) && t->type != TOKEN_EOF) {
				print_token(t);
				free_token(t);
			}
			free(lex);
			free(buf);
		}
	}
}
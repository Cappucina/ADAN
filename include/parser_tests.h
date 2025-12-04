#ifndef PARSER_TEST_H
#define PARSER_TEST_H

#include "ast.h"

typedef struct {
	const char* input;
	ExpectedNode* expected_ast;
} ParserTest;

bool compare_ast(ASTNode* actual, ExpectedNode* expected);

ExpectedNode* create_expected_assignment();

ExpectedNode* create_expected_if();

ExpectedNode* create_expected_while();

ExpectedNode* create_expected_for();

ExpectedNode* create_expected_program();

void free_expected_ast(ExpectedNode* node);

void run_parser_test(const char* input, ExpectedNode* expected_ast);

void create_parser_tests();

#endif
#ifndef PARSER_TEST_H
#define PARSER_TEST_H

#include "ast.h"

typedef struct ExpectedNode {
    ASTNodeType type;
    const char* token_text;
    int child_count;
    struct ExpectedNode** children;
} ExpectedNode;

typedef struct {
    const char* input;
    ExpectedNode* expected_ast;
} ParserTest;

bool compare_ast(ASTNode* actual, ExpectedNode* expected);

ExpectedNode* create_expected_assignment();

ExpectedNode* create_expected_program();

ExpectedNode* create_expected_if();

void free_expected_ast(ExpectedNode* node);

void run_parser_test(const char* input, ExpectedNode* expected_ast);

void create_parser_tests();

#endif
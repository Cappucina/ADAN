#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "parser_tests.h"

bool compare_ast(ASTNode* actual, ExpectedNode* expected) {
    if (!actual || !expected) return false;
    if (actual->type != expected->type) return false;
    if (expected->token_text && strcmp(actual->token.text, expected->token_text) != 0) return false;
    if (actual->child_count != expected->child_count) return false;

    for (int i = 0; i < actual->child_count; i++) {
        if (!compare_ast(actual->children[i], expected->children[i])) return false;
    }

    return true;
}

ExpectedNode* create_expected_assignment() {
    ExpectedNode* id_node = malloc(sizeof(ExpectedNode));

    id_node->type = AST_IDENTIFIER;
    id_node->token_text = "x";
    id_node->child_count = 0;
    id_node->children = NULL;

    ExpectedNode* type_node = malloc(sizeof(ExpectedNode));

    type_node->type = AST_TYPE;
    type_node->token_text = "int";
    type_node->child_count = 0;
    type_node->children = NULL;

    ExpectedNode* literal_node = malloc(sizeof(ExpectedNode));

    literal_node->type = AST_LITERAL;
    literal_node->token_text = "5";
    literal_node->child_count = 0;
    literal_node->children = NULL;

    ExpectedNode* assignment_node = malloc(sizeof(ExpectedNode));

    assignment_node->type = AST_ASSIGNMENT;
    assignment_node->token_text = NULL;
    assignment_node->child_count = 3;
    assignment_node->children = malloc(sizeof(ExpectedNode*) * 3);
    assignment_node->children[0] = id_node;
    assignment_node->children[1] = type_node;
    assignment_node->children[2] = literal_node;

    return assignment_node;
}

ExpectedNode* create_expected_if() {
    ExpectedNode* condition = malloc(sizeof(ExpectedNode));

    condition->type = AST_IDENTIFIER;
    condition->token_text = "x";
    condition->child_count = 0;
    condition->children = NULL;

    ExpectedNode* body = malloc(sizeof(ExpectedNode));

    body->type = AST_BLOCK;
    body->token_text = NULL;
    body->child_count = 0;
    body->children = NULL;

    ExpectedNode* if_node = malloc(sizeof(ExpectedNode));

    if_node->type = AST_IF;
    if_node->token_text = NULL;
    if_node->child_count = 2;
    if_node->children = malloc(sizeof(ExpectedNode*) * 2);
    if_node->children[0] = condition;
    if_node->children[1] = body;

    return if_node;
}

ExpectedNode* create_expected_while() {
    ExpectedNode* left = malloc(sizeof(ExpectedNode));
    left->type = AST_IDENTIFIER;
    left->token_text = "x";
    left->child_count = 0;
    left->children = NULL;

    ExpectedNode* operator = malloc(sizeof(ExpectedNode));
    operator->type = AST_OPERATORS;
    operator->token_text = "<";
    operator->child_count = 0;
    operator->children = NULL;

    ExpectedNode* right = malloc(sizeof(ExpectedNode));
    right->type = AST_LITERAL;
    right->token_text = "10";
    right->child_count = 0;
    right->children = NULL;

    ExpectedNode* comparison = malloc(sizeof(ExpectedNode));
    comparison->type = AST_COMPARISON;
    comparison->token_text = NULL;
    comparison->child_count = 3;
    comparison->children = malloc(sizeof(ExpectedNode*) * 3);
    comparison->children[0] = left;
    comparison->children[1] = operator;
    comparison->children[2] = right;

    ExpectedNode* block = malloc(sizeof(ExpectedNode));
    block->type = AST_BLOCK;
    block->token_text = NULL;
    block->child_count = 0;
    block->children = NULL;
    
    ExpectedNode* while_node = malloc(sizeof(ExpectedNode));
    while_node->type = AST_WHILE;
    while_node->token_text = NULL;
    while_node->child_count = 2;
    while_node->children = malloc(sizeof(ExpectedNode*) * 2);
    while_node->children[0] = comparison;
    while_node->children[1] = block;

    return while_node;
}

ExpectedNode* create_expected_for() {
    ExpectedNode* literal = malloc(sizeof(ExpectedNode));

    literal->type = AST_LITERAL;
    literal->token_text = "10";
    literal->child_count = 0;
    literal->children = NULL;

    ExpectedNode* for_node = malloc(sizeof(ExpectedNode));

    for_node->type = AST_FOR;
    for_node->token_text = NULL;
    for_node->child_count = 1;
    for_node->children = malloc(sizeof(ExpectedNode*));
    for_node->children[0] = literal;

    return for_node;
}

ExpectedNode* create_expected_program() {
    ExpectedNode* assign = create_expected_assignment();

    ExpectedNode* type = malloc(sizeof(ExpectedNode));
    type->type = AST_TYPE;
    type->token_text = "void";
    type->child_count = 0;
    type->children = NULL;

    ExpectedNode* id = malloc(sizeof(ExpectedNode));
    id->type = AST_IDENTIFIER;
    id->token_text = "test";
    id->child_count = 0;
    id->children = NULL;

    ExpectedNode* params = malloc(sizeof(ExpectedNode));
    params->type = AST_PARAMS;
    params->token_text = NULL;
    params->child_count = 0;
    params->children = NULL;

    ExpectedNode* block = malloc(sizeof(ExpectedNode));
    block->type = AST_BLOCK;
    block->token_text = NULL;
    block->child_count = 1;
    block->children = malloc(sizeof(ExpectedNode*));
    block->children[0] = assign;

    ExpectedNode* program_node = malloc(sizeof(ExpectedNode));
    program_node->type = AST_PROGRAM;
    program_node->token_text = NULL;
    program_node->child_count = 4;
    program_node->children = malloc(sizeof(ExpectedNode*) * 4);
    program_node->children[0] = type;
    program_node->children[1] = id;
    program_node->children[2] = params;
    program_node->children[3] = block;

    return program_node;
}

void free_expected_ast(ExpectedNode* node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) {
        free_expected_ast(node->children[i]);
    }

    free(node->children);
    free(node);
}

void run_parser_test(const char* input, ExpectedNode* expected_ast) {
    Lexer* lexer = create_lexer(input);
    Parser parser;

    init_parser(&parser, lexer);

    ASTNode* ast = parse_statement(&parser);

    if (parser.error) {
        printf("PARSER FAIL: %s\n  Error: %s\n", input, parser.error_message);
    } else if (!ast) {
        printf("PARSER FAIL: %s\n  Error: parse_statement returned NULL\n", input);
    } else if (!compare_ast(ast, expected_ast)) {
        printf("PARSER FAIL: %s\n", input);
    } else {
        printf("PARSER PASS: %s\n", input);
    }

    if (ast) free_ast(ast);
    free_parser(&parser);
}

void create_parser_tests() {
    ParserTest tests[] = {
        { "x::int = 5;", create_expected_assignment() },
        { "if (x) {}", create_expected_if() },
        { "while (x < 10) {}", create_expected_while() },
        { "for (i::int = 0; i < 10; i++) {}", create_expected_for() },
        { "program::void test() { x::int = 5; }", create_expected_program() },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < num_tests; i++) {
        run_parser_test(tests[i].input, tests[i].expected_ast);
        free_expected_ast(tests[i].expected_ast);
    }
}

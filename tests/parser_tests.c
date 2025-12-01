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

//////////////////////////////////////////////////////////////////////
// 
//  Expected AST builder functions
// 

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

ExpectedNode* create_expected_program() { return NULL; }

ExpectedNode* create_expected_if() { return NULL; }

//////////////////////////////////////////////////////////////////////

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
    } else if (!compare_ast(ast, expected_ast)) {
        printf("PARSER FAIL: %s\n", input);
    } else {
        printf("PARSER PASS: %s\n", input);
    }

    free_ast(ast);
    free_parser(&parser);
    free_lexer(lexer);
}

void create_parser_tests() {
    ParserTest tests[] = {
        { "x::int = 5;", create_expected_assignment() },
        { "program::void test() { a::int = 5; b::float = 2.2; }", create_expected_program() },
        { "if (x > 0) { y = x; }", create_expected_if() },
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < num_tests; i++) {
        run_parser_test(tests[i].input, tests[i].expected_ast);
        free_expected_ast(tests[i].expected_ast);
    }
}
#ifndef AST_H
#define AST_H

#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include "semantic.h"

typedef struct ASTNode ASTNode;

typedef enum { NODE_ACTUAL, NODE_EXPECTED } NodeKind;

typedef enum {
    AST_PROGRAM,
    AST_IF,
    AST_WHILE,
    AST_INCLUDE,
    AST_BREAK,
    AST_RETURN,
    AST_FOR,
    AST_STATEMENT,
    AST_EXPRESSION,
    AST_ASSIGNMENT,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_FUNCTION_CALL,
    AST_BLOCK,
    AST_BINARY_OP,
    AST_BINARY_EXPR,
    AST_UNARY_OP,
    AST_UNARY_EXPR,
    AST_COMPARISON,
    AST_LOGICAL_OP,
    AST_INCREMENT_EXPR,
    AST_GROUPED_EXPR,
    AST_ARRAY_ACCESS,
    AST_MEMBER_ACCESS,
    AST_TERNARY_EXPR,
    AST_CAST_EXPR,
    AST_TYPE,
    AST_PARAMS,
    AST_OPERATORS,
    AST_ARRAY_LITERAL,
} ASTNodeType;

typedef struct ExpectedNode {
    ASTNodeType type;
    const char* token_text;
    TokenType token_type;
    int child_count;
    struct ExpectedNode** children;
} ExpectedNode;

static inline const char* node_type_to_string(ASTNodeType type) {
    switch (type) {
        case AST_PROGRAM: return "AST_PROGRAM";
        case AST_ASSIGNMENT: return "AST_ASSIGNMENT";
        case AST_IDENTIFIER: return "AST_IDENTIFIER";
        case AST_TYPE: return "AST_TYPE";
        case AST_LITERAL: return "AST_LITERAL";
        case AST_IF: return "AST_IF";
        case AST_WHILE: return "AST_WHILE";
        case AST_FOR: return "AST_FOR";
        case AST_BLOCK: return "AST_BLOCK";
        case AST_COMPARISON: return "AST_COMPARISON";
        case AST_INCREMENT_EXPR: return "AST_INCREMENT_EXPR";
        case AST_OPERATORS: return "AST_OPERATORS";
        case AST_PARAMS: return "AST_PARAMS";
        case AST_FUNCTION_CALL: return "AST_FUNCTION_CALL";
        case AST_BINARY_OP: return "AST_BINARY_OP";
        case AST_BINARY_EXPR: return "AST_BINARY_EXPR";
        case AST_UNARY_OP: return "AST_UNARY_OP";
        case AST_UNARY_EXPR: return "AST_UNARY_EXPR";
        case AST_ARRAY_LITERAL: return "AST_ARRAY_LITERAL";
        default: return "AST_UNKNOWN";
    }
}

struct ASTNode {
    ASTNodeType type;
    Token token;
    ASTNode** children;
    int child_count;
    Type annotated_type;
};

static inline void free_ast(ASTNode* node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) {
        free_ast(node->children[i]);
    }
    
    if (node->children) free(node->children);
    if (node->token.text) free(node->token.text);

    free(node);
}

static inline void print_ast(void* node, NodeKind kind, int indent) {
    if (!node) return;

    ASTNode* actual = NULL;
    ExpectedNode* expected = NULL;
    ASTNodeType type;
    const char* text = NULL;
    int child_count;
    void** children;

    if (kind == NODE_ACTUAL) {
        actual = (ASTNode*)node;
        type = actual->type;
        text = actual->token.text;
        child_count = actual->child_count;
        children = (void**)actual->children;
    } else {
        expected = (ExpectedNode*)node;
        type = expected->type;
        text = expected->token_text;
        child_count = expected->child_count;
        children = (void**)expected->children;
    }

    for (int i = 0; i < indent; i++) printf("  ");
    printf("%s", node_type_to_string(type));
    if (text) printf("('%s')", text);
    printf("\n");

    for (int i = 0; i < child_count; i++) {
        print_ast(children[i], kind, indent + 1);
    }
}

#endif
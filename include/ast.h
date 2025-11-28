#include "lexer.h"
#include <stdlib.h>

#ifndef AST_H
#define AST_H

typedef enum {
    //
    //  Keywords
    //
    AST_PROGRAM,
    AST_IF,
    AST_WHILE,
    AST_INCLUDE,
    AST_BREAK,
    AST_RETURN,

    //
    //  Groups
    //
    AST_STATEMENT,
    AST_EXPRESSION,
    AST_ASSIGNMENT,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_FUNCTION_CALL,
    AST_BLOCK,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_TYPE,
    AST_PARAMS,
    AST_OPERATORS,
    AST_COMPARISON,
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    Token token;

    struct ASTNode** children;    // An array of child nodes.
    int child_count;
} ASTNode;

static inline void free_ast(ASTNode* node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) {
        //
        //  Recursively call free_ast to continuously free children
        //
        free_ast(node->children[i]);
    }

    //
    //  Checking if it exists, if so, free the array itself.
    //
    if (node->children) free(node->children);
    if (node->token.text) free(node->token.text);

    free(node);
}

#endif

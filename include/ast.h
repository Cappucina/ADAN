#include "lexer.h"
#include <stdlib.h>

#ifndef AST_H
#define AST_H

typedef enum {
    //
    //  Keywords / Statements
    //
    AST_PROGRAM,
    AST_IF,
    AST_WHILE,
    AST_INCLUDE,
    AST_BREAK,
    AST_RETURN,
    AST_FOR,

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

    //
    //  Binary expressions
    //
    AST_BINARY_OP,      // Represents the operator itself (+, -, *, /, etc.)
    AST_BINARY_EXPR,    // The full binary expression node

    //
    //  Unary expressions
    //
    AST_UNARY_OP,       // The operator itself (-, !, ++, --)
    AST_UNARY_EXPR,     // The full unary expression node

    //
    //  Comparisons & logical
    //
    AST_COMPARISON,     // <, >, <=, >=, ==, !=
    AST_LOGICAL_OP,     // &&, ||

    //
    //  Increment / Decrement
    //
    AST_INCREMENT_EXPR, // Prefix or postfix ++/--

    //
    //  Grouping
    //
    AST_GROUPED_EXPR,   // Expressions inside parentheses

    //
    //  Arrays / members
    //
    AST_ARRAY_ACCESS,   // arr[index]
    AST_MEMBER_ACCESS,  // obj.field

    //
    //  Ternary / Conditional
    //
    AST_TERNARY_EXPR,   // cond ? expr1 : expr2

    //
    //  Type casts / conversions
    //
    AST_CAST_EXPR,      // (int)x, (float)y

    //
    //  Parameters / operators / types
    //
    AST_TYPE,
    AST_PARAMS,
    AST_OPERATORS,
    AST_ARRAY_LITERAL,
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

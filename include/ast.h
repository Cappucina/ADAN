#include "lexer.h"

#ifndef AST_H
#define AST_H
/*

        AST_PROGRAM
        ├─ AST_STATEMENT
        │   ├─ AST_ASSIGNMENT
        │   │   ├─ AST_IDENTIFIER
        │   │   └─ AST_EXPRESSION
        │   │       └─ AST_BINARY_OP
        │   │           ├─ AST_LITERAL / AST_IDENTIFIER
        │   │           └─ AST_LITERAL / AST_IDENTIFIER
        │   ├─ AST_IF
        │   │   ├─ AST_EXPRESSION (condition)
        │   │   └─ AST_BLOCK
        │   │       └─ AST_STATEMENT
        │   ├─ AST_WHILE
        │   │   ├─ AST_EXPRESSION (condition)
        │   │   └─ AST_BLOCK
        │   │       └─ AST_STATEMENT
        │   └─ AST_FUNCTION_CALL
        │       ├─ AST_IDENTIFIER (function name)
        │       └─ AST_EXPRESSION[] (arguments)
        ├─ AST_INCLUDE
        |─ AST_BREAK
        └─ AST_BLOCK
            └─ AST_STATEMENT

*/
typedef enum {
    // 
    //  Keywords
    // 
    AST_PROGRAM,
    AST_IF,
    AST_WHILE,
    AST_INCLUDE,
    AST_BREAK,
    
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
} ASTNodeType;

typedef struct {
    ASTNodeType type;
    Token token;
    struct ASTNode **children;    // An array of child nodes.
    int child_count;
} ASTNode;

#endif
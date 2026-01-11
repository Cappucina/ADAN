#ifndef AST_H
#define AST_H

typedef enum
{
    AST_IDENT,
    AST_EXPRESSION,
    AST_LITERAL,

    AST_PROGRAM,
    AST_FOR,
    AST_WHILE,
    AST_CONTINUE,
    AST_BREAK,
    AST_INCLUDE,
    AST_IF,
    AST_RETURN,
    AST_STRUCT,
    AST_ELSE,
} ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;

    struct ASTNode* left;
    struct ASTNode* right;

    int line, column;
} ASTNode;

#endif
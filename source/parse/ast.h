#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    AST_IDENT,
    AST_EXPRESSION,
    AST_LITERAL,
    AST_PROGRAM,
    AST_PROGRAM_CALL,
    AST_FOR,
    AST_WHILE,
    AST_CONTINUE,
    AST_BREAK,
    AST_INCLUDE,
    AST_IF,
    AST_RETURN,
    AST_STRUCT,
    AST_ELSE,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_BLOCK,
    AST_VARIABLE_DECL,
    AST_ASSIGNMENT,
    AST_TYPE,
    AST_ARRAY_TYPE,
    AST_STRING_LITERAL,
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_BOOL_LITERAL,
    AST_INDEX_ACCESS,
    AST_ELIF,
    AST_SWITCH,
    AST_CASE,
    AST_DEFAULT_CASE,
    AST_PARAM,
    AST_PARAM_LIST
} ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;

    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode** children;
    size_t child_count;

    int line, column;
    const char* file_name;

    union {
        int64_t int_value;
        double float_value;
        const char* string_value;
        bool bool_value;
    } value;

    TokenType op;

    struct Type* resolved_type;
} ASTNode;

#endif

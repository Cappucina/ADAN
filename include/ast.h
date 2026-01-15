#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../source/lex/lexer.h"

typedef enum
{
    AST_ROOT,
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
    AST_ELSE_IF,
    AST_SWITCH,
    AST_CASE,
    AST_DEFAULT_CASE,
    AST_PARAM,
    AST_PARAM_LIST
} ASTNodeType;

typedef struct ASTNode
{
    ASTNodeType type;

    int line, column;
    const char* file_name;

    union {
        struct
        {
            char* org;
            char** libs;
            uint32_t count;
        } include;

        struct
        {
            char* name;
        } ident;

        struct
        {
            int64_t value;
        } int_literal;

        struct
        {
            double value;
        } float_literal;

        struct
        {
            char* value;
        } string_literal;

        struct
        {
            bool value;
        } bool_literal;

        struct
        {
            char* op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary;

        struct
        {
            char* op;
            struct ASTNode* operand;
        } unary;

        struct
        {
            struct ASTNode** statements;
            size_t count;
        } block;

        struct
        {
            struct ASTNode** params;
            size_t count;
        } param_list;

        struct
        {
            char* name;
            struct ASTNode** members;
            size_t count;
        } struct_decl;

        struct
        {
            struct ASTNode* value;
        } return_stmt;
    } data;
} ASTNode;

void free_ast(ASTNode* node);

ASTNode* create_ast_node(ASTNodeType type);

ASTNode* create_ident_node(const char* name);

ASTNode* create_string_literal_node(const char* value);

ASTNode* create_binary_node(const char* op, ASTNode* left, ASTNode* right);

ASTNode* create_unary_node(const char* op, ASTNode* operand);

ASTNode* create_block_node(ASTNode** statements, size_t count);

ASTNode* create_param_list_node(ASTNode** params, size_t count);

ASTNode* create_struct_decl_node(const char* name, ASTNode** members, size_t count);

#endif

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lex/lexer.h"

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

    int line, column;
    const char* file_name;

    union {
        struct {
            char* name;
        } ident;

        struct {
            int64_t value;
        } int_literal;

        struct {
            double value;
        } float_literal;

        struct {
            char* value;
        } string_literal;

        struct {
            bool value;
        } bool_literal;

        struct {
            char* op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary;

        struct {
            char* op;
            struct ASTNode* operand;
        } unary;

        struct {
            struct ASTNode** statements;
            size_t count;
        } block;

        struct {
            struct ASTNode** params;
            size_t count;
        } param_list;

        struct {
            char* name;
            struct ASTNode** members;
            size_t count;
        } struct_decl;

    } data;

    // union {
    //     int64_t int_value;
    //     double float_value;
    //     const char* string_value;
    //     bool bool_value;
    // } value;

    // struct Type* resolved_type;
} ASTNode;

#endif

#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct ASTNode ASTNode;

typedef enum
{
    AST_EMPTY,
    AST_PROGRAM,
    AST_INCLUDE,
    AST_FUNCTION_DEF,
    AST_VARIABLE_DEF,
    AST_STRUCT_DEF,
    AST_STRUCT_MEMBER,
    AST_TYPE,
    AST_PARAM,
    AST_CODE_BLOCK,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_EXPR_STMT,
    AST_ASSIGNMENT,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_POSTFIX_OP,
    AST_IDENTIFIER,
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STRING_LITERAL,
    AST_CHAR_LITERAL,
    AST_TRUE,
    AST_FALSE,
    AST_NULL,
    AST_LIST,
} ASTNodeType;

struct ASTNode
{
    ASTNodeType type;
    void* data;
    ASTNode* next;
};

ASTNode* ast_empty(void);

ASTNode* ast_allowed_top_level(ASTNode* includes, ASTNode* top_levels);

ASTNode* ast_append_include(ASTNode* list, ASTNode* include);

ASTNode* ast_include(const char* name, ASTNode* tail);

ASTNode* ast_include_tail(const char* name, ASTNode* tail);

ASTNode* ast_append_top_level(ASTNode* list, ASTNode* top_level);

ASTNode* ast_append_param(ASTNode* list, ASTNode* param);

ASTNode* ast_single_param(ASTNode* param);

ASTNode* ast_param(const char* name, ASTNode* type);

ASTNode* ast_append_struct_member(ASTNode* list, ASTNode* member);

ASTNode* ast_struct(const char* name, ASTNode* members);

ASTNode* ast_type_void(void);

ASTNode* ast_type_int(void);

ASTNode* ast_type_float(void);

ASTNode* ast_type_string(void);

ASTNode* ast_type_bool(void);

ASTNode* ast_type_char(void);

ASTNode* ast_type_user(const char* name);

ASTNode* ast_type_pointer(ASTNode* base, ASTNode* stars);

ASTNode* ast_type_array(ASTNode* base, int size);

ASTNode* ast_append_pointer_star(ASTNode* stars);

ASTNode* ast_append_stmt(ASTNode* list, ASTNode* stmt);

ASTNode* ast_code_block(ASTNode* stmts);

ASTNode* ast_block(ASTNode* stmts);

ASTNode* ast_if_stmt(ASTNode* cond, ASTNode* then_block, ASTNode* else_block);

ASTNode* ast_while_stmt(ASTNode* cond, ASTNode* block);

ASTNode* ast_for_stmt(ASTNode* init, ASTNode* cond, ASTNode* iter, ASTNode* block);

ASTNode* ast_return_stmt(ASTNode* expr);

ASTNode* ast_break_stmt(void);

ASTNode* ast_continue_stmt(void);

ASTNode* ast_expr_stmt(ASTNode* expr);

ASTNode* ast_assignment(ASTNode* lhs, ASTNode* op, ASTNode* rhs);

ASTNode* ast_assign_op(const char* op);

ASTNode* ast_binary_op(const char* op, ASTNode* lhs, ASTNode* rhs);

ASTNode* ast_unary_op(const char* op, ASTNode* expr);

ASTNode* ast_postfix_op(const char* op, ASTNode* expr);

ASTNode* ast_identifier(const char* name);

ASTNode* ast_int_literal(int value);

ASTNode* ast_float_literal(double value);

ASTNode* ast_string_literal(const char* value);

ASTNode* ast_char_literal(char value);

ASTNode* ast_true(void);

ASTNode* ast_false(void);

ASTNode* ast_null(void);

ASTNode* ast_variable_def(const char* name, ASTNode* type, ASTNode* init);

ASTNode* ast_function_def(ASTNode* type, const char* name, ASTNode* params, ASTNode* body);

ASTNode* ast_single_stmt(ASTNode* stmt);

void ast_print(ASTNode* root);

void ast_free(ASTNode* root);

#endif
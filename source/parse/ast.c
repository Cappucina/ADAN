#include "ast.h"

ASTNode* ast_new(ASTNodeType type, size_t line, size_t column)
{
}

ASTNode* ast_int_literal(int64_t value, size_t line, size_t column)
{
}

ASTNode* ast_float_literal(double value, size_t line, size_t column)
{
}

ASTNode* ast_bool_literal(int value, size_t line, size_t column)
{
}

ASTNode* ast_string_literal(const char* value, size_t line, size_t column)
{
}

ASTNode* ast_char_literal(char value, size_t line, size_t column)
{
}

ASTNode* ast_identifier(const char* name, size_t line, size_t column)
{
}

ASTNode* ast_type(const char* name, size_t line, size_t column)
{
}

ASTNode* ast_binary_expr(ASTOperator op, ASTNode* left, ASTNode* right, size_t line, size_t column)
{
}

ASTNode* ast_unary_expr(ASTOperator op, ASTNode* operand, size_t line, size_t column)
{
}

ASTNode* ast_assignment_expr(ASTNode* target, ASTNode* value, size_t line, size_t column)
{
}

ASTNode* ast_call_expr(ASTNode* callee, ASTList* arguments, size_t line, size_t column)
{
}

ASTNode* ast_member_access(ASTNode* object, const char* member, size_t line, size_t column)
{
}

ASTNode* ast_array_access(ASTNode* object, ASTNode* index, size_t line, size_t column)
{
}

ASTNode* ast_variable_decl(const char* name, ASTNode* type, ASTNode* initializer, size_t line,
                           size_t column)
{
}

ASTNode* ast_block_stmt(ASTList* statements, size_t line, size_t column)
{
}

ASTNode* ast_if_stmt(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch, size_t line,
                     size_t column)
{
}

ASTNode* ast_while_stmt(ASTNode* condition, ASTNode* body, size_t line, size_t column)
{
}

ASTNode* ast_for_stmt(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body,
                      size_t line, size_t column)
{
}

ASTNode* ast_return_stmt(ASTNode* value, size_t line, size_t column)
{
}

ASTNode* ast_function_decl(const char* name, ASTNode* return_type, ASTList* parameters,
                           ASTBlockStmt body, size_t line, size_t column)
{
}

ASTList* ast_list_create(ASTNode** items, size_t count)
{
}

void ast_free(ASTNode* node)
{
}
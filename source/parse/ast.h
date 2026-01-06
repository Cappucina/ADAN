#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    size_t line;
    size_t column;
} SourceLocation;

typedef enum
{
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_BOOL_LITERAL,
    AST_STRING_LITERAL,
    AST_CHAR_LITERAL,
    AST_NULL_LITERAL,

    AST_IDENTIFIER,
    AST_TYPE,

    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_ASSIGNMENT_EXPR,
    AST_CALL_EXPR,
    AST_MEMBER_ACCESS_EXPR,
    AST_ARRAY_ACCESS_EXPR,

    AST_VARIABLE_DECL,
    AST_BLOCK_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_RETURN_STMT,
    AST_BREAK_STMT,
    AST_CONTINUE_STMT,

    AST_FUNCTION_DECL,

    AST_INCLUDE_DIRECTIVE,
    AST_LIST
} ASTNodeType;

typedef enum
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,

    OP_AND,
    OP_OR,

    OP_ASSIGN,

    OP_NEGATE,
    OP_NOT,

    OP_PRE_INC,
    OP_PRE_DEC,
    OP_POST_INC,
    OP_POST_DEC
} ASTOperator;

typedef struct ASTNode ASTNode;

typedef struct
{
    ASTNode** items;
    size_t count;
} ASTList;

typedef struct
{
    int64_t value;
} ASTIntLiteral;

typedef struct
{
    double value;
} ASTFloatLiteral;

typedef struct
{
    int value;
} ASTBoolLiteral;

typedef struct
{
    char* value;
} ASTStringLiteral;

typedef struct
{
    char value;
} ASTCharLiteral;

typedef struct
{
    char* name;
} ASTIdentifier;

typedef struct
{
    char* name;
} ASTType;

typedef struct
{
    ASTOperator op;
    ASTNode* left;
    ASTNode* right;
} ASTBinaryExpr;

typedef struct
{
    ASTOperator op;
    ASTNode* operand;
} ASTUnaryExpr;

typedef struct
{
    ASTNode* target;
    ASTNode* value;
} ASTAssignmentExpr;

typedef struct
{
    ASTNode* callee;
    ASTList* arguments;
} ASTCallExpr;

typedef struct
{
    ASTNode* object;
    char* member;
} ASTMemberAccessExpr;

typedef struct
{
    ASTNode* object;
    ASTNode* index;
} ASTArrayAccessExpr;

typedef struct
{
    char* name;
    ASTNode* type;
    ASTNode* initializer;
} ASTVariableDecl;

typedef struct
{
    ASTList* statements;
} ASTBlockStmt;

typedef struct
{
    ASTNode* condition;
    ASTNode* then_branch;
    ASTNode* else_branch;
} ASTIfStmt;

typedef struct
{
    ASTNode* condition;
    ASTNode* body;
} ASTWhileStmt;

typedef struct
{
    ASTNode* initializer;
    ASTNode* condition;
    ASTNode* increment;
    ASTNode* body;
} ASTForStmt;

typedef struct
{
    ASTNode* value;
} ASTReturnStmt;

typedef struct
{
    char* name;
    ASTNode* return_type;
    ASTList* parameters;
    ASTBlockStmt body;
} ASTFunctionDecl;

typedef struct
{
    char* path;
} ASTIncludeDirective;

struct ASTNode
{
    ASTNodeType type;
    SourceLocation loc;

    union {
        ASTIntLiteral int_literal;
        ASTFloatLiteral float_literal;
        ASTBoolLiteral bool_literal;
        ASTStringLiteral string_literal;
        ASTCharLiteral char_literal;

        ASTIdentifier identifier;
        ASTType type_node;

        ASTBinaryExpr binary_expr;
        ASTUnaryExpr unary_expr;
        ASTAssignmentExpr assignment_expr;
        ASTCallExpr call_expr;
        ASTMemberAccessExpr member_access;
        ASTArrayAccessExpr array_access;

        ASTVariableDecl variable_decl;
        ASTBlockStmt block_stmt;
        ASTIfStmt if_stmt;
        ASTWhileStmt while_stmt;
        ASTForStmt for_stmt;
        ASTReturnStmt return_stmt;

        ASTFunctionDecl function_decl;

        ASTIncludeDirective include;
        ASTList list;
    } as;
};

ASTNode* ast_new(ASTNodeType type, size_t line, size_t column);

ASTNode* ast_int_literal(int64_t value, size_t line, size_t column);

ASTNode* ast_float_literal(double value, size_t line, size_t column);

ASTNode* ast_bool_literal(int value, size_t line, size_t column);

ASTNode* ast_string_literal(const char* value, size_t line, size_t column);

ASTNode* ast_char_literal(char value, size_t line, size_t column);

ASTNode* ast_identifier(const char* name, size_t line, size_t column);

ASTNode* ast_type(const char* name, size_t line, size_t column);

ASTNode* ast_binary_expr(ASTOperator op, ASTNode* left, ASTNode* right, size_t line, size_t column);

ASTNode* ast_unary_expr(ASTOperator op, ASTNode* operand, size_t line, size_t column);

ASTNode* ast_assignment_expr(ASTNode* target, ASTNode* value, size_t line, size_t column);

ASTNode* ast_call_expr(ASTNode* callee, ASTList* arguments, size_t line, size_t column);

ASTNode* ast_member_access(ASTNode* object, const char* member, size_t line, size_t column);

ASTNode* ast_array_access(ASTNode* object, ASTNode* index, size_t line, size_t column);

ASTNode* ast_variable_decl(const char* name, ASTNode* type, ASTNode* initializer, size_t line,
                           size_t column);

ASTNode* ast_block_stmt(ASTList* statements, size_t line, size_t column);

ASTNode* ast_if_stmt(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch, size_t line,
                     size_t column);

ASTNode* ast_while_stmt(ASTNode* condition, ASTNode* body, size_t line, size_t column);

ASTNode* ast_for_stmt(ASTNode* initializer, ASTNode* condition, ASTNode* increment, ASTNode* body,
                      size_t line, size_t column);

ASTNode* ast_return_stmt(ASTNode* value, size_t line, size_t column);

ASTNode* ast_function_decl(const char* name, ASTNode* return_type, ASTList* parameters,
                           ASTBlockStmt body, size_t line, size_t column);

ASTList* ast_list_create(ASTNode** items, size_t count);

void ast_free(ASTNode* node);

#endif
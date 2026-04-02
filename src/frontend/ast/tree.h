#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <stdbool.h>

typedef enum ASTNodeType
{
	AST_PROGRAM,
	AST_FUNCTION_DECLARATION,
	AST_VARIABLE_DECLARATION,
	AST_TYPE_DECLARATION,
	AST_IMPORT_STATEMENT,
	AST_LINK_DIRECTIVE,
	AST_IF_STATEMENT,
	AST_PARAMETER,
	AST_BLOCK,
	AST_CALL,
	AST_IDENTIFIER,
	AST_STRING_LITERAL,
	AST_INTERPOLATED_STRING,
	AST_NUMBER_LITERAL,
	AST_TYPE,
	AST_RETURN_STATEMENT,
	AST_EXPRESSION_STATEMENT,
	AST_WHILE_STMT,
	AST_FOR_STMT,
	AST_BINARY_OP,
	AST_ASSIGNMENT,
	AST_CAST,
	AST_BOOLEAN_LITERAL,
	AST_BREAK_STATEMENT,
	AST_CONTINUE_STATEMENT,
	AST_OBJECT_LITERAL,
	AST_ARRAY_LITERAL,
	AST_MEMBER_ACCESS,
	AST_ARRAY_ACCESS
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct
{
	ASTNode** decls;
	size_t count;
} ASTProgram;

typedef struct
{
	char* name;
	ASTNode** params;
	size_t param_count;
	bool is_variadic;
	char* variadic_name;
	ASTNode* variadic_type;
	ASTNode* return_type;
	ASTNode* body;
	bool is_extern;
	char* abi;           // e.g. "c", "stdcall", "fastcall"
	char* link_name;     // symbol name in native library
	char* library_name;  // which library to link from
	char* visibility;    // "public", "private", etc.
	bool is_export;      // export this function from the binary
} ASTFuncDecl;

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
	ASTNode* var_decl;
	ASTNode* condition;
	ASTNode* increment;
	ASTNode* body;
} ASTForStmt;

typedef struct
{
	char* key;
	ASTNode* value;
} ASTObjectProperty;

typedef struct
{
	ASTObjectProperty* properties;
	size_t count;
} ASTObjectLiteral;

typedef struct
{
	ASTNode** elements;
	size_t count;
} ASTArrayLiteral;

typedef struct
{
	char* name;
	ASTNode* value_type;
} ASTTypeDecl;

typedef struct
{
	ASTNode* object;
	ASTNode* property;
} ASTMemberAccess;

typedef struct
{
	ASTNode* array;
	ASTNode* index;
} ASTArrayAccess;

typedef struct
{
	char* name;
	ASTNode* type;
	ASTNode* initializer;
	bool is_mutable;
} ASTVarDecl;

typedef struct
{
	ASTNode* expr;
} ASTReturn;

typedef struct
{
	char* path;
} ASTImport;

typedef struct
{
	char* value;
	bool is_search_path;
} ASTLinkDirective;

typedef struct
{
	char* name;
	ASTNode* type;
} ASTParam;

typedef struct
{
	ASTNode** statements;
	size_t count;
} ASTBlock;

typedef struct
{
	char* callee;
	ASTNode** args;
	size_t arg_count;
} ASTCall;

typedef struct
{
	char* name;
} ASTIdentifier;

typedef struct
{
	char* value;
} ASTStringLiteral;

typedef struct
{
	char* value;
} ASTNumberLiteral;

typedef struct
{
	char* name;
} ASTType;

typedef struct
{
	bool value;
} ASTBooleanLiteral;

typedef struct
{
	ASTNode* expr;
} ASTExprStmt;

typedef struct
{
	char* op;
	ASTNode* left;
	ASTNode* right;
} ASTBinaryOp;

typedef struct
{
	char* name;
	ASTNode* value;
} ASTAssignment;

typedef struct
{
	ASTNode* target_type;
	ASTNode* expr;
} ASTCast;

struct ASTNode
{
	ASTNodeType type;
	size_t line;
	size_t column;
	union
	{
		ASTProgram program;
		ASTFuncDecl func_decl;
		ASTVarDecl var_decl;
		ASTTypeDecl type_decl;
		ASTImport import;
		ASTLinkDirective link_directive;
		ASTIfStmt if_stmt;
		ASTParam param;
		ASTBlock block;
		ASTCall call;
		ASTIdentifier identifier;
		ASTStringLiteral string_literal;
		ASTNumberLiteral number_literal;
		ASTType type_node;
		ASTReturn ret;
		ASTExprStmt expr_stmt;
		ASTBinaryOp binary_op;
		ASTAssignment assignment;
		ASTCast cast;
		ASTBooleanLiteral boolean_literal;
		ASTWhileStmt while_stmt;
		ASTForStmt for_stmt;
		ASTObjectLiteral object_literal;
		ASTArrayLiteral array_literal;
		ASTMemberAccess member_access;
		ASTArrayAccess array_access;
	};
};

ASTNode* ast_init(ASTNodeType type, size_t line, size_t column);

void ast_free(ASTNode* node);

ASTNode* ast_create_return(ASTNode* expr, size_t line, size_t column);

ASTNode* ast_create_program(ASTNode** decls, size_t count, size_t line, size_t column);

ASTNode* ast_create_function_declaration(const char* name, ASTNode** params, size_t param_count,
										 ASTNode* return_type, ASTNode* body, bool is_variadic,
										 const char* variadic_name, ASTNode* variadic_type,
										 size_t line, size_t column, bool is_extern);

ASTNode* ast_create_variable_declaration(const char* name, ASTNode* type, ASTNode* initializer,
                                         bool is_mutable, size_t line, size_t column);

ASTNode* ast_create_type_declaration(const char* name, ASTNode* value_type, size_t line,
                                     size_t column);

ASTNode* ast_create_import(const char* path, size_t line, size_t column);

ASTNode* ast_create_link_directive(const char* value, bool is_search_path, size_t line,
	                               size_t column);

ASTNode* ast_create_parameter(const char* name, ASTNode* type, size_t line, size_t column);

ASTNode* ast_create_block(ASTNode** statements, size_t count, size_t line, size_t column);

ASTNode* ast_create_call(const char* callee, ASTNode** args, size_t arg_count, size_t line,
                         size_t column);

ASTNode* ast_create_identifier(const char* name, size_t line, size_t column);

ASTNode* ast_create_string_literal(const char* value, size_t line, size_t column);

ASTNode* ast_create_number_literal(const char* value, size_t line, size_t column);

ASTNode* ast_create_type(const char* name, size_t line, size_t column);

ASTNode* ast_create_expression_statement(ASTNode* expr, size_t line, size_t column);

ASTNode* ast_create_binary_op(const char* op, ASTNode* left, ASTNode* right, size_t line,
                              size_t column);

ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch, size_t line,
                       size_t column);

ASTNode* ast_create_while(ASTNode* condition, ASTNode* body, size_t line, size_t column);

ASTNode* ast_create_assignment(const char* name, ASTNode* value, size_t line, size_t column);

ASTNode* ast_create_cast(ASTNode* target_type, ASTNode* expr, size_t line, size_t column);

ASTNode* ast_create_boolean_literal(bool value, size_t line, size_t column);

ASTNode* ast_create_break(size_t line, size_t column);

ASTNode* ast_create_continue(size_t line, size_t column);

ASTNode* ast_create_for(ASTNode* var_decl, ASTNode* condition, ASTNode* increment, ASTNode* body,
                        size_t line, size_t column);

ASTNode* ast_create_object_literal(ASTObjectProperty* properties, size_t count, size_t line,
                                   size_t column);

ASTNode* ast_create_array_literal(ASTNode** elements, size_t count, size_t line, size_t column);

ASTNode* ast_create_member_access(ASTNode* object, ASTNode* property, size_t line, size_t column);

ASTNode* ast_create_array_access(ASTNode* array, ASTNode* index, size_t line, size_t column);

void ast_print(ASTNode* node, int indent);

#endif
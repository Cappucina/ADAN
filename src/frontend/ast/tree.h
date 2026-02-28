#ifndef TREE_H
#define TREE_H

#include <stdlib.h>

typedef enum ASTNodeType
{
	AST_PROGRAM,  // Root; contains a list of statements.
	AST_FUNCTION_DECLARATION,
	AST_VARIABLE_DECLARATION,
	AST_IMPORT_STATEMENT,
	AST_PARAMETER,  // Goes inside of a parameter list.
	AST_BLOCK,
	AST_CALL,  // Function call. `callee(arg, ...)`.
	AST_IDENTIFIER,
	AST_STRING_LITERAL,
	AST_NUMBER_LITERAL,
	AST_TYPE,  // i32, u32, string, etc.
	AST_RETURN_STATEMENT,
	AST_EXPRESSION_STATEMENT  // Expression used as a statement.
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct
{
	ASTNode** decls;  // Array of top level declaration AST nodes.
	size_t count;
} ASTProgram;

// Statement node structures

typedef struct
{
	char* name;
	ASTNode** params;
	size_t param_count;
	ASTNode* return_type;
	ASTNode* body;
} ASTFuncDecl;

typedef struct
{
	char* name;
	ASTNode* type;
	ASTNode* initializer;  // Can be NULL if no initializer is provided.
} ASTVarDecl;

typedef struct
{
	ASTNode* expr;  // May be NULL if no expression is returned
} ASTReturn;

typedef struct
{
	char* path;
} ASTImport;

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
	ASTNode* expr;  // The expression being used as a statement
} ASTExprStmt;

// Unified AST node structure

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
		ASTImport import;
		ASTParam param;
		ASTBlock block;
		ASTCall call;
		ASTIdentifier identifier;
		ASTStringLiteral string_literal;
		ASTNumberLiteral number_literal;
		ASTType type_node;
		ASTReturn ret;
		ASTExprStmt expr_stmt;
	};
};

ASTNode* ast_init(ASTNodeType type, size_t line, size_t column);

void ast_free(ASTNode* node);

// AST helper functions

ASTNode* ast_create_return(ASTNode* expr, size_t line, size_t column);

ASTNode* ast_create_program(ASTNode** decls, size_t count, size_t line, size_t column);

ASTNode* ast_create_function_declaration(const char* name, ASTNode** params, size_t param_count,
                                         ASTNode* return_type, ASTNode* body, size_t line,
                                         size_t column);

ASTNode* ast_create_variable_declaration(const char* name, ASTNode* type, ASTNode* initializer,
                                         size_t line, size_t column);

ASTNode* ast_create_import(const char* path, size_t line, size_t column);

ASTNode* ast_create_parameter(const char* name, ASTNode* type, size_t line, size_t column);

ASTNode* ast_create_block(ASTNode** statements, size_t count, size_t line, size_t column);

ASTNode* ast_create_call(const char* callee, ASTNode** args, size_t arg_count, size_t line,
                         size_t column);

ASTNode* ast_create_identifier(const char* name, size_t line, size_t column);

ASTNode* ast_create_string_literal(const char* value, size_t line, size_t column);

ASTNode* ast_create_number_literal(const char* value, size_t line, size_t column);

ASTNode* ast_create_type(const char* name, size_t line, size_t column);

ASTNode* ast_create_expression_statement(ASTNode* expr, size_t line, size_t column);

// Debugging functions

void ast_print(ASTNode* node, int indent);

#endif
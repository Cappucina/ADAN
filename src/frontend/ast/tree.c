#include <stdio.h>
#include <stdlib.h>

#include "tree.h"

ASTNode* ast_init(ASTNodeType type, size_t line, size_t column)
{
	ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
	if (!node)
	{
		printf("Failed to allocate memory for ASTNode! (Error)\n");
		return NULL;
	}

	node->type = type;
	node->line = line;
	node->column = column;

	return node;
}

void ast_free(ASTNode* node)
{
	if (!node)
	{
		return;
	}

	switch (node->type)
	{
	case AST_PROGRAM:
		for (size_t i = 0; i < node->program.count; i++)
		{
			ast_free(node->program.decls[i]);
		}
		free(node->program.decls);
		break;
	case AST_FUNCTION_DECLARATION:
		free(node->func_decl.name);
		for (size_t i = 0; i < node->func_decl.param_count; i++)
		{
			ast_free(node->func_decl.params[i]);
		}
		free(node->func_decl.params);
		ast_free(node->func_decl.return_type);
		ast_free(node->func_decl.body);
		break;
	case AST_VARIABLE_DECLARATION:
		free(node->var_decl.name);
		ast_free(node->var_decl.type);
		ast_free(node->var_decl.initializer);
		break;
	case AST_IMPORT_STATEMENT:
		free(node->import.path);
		break;
	case AST_PARAMETER:
		free(node->param.name);
		ast_free(node->param.type);
		break;
	case AST_BLOCK:
		for (size_t i = 0; i < node->block.count; i++)
		{
			ast_free(node->block.statements[i]);
		}
		free(node->block.statements);
		break;
	case AST_CALL_STATEMENT:
	case AST_CALL:
		free(node->call.callee);
		for (size_t i = 0; i < node->call.arg_count; i++)
		{
			ast_free(node->call.args[i]);
		}
		free(node->call.args);
		break;
	case AST_IDENTIFIER:
		free(node->identifier.name);
		break;
	case AST_STRING_LITERAL:
		free(node->string_literal.value);
		break;
	case AST_NUMBER_LITERAL:
		free(node->number_literal.value);
		break;
	case AST_TYPE:
		free(node->type_node.name);
		break;
	}

	free(node);
}

// AST helper functions

ASTNode* ast_create_program(ASTNode** decls, size_t count)
{
    return NULL;
}

ASTNode* ast_create_function_declaration(const char* name, ASTNode** params, size_t param_count,
                                         ASTNode* return_type, ASTNode* body)
{
    return NULL;
}

ASTNode* ast_create_variable_declaration(const char* name, ASTNode* type, ASTNode* initializer)
{
    return NULL;
}

ASTNode* ast_create_import(const char* path)
{
    return NULL;
}

ASTNode* ast_create_parameter(const char* name, ASTNode* type)
{
    return NULL;
}

ASTNode* ast_create_block(ASTNode** statements, size_t count)
{
    return NULL;
}

ASTNode* ast_create_call(const char* callee, ASTNode** args, size_t arg_count)
{
    return NULL;
}

ASTNode* ast_create_identifier(const char* name)
{
    return NULL;
}

ASTNode* ast_create_string_literal(const char* value)
{
    return NULL;
}

ASTNode* ast_create_number_literal(const char* value)
{
    return NULL;
}

ASTNode* ast_create_type(const char* name)
{
    return NULL;
}
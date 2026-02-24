#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "../../helper.h"

ASTNode* ast_init(ASTNodeType type, size_t line, size_t column)
{
	ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
	if (!node)
	{
		fprintf(stderr, "Failed to allocate memory for ASTNode! (Error)\n");
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
		case AST_RETURN_STATEMENT:
			ast_free(node->ret.expr);
			break;
		default:
			break;
	}

	free(node);
}

// AST helper functions

ASTNode* ast_create_return(ASTNode* expr, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_RETURN_STATEMENT, line, column);
	if (!node)
		return NULL;
	node->ret.expr = expr;
	return node;
}

ASTNode* ast_create_program(ASTNode** decls, size_t count, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_PROGRAM, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST program node! (Error)\n");
		return NULL;
	}

	node->program.decls = decls;
	node->program.count = count;
	return node;
}

ASTNode* ast_create_function_declaration(const char* name, ASTNode** params, size_t param_count,
                                         ASTNode* return_type, ASTNode* body, size_t line,
                                         size_t column)
{
	ASTNode* node = ast_init(AST_FUNCTION_DECLARATION, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST function declaration node! (Error)\n");
		return NULL;
	}

	node->func_decl.name = clone_string(name, strlen(name));
	if (!node->func_decl.name)
	{
		ast_free(node);
		return NULL;
	}
	node->func_decl.params = params;
	node->func_decl.param_count = param_count;
	node->func_decl.return_type = return_type;
	node->func_decl.body = body;
	return node;
}

ASTNode* ast_create_variable_declaration(const char* name, ASTNode* type, ASTNode* initializer,
                                         size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_VARIABLE_DECLARATION, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST variable declaration node! (Error)\n");
		return NULL;
	}

	node->var_decl.name = clone_string(name, strlen(name));
	if (!node->var_decl.name)
	{
		ast_free(node);
		return NULL;
	}
	node->var_decl.type = type;
	node->var_decl.initializer = initializer;
	return node;
}

ASTNode* ast_create_import(const char* path, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_IMPORT_STATEMENT, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST import statement node! (Error)\n");
		return NULL;
	}

	node->import.path = clone_string(path, strlen(path));
	if (!node->import.path)
	{
		ast_free(node);
		return NULL;
	}
	return node;
}

ASTNode* ast_create_parameter(const char* name, ASTNode* type, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_PARAMETER, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST parameter node! (Error)\n");
		return NULL;
	}

	node->param.name = clone_string(name, strlen(name));
	if (!node->param.name)
	{
		ast_free(node);
		return NULL;
	}
	node->param.type = type;
	return node;
}

ASTNode* ast_create_block(ASTNode** statements, size_t count, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_BLOCK, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST block node! (Error)\n");
		return NULL;
	}

	node->block.statements = statements;
	node->block.count = count;
	return node;
}

ASTNode* ast_create_call(const char* callee, ASTNode** args, size_t arg_count, size_t line,
                         size_t column)
{
	ASTNode* node = ast_init(AST_CALL, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST call node! (Error)\n");
		return NULL;
	}

	node->call.callee = clone_string(callee, strlen(callee));
	if (!node->call.callee)
	{
		ast_free(node);
		return NULL;
	}
	node->call.args = args;
	node->call.arg_count = arg_count;
	return node;
}

ASTNode* ast_create_identifier(const char* name, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_IDENTIFIER, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST identifier node! (Error)\n");
		return NULL;
	}

	node->identifier.name = clone_string(name, strlen(name));
	if (!node->identifier.name)
	{
		ast_free(node);
		return NULL;
	}
	return node;
}

ASTNode* ast_create_string_literal(const char* value, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_STRING_LITERAL, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST string literal node! (Error)\n");
		return NULL;
	}

	node->string_literal.value = clone_string(value, strlen(value));
	if (!node->string_literal.value)
	{
		ast_free(node);
		return NULL;
	}
	return node;
}

ASTNode* ast_create_number_literal(const char* value, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_NUMBER_LITERAL, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST number literal node! (Error)\n");
		return NULL;
	}

	node->number_literal.value = clone_string(value, strlen(value));
	if (!node->number_literal.value)
	{
		ast_free(node);
		return NULL;
	}
	return node;
}

ASTNode* ast_create_type(const char* name, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_TYPE, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST type node! (Error)\n");
		return NULL;
	}

	node->type_node.name = clone_string(name, strlen(name));
	if (!node->type_node.name)
	{
		ast_free(node);
		return NULL;
	}
	return node;
}

// Debugging functions

void ast_print(ASTNode* node, int indent)
{
	if (!node)
	{
		return;
	}

	for (int i = 0; i < indent; i++)
	{
		printf("  ");
	}

	switch (node->type)
	{
		case AST_PROGRAM:
			printf("Program:\n");
			for (size_t i = 0; i < node->program.count; i++)
			{
				ast_print(node->program.decls[i], indent + 1);
			}
			break;
		case AST_FUNCTION_DECLARATION:
			printf("Function Declaration: %s\n", node->func_decl.name);
			for (size_t i = 0; i < node->func_decl.param_count; i++)
			{
				ast_print(node->func_decl.params[i], indent + 1);
			}
			for (int i = 0; i < indent + 1; i++)
				printf("  ");
			printf("Return Type:\n");
			ast_print(node->func_decl.return_type, indent + 2);
			for (int i = 0; i < indent + 1; i++)
				printf("  ");
			printf("Body:\n");
			ast_print(node->func_decl.body, indent + 2);
			break;
		case AST_VARIABLE_DECLARATION:
			printf("Variable Declaration: %s\n", node->var_decl.name);
			for (int i = 0; i < indent + 1; i++)
				printf("  ");
			printf("Type:\n");
			ast_print(node->var_decl.type, indent + 2);
			for (int i = 0; i < indent + 1; i++)
				printf("  ");
			printf("Initializer:\n");
			ast_print(node->var_decl.initializer, indent + 2);
			break;
		case AST_IMPORT_STATEMENT:
			printf("Import: %s\n", node->import.path);
			break;
		case AST_PARAMETER:
			printf("Parameter: %s\n", node->param.name);
			for (int i = 0; i < indent + 1; i++)
				printf("  ");
			printf("Type:\n");
			ast_print(node->param.type, indent + 2);
			break;
		case AST_BLOCK:
			printf("Block:\n");
			for (size_t i = 0; i < node->block.count; i++)
			{
				ast_print(node->block.statements[i], indent + 1);
			}
			break;
		case AST_CALL:
			printf("Call: %s\n", node->call.callee);
			for (size_t i = 0; i < node->call.arg_count; i++)
			{
				for (int j = 0; j < indent + 1; j++)
					printf("  ");
				printf("Argument %zu:\n", i + 1);
				ast_print(node->call.args[i], indent + 2);
			}
			break;
		case AST_IDENTIFIER:
			printf("Identifier: %s\n", node->identifier.name);
			break;
		case AST_STRING_LITERAL:
			printf("String Literal: %s\n", node->string_literal.value);
			break;
		case AST_NUMBER_LITERAL:
			printf("Number Literal: %s\n", node->number_literal.value);
			break;
		case AST_TYPE:
			printf("Type: %s\n", node->type_node.name);
			break;
	}
}
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
			free(node->func_decl.variadic_name);
			for (size_t i = 0; i < node->func_decl.param_count; i++)
			{
				ast_free(node->func_decl.params[i]);
			}
			free(node->func_decl.params);
			ast_free(node->func_decl.variadic_type);
			ast_free(node->func_decl.return_type);
			ast_free(node->func_decl.body);
			break;
		case AST_IF_STATEMENT:
			ast_free(node->if_stmt.condition);
			ast_free(node->if_stmt.then_branch);
			if (node->if_stmt.else_branch)
			{
				ast_free(node->if_stmt.else_branch);
			}
			break;
		case AST_WHILE_STMT:
			ast_free(node->while_stmt.condition);
			ast_free(node->while_stmt.body);
			break;
		case AST_FOR_STMT:
			ast_free(node->for_stmt.var_decl);
			ast_free(node->for_stmt.condition);
			ast_free(node->for_stmt.increment);
			ast_free(node->for_stmt.body);
			break;
		case AST_VARIABLE_DECLARATION:
			free(node->var_decl.name);
			ast_free(node->var_decl.type);
			ast_free(node->var_decl.initializer);
			break;
		case AST_TYPE_DECLARATION:
			free(node->type_decl.name);
			ast_free(node->type_decl.value_type);
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
		case AST_EXPRESSION_STATEMENT:
			ast_free(node->expr_stmt.expr);
			break;
		case AST_BINARY_OP:
			free(node->binary_op.op);
			ast_free(node->binary_op.left);
			ast_free(node->binary_op.right);
			break;
		case AST_ASSIGNMENT:
			free(node->assignment.name);
			ast_free(node->assignment.value);
			break;
		case AST_CAST:
			ast_free(node->cast.target_type);
			ast_free(node->cast.expr);
			break;
		case AST_BREAK_STATEMENT:
		case AST_CONTINUE_STATEMENT:
			break;
		case AST_OBJECT_LITERAL:
			for (size_t i = 0; i < node->object_literal.count; i++)
			{
				free(node->object_literal.properties[i].key);
				ast_free(node->object_literal.properties[i].value);
			}
			free(node->object_literal.properties);
			break;
		case AST_ARRAY_LITERAL:
			for (size_t i = 0; i < node->array_literal.count; i++)
			{
				ast_free(node->array_literal.elements[i]);
			}
			free(node->array_literal.elements);
			break;
		case AST_MEMBER_ACCESS:
			ast_free(node->member_access.object);
			ast_free(node->member_access.property);
			break;
		case AST_ARRAY_ACCESS:
			ast_free(node->array_access.array);
			ast_free(node->array_access.index);
			break;
		default:
			break;
	}

	free(node);
}

ASTNode* ast_create_return(ASTNode* expr, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_RETURN_STATEMENT, line, column);
	if (!node)
	{
		return NULL;
	}
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
										 ASTNode* return_type, ASTNode* body, bool is_variadic,
										 const char* variadic_name, ASTNode* variadic_type,
										 size_t line, size_t column)
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
	node->func_decl.is_variadic = is_variadic;
	node->func_decl.variadic_name = variadic_name ? clone_string(variadic_name, strlen(variadic_name)) : NULL;
	node->func_decl.variadic_type = variadic_type;
	if (variadic_name && !node->func_decl.variadic_name)
	{
		ast_free(node);
		return NULL;
	}
	node->func_decl.return_type = return_type;
	node->func_decl.body = body;
	return node;
}

ASTNode* ast_create_variable_declaration(const char* name, ASTNode* type, ASTNode* initializer,
	                                     bool is_mutable, size_t line, size_t column)
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
	node->var_decl.is_mutable = is_mutable;
	return node;
}

ASTNode* ast_create_type_declaration(const char* name, ASTNode* value_type, size_t line,
	                                 size_t column)
{
	ASTNode* node = ast_init(AST_TYPE_DECLARATION, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST type declaration node! (Error)\n");
		return NULL;
	}

	node->type_decl.name = clone_string(name, strlen(name));
	if (!node->type_decl.name)
	{
		ast_free(node);
		return NULL;
	}
	node->type_decl.value_type = value_type;
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

ASTNode* ast_create_expression_statement(ASTNode* expr, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_EXPRESSION_STATEMENT, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST expression statement node! (Error)\n");
		return NULL;
	}

	node->expr_stmt.expr = expr;
	return node;
}

ASTNode* ast_create_binary_op(const char* op, ASTNode* left, ASTNode* right, size_t line,
                              size_t column)
{
	ASTNode* node = ast_init(AST_BINARY_OP, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST binary op node! (Error)\n");
		return NULL;
	}

	node->binary_op.op = clone_string(op, strlen(op));
	if (!node->binary_op.op)
	{
		ast_free(node);
		return NULL;
	}
	node->binary_op.left = left;
	node->binary_op.right = right;
	return node;
}

ASTNode* ast_create_assignment(const char* name, ASTNode* value, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_ASSIGNMENT, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST assignment node! (Error)\n");
		return NULL;
	}
	node->assignment.name = clone_string(name, strlen(name));
	if (!node->assignment.name)
	{
		ast_free(node);
		return NULL;
	}
	node->assignment.value = value;
	return node;
}

ASTNode* ast_create_cast(ASTNode* target_type, ASTNode* expr, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_CAST, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST cast node! (Error)\n");
		return NULL;
	}
	node->cast.target_type = target_type;
	node->cast.expr = expr;
	return node;
}

ASTNode* ast_create_while(ASTNode* condition, ASTNode* body, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_WHILE_STMT, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST while node! (Error)\n");
		return NULL;
	}
	node->while_stmt.condition = condition;
	node->while_stmt.body = body;
	return node;
}

ASTNode* ast_create_boolean_literal(bool value, size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_BOOLEAN_LITERAL, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST boolean literal node! (Error)\n");
		return NULL;
	}
	node->boolean_literal.value = value;
	return node;
}

ASTNode* ast_create_break(size_t line, size_t column)
{
	return ast_init(AST_BREAK_STATEMENT, line, column);
}

ASTNode* ast_create_continue(size_t line, size_t column)
{
	return ast_init(AST_CONTINUE_STATEMENT, line, column);
}

ASTNode* ast_create_for(ASTNode* var_decl, ASTNode* condition, ASTNode* increment, ASTNode* body,
                        size_t line, size_t column)
{
	ASTNode* node = ast_init(AST_FOR_STMT, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST for node! (Error)\n");
		return NULL;
	}
	node->for_stmt.var_decl = var_decl;
	node->for_stmt.condition = condition;
	node->for_stmt.increment = increment;
	node->for_stmt.body = body;
	return node;
}

ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch, size_t line,
                       size_t column)
{
	ASTNode* node = ast_init(AST_IF_STATEMENT, line, column);
	if (!node)
	{
		fprintf(stderr, "Failed to create AST if statement node! (Error)\n");
		return NULL;
	}
	node->if_stmt.condition = condition;
	node->if_stmt.then_branch = then_branch;
	node->if_stmt.else_branch = else_branch;
	return node;
}

ASTNode* ast_create_object_literal(ASTObjectProperty* properties, size_t count, size_t line,
                                   size_t column)
{
	ASTNode* node = ast_init(AST_OBJECT_LITERAL, line, column);
	node->object_literal.properties = properties;
	node->object_literal.count = count;
	return node;
}

ASTNode* ast_create_array_literal(ASTNode** elements, size_t count, size_t line,
                                  size_t column)
{
	ASTNode* node = ast_init(AST_ARRAY_LITERAL, line, column);
	node->array_literal.elements = elements;
	node->array_literal.count = count;
	return node;
}

ASTNode* ast_create_member_access(ASTNode* object, ASTNode* property, size_t line,
                                  size_t column)
{
	ASTNode* node = ast_init(AST_MEMBER_ACCESS, line, column);
	node->member_access.object = object;
	node->member_access.property = property;
	return node;
}

ASTNode* ast_create_array_access(ASTNode* array, ASTNode* index, size_t line,
                                 size_t column)
{
	ASTNode* node = ast_init(AST_ARRAY_ACCESS, line, column);
	node->array_access.array = array;
	node->array_access.index = index;
	return node;
}

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
			printf("Function Declaration: %s%s\n", node->func_decl.name,
			       node->func_decl.is_variadic ? " (...)" : "");
			for (size_t i = 0; i < node->func_decl.param_count; i++)
			{
				ast_print(node->func_decl.params[i], indent + 1);
			}
			if (node->func_decl.is_variadic && node->func_decl.variadic_name)
			{
				for (int i = 0; i < indent + 1; i++)
				{
					printf("  ");
				}
				printf("Variadic Parameter: %s\n", node->func_decl.variadic_name);
				if (node->func_decl.variadic_type)
				{
					ast_print(node->func_decl.variadic_type, indent + 2);
				}
			}
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Return Type:\n");
			ast_print(node->func_decl.return_type, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Body:\n");
			ast_print(node->func_decl.body, indent + 2);
			break;
		case AST_IF_STATEMENT:
			printf("If Statement:\n");
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Condition:\n");
			ast_print(node->if_stmt.condition, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Then:\n");
			ast_print(node->if_stmt.then_branch, indent + 2);
			if (node->if_stmt.else_branch)
			{
				for (int i = 0; i < indent + 1; i++)
				{
					printf("  ");
				}
				printf("Else:\n");
				ast_print(node->if_stmt.else_branch, indent + 2);
			}
			break;
		case AST_WHILE_STMT:
			for (int i = 0; i < indent; i++)
			{
				printf("  ");
			}
			printf("WhileStatement:\n");
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Condition:\n");
			ast_print(node->while_stmt.condition, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Body:\n");
			ast_print(node->while_stmt.body, indent + 2);
			break;
		case AST_FOR_STMT:
			for (int i = 0; i < indent; i++)
			{
				printf("  ");
			}
			printf("ForStatement:\n");
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("VarDecl:\n");
			ast_print(node->for_stmt.var_decl, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Condition:\n");
			ast_print(node->for_stmt.condition, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Increment:\n");
			ast_print(node->for_stmt.increment, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Body:\n");
			ast_print(node->for_stmt.body, indent + 2);
			break;
		case AST_VARIABLE_DECLARATION:
			printf("Variable Declaration: %s (%s)\n", node->var_decl.name,
			       node->var_decl.is_mutable ? "mutable" : "const");
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Type:\n");
			ast_print(node->var_decl.type, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Initializer:\n");
			ast_print(node->var_decl.initializer, indent + 2);
			break;
		case AST_TYPE_DECLARATION:
			printf("Type Declaration: %s\n", node->type_decl.name);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Value Type:\n");
			ast_print(node->type_decl.value_type, indent + 2);
			break;
		case AST_IMPORT_STATEMENT:
			printf("Import: %s\n", node->import.path);
			break;
		case AST_PARAMETER:
			printf("Parameter: %s\n", node->param.name);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
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
				{
					printf("  ");
				}
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
		case AST_BOOLEAN_LITERAL:
			printf("Boolean Literal: %s\n",
			       node->boolean_literal.value ? "true" : "false");
			break;
		case AST_TYPE:
			printf("Type: %s\n", node->type_node.name);
			break;
		case AST_RETURN_STATEMENT:
			printf("Return Statement:\n");
			ast_print(node->ret.expr, indent + 1);
			break;
		case AST_EXPRESSION_STATEMENT:
			printf("Expression Statement:\n");
			ast_print(node->expr_stmt.expr, indent + 1);
			break;
		case AST_INTERPOLATED_STRING:
			printf("Interpolated String (desugared):\n");
			break;
		case AST_BINARY_OP:
			printf("Binary Op: %s\n", node->binary_op.op);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Left:\n");
			ast_print(node->binary_op.left, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Right:\n");
			ast_print(node->binary_op.right, indent + 2);
			break;
		case AST_ASSIGNMENT:
			printf("Assignment: %s\n", node->assignment.name);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Value:\n");
			ast_print(node->assignment.value, indent + 2);
			break;
		case AST_CAST:
			printf("Cast:\n");
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Target Type:\n");
			ast_print(node->cast.target_type, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Expression:\n");
			ast_print(node->cast.expr, indent + 2);
			break;
		case AST_BREAK_STATEMENT:
			printf("Break Statement\n");
			break;
		case AST_CONTINUE_STATEMENT:
			printf("Continue Statement\n");
			break;
		case AST_OBJECT_LITERAL:
			printf("Object Literal:\n");
			for (size_t i = 0; i < node->object_literal.count; i++)
			{
				for (int j = 0; j < indent + 1; j++)
				{
					printf("  ");
				}
				printf("Property %s:\n", node->object_literal.properties[i].key);
				ast_print(node->object_literal.properties[i].value, indent + 2);
			}
			break;
		case AST_ARRAY_LITERAL:
			printf("Array Literal:\n");
			for (size_t i = 0; i < node->array_literal.count; i++)
			{
				ast_print(node->array_literal.elements[i], indent + 1);
			}
			break;
		case AST_MEMBER_ACCESS:
			printf("Member Access:\n");
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Object:\n");
			ast_print(node->member_access.object, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Property:\n");
			ast_print(node->member_access.property, indent + 2);
			break;
		case AST_ARRAY_ACCESS:
			printf("Array Access:\n");
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Array:\n");
			ast_print(node->array_access.array, indent + 2);
			for (int i = 0; i < indent + 1; i++)
			{
				printf("  ");
			}
			printf("Index:\n");
			ast_print(node->array_access.index, indent + 2);
			break;
	}
}
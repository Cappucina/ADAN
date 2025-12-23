#include "semantic.h"
#include "util.h"
#include "ast.h"
#include <string.h>
#include "parser.h"
#include "logs.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "library.h"

static int semantic_error_count = 0;
static int semantic_warning_count = 0;
static int semantic_tip_count = 0;

int semantic_get_error_count() {
	return semantic_error_count;
}

int semantic_get_warning_count() {
	return semantic_warning_count;
}

int semantic_get_tip_count() {
	return semantic_tip_count;
}

bool array_element_types_match(CompleteType array_type, ASTNode *array_literal, SymbolTable *table);

const char *type_to_string(CompleteType c_type);

bool same_type_recursive(CompleteType a, CompleteType b)
{
	if (a.type != b.type)
		return false;

	if (a.type == TYPE_ARRAY)
	{
		if (!a.points_to || !b.points_to)
			return false;
		return same_type_recursive(*a.points_to, *b.points_to);
	}

	return true;
}

void type_to_string_recursive(CompleteType t, char *buf, size_t bufsize)
{
	if (t.type != TYPE_ARRAY)
	{
		switch (t.type)
		{
		case TYPE_INT:
			snprintf(buf, bufsize, "int");
			break;
		case TYPE_FLOAT:
			snprintf(buf, bufsize, "float");
			break;
		case TYPE_STRING:
			snprintf(buf, bufsize, "string");
			break;
		case TYPE_CHAR:
			snprintf(buf, bufsize, "char");
			break;
		case TYPE_BOOLEAN:
			snprintf(buf, bufsize, "bool");
			break;
		case TYPE_VOID:
			snprintf(buf, bufsize, "void");
			break;
		case TYPE_NULL:
			snprintf(buf, bufsize, "null");
			break;
		case TYPE_UNKNOWN:
			snprintf(buf, bufsize, "unknown");
			break;
		default:
			snprintf(buf, bufsize, "array");
			break;
		}
	}
	else
	{
		char inner[128];
		if (t.points_to)
			type_to_string_recursive(*t.points_to, inner, sizeof(inner));
		else
			snprintf(inner, sizeof(inner), "unknown");
		snprintf(buf, bufsize, "%s[]", inner);
	}
}

SymbolTable *init_symbol_table()
{
	SymbolTable *table = malloc(sizeof(SymbolTable));

	table->bucket_count = 64;
	table->buckets = calloc(table->bucket_count, sizeof(Symbol *));
	table->parent = NULL;
	table->loop_depth = 0;
	CompleteType void_type;
	void_type.type = TYPE_VOID;
	table->current_return_type = void_type;

	return table;
}

void enter_scope(SymbolTable *table)
{
	SymbolTable *saved = malloc(sizeof(SymbolTable));
	if (!saved)
		return;

	*saved = *table;

	table->parent = saved;
	table->buckets = calloc(saved->bucket_count, sizeof(Symbol *));
	table->loop_depth = saved->loop_depth;
	table->current_return_type = saved->current_return_type;
}

void exit_scope(SymbolTable *table)
{
	if (!table->parent)
		return;

	for (int i = 0; i < table->bucket_count; i++)
	{
		Symbol *sym = table->buckets[i];
		while (sym)
		{
			Symbol *next = sym->next;
			free(sym->name);
			free(sym);
			sym = next;
		}
	}

	free(table->buckets);

	SymbolTable *parent = table->parent;
	*table = *parent;
	free(parent);
}

bool add_symbol(SymbolTable *table, const char *name, CompleteType type, ASTNode *node)
{
	if (!table || !name)
		return false;

	unsigned long hash = hash_string(name);
	int index = (int)(hash % table->bucket_count);

	//
	//  Check for duplicates in the current scope
	//
	Symbol *current_symbol = table->buckets[index];
	while (current_symbol)
	{
		if (strcmp(current_symbol->name, name) == 0)
			return false;
		current_symbol = current_symbol->next;
	}

	//
	//  Allocate a new symbol
	//
	Symbol *new_symbol = malloc(sizeof(Symbol));
	if (!new_symbol)
		return false;

	new_symbol->name = strdup(name);
	if (!new_symbol->name)
	{
		free(new_symbol);
		return false;
	}

	new_symbol->type = type;
	new_symbol->node = node;
	new_symbol->usage_count = 0;
	new_symbol->next = table->buckets[index];

	table->buckets[index] = new_symbol;
	return true;
}

Symbol *lookup_symbol(SymbolTable *table, const char *name)
{
	if (!table || !name)
		return NULL;

	while (table)
	{
		if (!table->buckets || table->bucket_count <= 0)
		{
			table = table->parent;
			continue;
		}

		unsigned long hash = hash_string(name);
		int index = (int)(hash % table->bucket_count);

		Symbol *current_symbol = table->buckets[index];
		while (current_symbol)
		{
			if (strcmp(current_symbol->name, name) == 0)
			{
				current_symbol->usage_count++;
				return current_symbol;
			}
			current_symbol = current_symbol->next;
		}
		table = table->parent;
	}

	return NULL;
}

bool symbol_in_scope(SymbolTable *table, const char *name)
{
	if (!table || !name)
		return false;

	unsigned long hash = hash_string(name);
	int index = (int)(hash % table->bucket_count);

	Symbol *current_symbol = table->buckets[index];
	while (current_symbol)
	{
		if (strcmp(current_symbol->name, name) == 0)
			return true;
		current_symbol = current_symbol->next;
	}

	return false;
}

//
//  AST Traversal / Semantic Checks
//
void analyze_continue(ASTNode *continue_node, SymbolTable *table);
void analyze_block(ASTNode *block, SymbolTable *table)
{
	if (block == NULL || table == NULL)
		return;
	if (block->type != AST_BLOCK)
		return;

	for (int i = 0; i < block->child_count; i++)
	{
		analyze_statement(block->children[i], table);
	}
}

void analyze_statement(ASTNode *statement, SymbolTable *table)
{
	if (statement == NULL || table == NULL)
		return;

	switch (statement->type)
	{
	case AST_ASSIGNMENT:
		analyze_assignment(statement, table);
		break;

	case AST_DECLARATION:
		analyze_declaration(statement, table);
		break;

	case AST_CONTINUE:
		analyze_continue(statement, table);
		break;

	case AST_IF:
		analyze_if(statement, table);
		break;

	case AST_WHILE:
		analyze_while(statement, table);
		break;

	case AST_FOR:
		analyze_for(statement, table);
		break;

	case AST_RETURN:
		analyze_return(statement, table);
		break;

	case AST_BREAK:
		analyze_break(statement, table);
		break;

	case AST_ARRAY_ACCESS:
	case AST_ARRAY_INDEX:
		analyze_array_access(statement, table);
		break;

	case AST_EXPRESSION:
	case AST_BINARY_OP:
	case AST_BINARY_EXPR:
	case AST_UNARY_OP:
	case AST_UNARY_EXPR:
	case AST_COMPARISON:
	case AST_LOGICAL_OP:
	case AST_ADDRESS_OF:
	case AST_DEREFERENCE:
		analyze_expression(statement, table);
		break;

	case AST_INCREMENT_EXPR:
		analyze_expression(statement, table);
		break;

	case AST_FUNCTION_CALL:
		analyze_function_call(statement, table);
		break;

	case AST_IDENTIFIER:
	case AST_LITERAL:
		break;

	default:
		semantic_error(statement, SemanticErrorMessages[SEMANTIC_UNKNOWN_STATEMENT], statement->token.text);
		break;
	}
}

void analyze_file(ASTNode *file_node, SymbolTable *table)
{
	if (!file_node || !table)
		return;
	if (file_node->type != AST_FILE)
		return;

	for (int i = 0; i < file_node->child_count; i++)
	{
		ASTNode *child = file_node->children[i];
		if (child->type == AST_INCLUDE)
		{
			analyze_include(child, table);
		}
		else if (child->type == AST_DECLARATION)
		{
			analyze_declaration(child, table);
		}
		else if (child->type == AST_PROGRAM)
		{
			analyze_program(child, table);
		}
	}
}

void analyze_program(ASTNode *func_node, SymbolTable *table)
{
	if (!func_node || !table || func_node->type != AST_PROGRAM)
		return;

	ASTNode *return_type_node = func_node->children[0];
	ASTNode *func_name_node = func_node->children[1];
	ASTNode *params_node = func_node->children[2];
	ASTNode *block_node = func_node->children[3];

	CompleteType return_type = get_expression_type(return_type_node, table);
	table->current_return_type = return_type;

	if (!add_symbol(table, func_name_node->token.text, return_type, func_node))
	{
		semantic_error(func_name_node,
					   SemanticErrorMessages[SEMANTIC_DUPLICATE_SYMBOL],
					   func_name_node->token.text);
		return;
	}

	enter_scope(table);

	if (params_node && params_node->type == AST_PARAMS)
	{
		for (int i = 0; i < params_node->child_count; i++)
		{
			ASTNode *param = params_node->children[i];
			CompleteType param_type =
				get_expression_type(param->children[1], table);

			const char *param_name = param->children[0]->token.text;
			if (!add_symbol(table, param_name, param_type, param))
			{
				semantic_error(param,
							   SemanticErrorMessages[SEMANTIC_DUPLICATE_SYMBOL],
							   param_name);
			}
		}
	}

	if (block_node && block_node->type == AST_BLOCK)
		analyze_block(block_node, table);

	if (return_type.type != TYPE_VOID)
	{
		bool has_return = false;

		for (int i = 0; i < block_node->child_count; i++)
		{
			ASTNode *stmt = block_node->children[i];
			if (stmt->type != AST_RETURN)
				continue;

			has_return = true;

			CompleteType actual = stmt->children[0]
									  ? stmt->children[0]->annotated_type
									  : (CompleteType){.type = TYPE_VOID};

			if (!check_type_compatibility(return_type, actual))
			{
				semantic_error(stmt,
							   SemanticErrorMessages[SEMANTIC_RETURN_TYPE_MISMATCH]);
			}
		}

		if (!has_return)
		{
			semantic_error(func_node,
						   SemanticErrorMessages[SEMANTIC_MISSING_RETURN]);
		}
	}

	exit_scope(table);
}

void analyze_for(ASTNode *for_node, SymbolTable *table)
{
	if (!for_node || !table)
		return;
	if (for_node->type != AST_FOR)
		return;

	ASTNode *assignment_node = for_node->children[0];
	ASTNode *condition_node = for_node->children[1];
	ASTNode *increment_node = for_node->children[2];
	ASTNode *block_node = for_node->children[3];

	if (assignment_node == NULL)
		return;

	enter_scope(table);
	analyze_statement(assignment_node, table);

	CompleteType type_boolean;
	type_boolean.type = TYPE_BOOLEAN;

	CompleteType cond_type = get_expression_type(condition_node, table); // LILY: THIS IS WHERE WE NEED TO BE
	if (cond_type.type != TYPE_BOOLEAN)
	{
		semantic_error(for_node, SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH], type_to_string(cond_type), type_to_string(type_boolean));
	}

	if (increment_node == NULL)
	{
		semantic_tip(for_node, SemanticTipMessages[SEMANTIC_TIP_PREFER_WHILE_LOOP]);
	}
	else
	{
		analyze_statement(increment_node, table);
	}

	if (block_node && block_node->type == AST_BLOCK)
	{
		table->loop_depth++;
		analyze_block(block_node, table);
		table->loop_depth--;
	}

	exit_scope(table);
}

void analyze_continue(ASTNode *continue_node, SymbolTable *table)
{
	if (!continue_node || !table)
		return;
	if (continue_node->type != AST_CONTINUE)
		return;

	int depth = table->loop_depth;
	if (depth <= 0)
	{
		semantic_error(continue_node, SemanticErrorMessages[SEMANTIC_CONTINUE_OUTSIDE_LOOP]);
	}
}

void analyze_if(ASTNode *if_node, SymbolTable *table)
{
	if (!if_node || !table)
		return;
	if (if_node->type != AST_IF)
		return;

	ASTNode *condition_node = if_node->children[0];
	ASTNode *block_node = if_node->children[1];

	if (condition_node == NULL)
		return;

	enter_scope(table);
	analyze_statement(condition_node, table);

	CompleteType type_boolean;
	type_boolean.type = TYPE_BOOLEAN;

	CompleteType cond_type = get_expression_type(condition_node, table);
	if (cond_type.type != TYPE_BOOLEAN)
	{
		semantic_error(if_node, SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH], type_to_string(cond_type), type_to_string(type_boolean));
	}

	if (block_node && block_node->type == AST_BLOCK)
	{
		analyze_block(block_node, table);
	}

	exit_scope(table);

	if (if_node->child_count >= 3)
	{
		ASTNode *else_block = if_node->children[2];
		if (else_block && else_block->type == AST_BLOCK)
		{
			enter_scope(table);
			analyze_block(else_block, table);
			exit_scope(table);
		}
		else if (else_block && else_block->type == AST_IF)
		{
			analyze_statement(else_block, table);
		}
	}
}

void analyze_while(ASTNode *while_node, SymbolTable *table)
{
	if (!while_node || !table)
		return;
	if (while_node->type != AST_WHILE)
		return;

	ASTNode *condition_node = while_node->children[0];
	ASTNode *block_node = while_node->children[1];

	if (condition_node == NULL)
		return;

	enter_scope(table);
	analyze_statement(condition_node, table);

	CompleteType type_boolean;
	type_boolean.type = TYPE_BOOLEAN;

	CompleteType cond_type = get_expression_type(condition_node, table);
	if (cond_type.type != TYPE_BOOLEAN)
	{
		semantic_error(while_node, SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH], type_to_string(cond_type), type_to_string(type_boolean));
	}

	if (block_node && block_node->type == AST_BLOCK)
	{
		table->loop_depth++;
		analyze_block(block_node, table);
		table->loop_depth--;
	}

	exit_scope(table);
}

void analyze_return(ASTNode *return_node, SymbolTable *table)
{
	if (!return_node || !table)
		return;
	if (return_node->type != AST_RETURN)
		return;

	CompleteType return_type = table->current_return_type;
	if (return_node->child_count > 0)
	{
		ASTNode *child_node = return_node->children[0];

		analyze_expression(child_node, table);

		CompleteType actual_type = get_expression_type(child_node, table);
		if (return_type.type != actual_type.type)
		{
			semantic_error(return_node, SemanticErrorMessages[SEMANTIC_RETURN_VALUE_TYPE_MISMATCH],
						   type_to_string(actual_type), type_to_string(return_type));
		}
	}
	else
	{
		if (return_type.type != TYPE_VOID)
		{
			semantic_error(return_node, SemanticErrorMessages[SEMANTIC_NON_VOID_RETURN_WITHOUT_VALUE],
						   type_to_string(return_type));
		}
	}
}

void analyze_break(ASTNode *break_node, SymbolTable *table)
{
	if (!break_node || !table)
		return;
	if (break_node->type != AST_BREAK)
		return;

	int depth = table->loop_depth;
	if (depth <= 0)
	{
		semantic_error(break_node, SemanticErrorMessages[SEMANTIC_BREAK_OUTSIDE_LOOP]);
	}
}

void analyze_declaration(ASTNode *declaration_node, SymbolTable *table)
{
	if (!declaration_node || !table)
		return;

	ASTNode *identifier_node = declaration_node->children[0];
	ASTNode *type_node = declaration_node->children[1];

	CompleteType expected_type = get_expression_type(type_node, table);

	if (declaration_node->child_count >= 3)
	{
		ASTNode *expression_node = declaration_node->children[2];

		analyze_expression(expression_node, table);

		CompleteType actual_type = get_expression_type(expression_node, table);

		if (!same_type_recursive(actual_type, expected_type))
		{
			char expected_str[128], actual_str[128];
			type_to_string_recursive(expected_type, expected_str, sizeof(expected_str));
			type_to_string_recursive(actual_type, actual_str, sizeof(actual_str));

			semantic_error(declaration_node,
						   SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH],
						   expected_str,
						   actual_str);
			return;
		}

		if (expected_type.type == TYPE_ARRAY && expected_type.points_to &&
			expression_node->type == AST_ARRAY_LITERAL)
		{
			if (!array_element_types_match(expected_type, expression_node, table))
			{
				char elem_str[128];
				type_to_string_recursive(*expected_type.points_to, elem_str, sizeof(elem_str));

				semantic_error(declaration_node,
							   SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH],
							   elem_str,
							   "<array element>");
				return;
			}
		}
	}

	add_symbol(table, identifier_node->token.text, expected_type, declaration_node);
}

//
//  See the visualized form (Tree): `../diagrams/variable_assignment.txt`
//
void print_type(CompleteType t)
{
	if (!t.points_to)
	{
		switch (t.type)
		{
		case TYPE_INT:
			printf("int");
			break;
		case TYPE_FLOAT:
			printf("float");
			break;
		case TYPE_STRING:
			printf("string");
			break;
		case TYPE_CHAR:
			printf("char");
			break;
		case TYPE_BOOLEAN:
			printf("bool");
			break;
		case TYPE_VOID:
			printf("void");
			break;
		case TYPE_NULL:
			printf("null");
			break;
		case TYPE_UNKNOWN:
			printf("unknown");
			break;
		case TYPE_ARRAY:
			printf("array[]");
			break;
		default:
			break;
		}
	}
	else
	{
		printf("array of ");
		print_type(*t.points_to);
	}
}

bool array_element_types_match(CompleteType array_type, ASTNode *array_literal, SymbolTable *table)
{
	if (!array_type.points_to || !array_literal)
		return false;

	for (int i = 0; i < array_literal->child_count; i++)
	{
		CompleteType elem_type = get_expression_type(array_literal->children[i], table);
		if (!same_type_recursive(*array_type.points_to, elem_type))
			return false;
	}
	return true;
}

void analyze_assignment(ASTNode *assignment_node, SymbolTable *table)
{
	if (!assignment_node || !table)
		return;

	ASTNode *identifier_node = assignment_node->children[0];
	ASTNode *expression_node = assignment_node->children[1];

	analyze_expression(expression_node, table);

	Symbol *symbol = lookup_symbol(table, identifier_node->token.text);
	if (!symbol)
	{
		semantic_error(assignment_node,
					   SemanticErrorMessages[SEMANTIC_UNKNOWN_VARIABLE],
					   identifier_node->token.text);
		return;
	}

	CompleteType lhs = symbol->type;
	CompleteType rhs = get_expression_type(expression_node, table);

	if (!same_type_recursive(lhs, rhs))
	{
		char lhs_str[128], rhs_str[128];
		type_to_string_recursive(lhs, lhs_str, sizeof(lhs_str));
		type_to_string_recursive(rhs, rhs_str, sizeof(rhs_str));

		semantic_error(assignment_node,
					   SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH],
					   lhs_str,
					   rhs_str);
		return;
	}

	if (lhs.type == TYPE_ARRAY && lhs.points_to &&
		expression_node->type == AST_ARRAY_LITERAL)
	{
		if (!array_element_types_match(lhs, expression_node, table))
		{
			char elem_str[128];
			type_to_string_recursive(*lhs.points_to, elem_str, sizeof(elem_str));

			semantic_error(assignment_node,
						   SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH],
						   elem_str,
						   "<array element>");
			return;
		}
	}
}

//
//  include <publisher>.<package>;
//
//  include adan.io; // Import ADAN's native I/O library.
//
void analyze_include(ASTNode *include_node, SymbolTable *table)
{
	if (!include_node || !table)
		return;

	if (include_node->type != AST_INCLUDE)
		return;
	if (include_node->child_count < 2)
		return;

	ASTNode *publisher_node = include_node->children[0];
	ASTNode *package_node = include_node->children[1];

	if (!publisher_node || !package_node)
		return;
	if (!publisher_node->token.text || !package_node->token.text)
		return;

	extern LibraryRegistry *global_library_registry;

	if (!global_library_registry)
	{
		log_semantic_error(include_node, "Library registry not initialized");
		return;
	}

	Library *lib = load_library(global_library_registry,
								publisher_node->token.text,
								package_node->token.text);

	if (!lib)
	{
		log_semantic_error(include_node, "Failed to load library %s.%s",
						   publisher_node->token.text, package_node->token.text);
		return;
	}

	if (!import_library_symbols(lib, table))
	{
		log_semantic_error(include_node, "Failed to import symbols from %s.%s",
						   publisher_node->token.text, package_node->token.text);
	}
}

void analyze_function_call(ASTNode *call_node, SymbolTable *table)
{
	if (!call_node || !table)
		return;
	if (call_node->type != AST_FUNCTION_CALL)
		return;
	if (call_node->child_count < 1)
		return;

	ASTNode *func_name_node = call_node->children[0];
	Symbol *func_symbol = lookup_symbol(table, func_name_node->token.text);

	if (func_symbol == NULL)
	{
		semantic_error(call_node, SemanticErrorMessages[SEMANTIC_FUNCTION_NOT_FOUND], func_name_node->token.text);
		return;
	}

	int arg_count = 0;

	CompleteType type_unknown;
	type_unknown.type = TYPE_UNKNOWN;

	CompleteType first_arg_type = type_unknown;
	if (call_node->child_count > 1)
	{
		ASTNode *params_node = call_node->children[1];
		if (params_node && params_node->type == AST_PARAMS)
		{
			arg_count = params_node->child_count;
			for (int i = 0; i < arg_count; i++)
			{
				ASTNode *arg = params_node->children[i];
				analyze_expression(arg, table);
				if (i == 0)
				{
					first_arg_type = get_expression_type(arg, table);
				}
			}
		}
	}

	const char *resolved_name = func_name_node->token.text;
	char resolved_buffer[256];
	if ((strcmp(resolved_name, "print") == 0 || strcmp(resolved_name, "println") == 0) && arg_count == 1)
	{
		if (first_arg_type.type == TYPE_INT)
		{
			snprintf(resolved_buffer, sizeof(resolved_buffer), "%s_int", resolved_name);
			resolved_name = resolved_buffer;
			free((char *)func_name_node->token.text);
			func_name_node->token.text = strdup(resolved_name);
		}
		else if (first_arg_type.type == TYPE_FLOAT)
		{
			snprintf(resolved_buffer, sizeof(resolved_buffer), "%s_float", resolved_name);
			resolved_name = resolved_buffer;
			free((char *)func_name_node->token.text);
			func_name_node->token.text = strdup(resolved_name);
		}
	}

	extern LibraryRegistry *global_library_registry;
	if (global_library_registry)
	{
		Library *lib = global_library_registry->libraries;
		while (lib)
		{
			LibraryFunction *func = lib->functions;
			while (func)
			{
				if (strcmp(func->name, resolved_name) == 0)
				{
					if (arg_count != func->param_count)
					{
						semantic_error(call_node, SemanticErrorMessages[SEMANTIC_WRONG_ARGUMENT_COUNT], arg_count, func->param_count);
					}
					return;
				}
				func = func->next;
			}
			lib = lib->next;
		}
	}
}

void check_entry_point(SymbolTable *table)
{
	if (!table)
		return;

	Symbol *main_symbol = lookup_symbol(table, "main");

	if (main_symbol == NULL)
		return;
}

void analyze_array_literal(ASTNode *node, SymbolTable *table)
{
	if (!node || node->type != AST_ARRAY_LITERAL)
		return;
	if (!validate_array_element_types(node, table))
		semantic_error(node, SemanticErrorMessages[SEMANTIC_ARRAY_MIXED_TYPES]);
}

bool validate_array_element_types(ASTNode *array_node, SymbolTable *table)
{
	if (array_node->child_count == 0)
		return true;

	CompleteType first_type = get_expression_type(array_node->children[0], table);
	for (int i = 1; i < array_node->child_count; i++)
	{
		CompleteType current_type = get_expression_type(array_node->children[i], table);
		if (current_type.type != first_type.type)
			return false;
	}

	return true;
}

void check_unreachable_code(ASTNode *block, SymbolTable *table)
{
	if (!block || block->type != AST_BLOCK)
		return;
	for (int i = 0; i < block->child_count - 1; i++)
	{
		ASTNode *statement = block->children[i];

		if (statement->type == AST_RETURN || statement->type == AST_BREAK)
		{
			ASTNode *next_statement = block->children[i + 1];
			semantic_warning(next_statement, SemanticWarningMessages[SEMANTIC_UNREACHABLE_CODE], next_statement->token.text);
		}
	}
}

bool has_all_paths_return(ASTNode *block, CompleteType return_type)
{
	if (!block || block->type != AST_BLOCK)
		return false;
	for (int i = block->child_count - 1; i >= 0; i--)
	{
		ASTNode *statement = block->children[i];

		if (statement->type == AST_RETURN)
			return true;
		if (statement->type == AST_IF)
		{
			if (statement->child_count >= 3)
			{
				ASTNode *then_block = statement->children[1];
				ASTNode *else_block = statement->children[2];

				if (has_all_paths_return(then_block, return_type) &&
					has_all_paths_return(else_block, return_type))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void analyze_variable_usage(SymbolTable *table)
{
	if (!table || !table->buckets)
		return;

	for (int i = 0; i < table->bucket_count; i++)
	{
		Symbol *symbol = table->buckets[i];
		while (symbol)
		{
			if (symbol->usage_count == 0 && symbol->node && symbol->node->type != AST_PROGRAM)
			{
				semantic_warning(symbol->node, SemanticWarningMessages[SEMANTIC_UNUSED_VARIABLE], symbol->name);
			}
			symbol = symbol->next;
		}
	}

	if (table->parent)
	{
		analyze_variable_usage(table->parent);
	}
}

//
//  Note: || = Disallow
//        && = Allow
//
void check_type_cast_validity(CompleteType complete_LFrom, CompleteType complete_LTo, ASTNode *node)
{
	Type to = complete_LTo.type;
	Type from = complete_LTo.type;

	// just some testing
	return;

	//
	//  Same types are always valid
	//
	if (from == to)
		return;

	//
	//  Disallow IMPLICIT conversions between numeric types (int <-> float)
	//
	if (is_numeric_type(complete_LFrom) && is_numeric_type(complete_LTo))
		return;

	//
	//  Disallow casting from char to int (ASCII/UNICODE value conversion)
	//
	if (from == TYPE_CHAR && to == TYPE_INT)
		return;

	//
	//  Disallow casting from int to char (truncate to ASCII/UNICODE character)
	//
	if (from == TYPE_INT && to == TYPE_CHAR)
		return;

	//
	//  Disallow casting from boolean to int (false = 0, true = 1)
	//
	if (from == TYPE_BOOLEAN && to == TYPE_INT)
		return;

	//
	//  Disallow casting from int to boolean (0 = false, non-zero = true)
	//
	if (from == TYPE_INT && to == TYPE_BOOLEAN)
		return;

	//
	//  Disallow casting from/to void (void is not a value type)
	//
	if (from == TYPE_VOID || to == TYPE_VOID)
	{
		semantic_error(node, "Cannot cast to or from void type");
		return;
	}

	//
	//  Disallow casting from/to null in most cases (null is special)
	//
	if (from == TYPE_NULL || to == TYPE_NULL)
	{
		semantic_error(node, "Cannot cast to or from null type");
		return;
	}

	//
	//  Disallow casting between arrays and primitives
	//
	if (from == TYPE_ARRAY || to == TYPE_ARRAY)
	{
		semantic_error(node, "Cannot cast arrays to or from other types");
	}

	//
	//  Disallow casting between string and numeric types directly
	//
	if ((from == TYPE_STRING && is_numeric_type(complete_LTo)) ||
		(is_numeric_type(complete_LFrom) && to == TYPE_STRING))
	{
		semantic_error(node, "Cannot cast between string and numeric types");
		return;
	}

	//
	//  Disallow casting between string and boolean
	//
	if ((from == TYPE_STRING && to == TYPE_BOOLEAN) ||
		(from == TYPE_BOOLEAN && to == TYPE_STRING))
	{
		semantic_error(node, "Cannot cast between string and boolean");
		return;
	}

	//
	//  Disallow unknown type casts
	//
	if (from == TYPE_UNKNOWN || to == TYPE_UNKNOWN)
	{
		semantic_error(node, "Cannot cast unknown types");
		return;
	}
}

CompleteType analyze_array_access(ASTNode *node, SymbolTable *table)
{
	CompleteType type_unknown = {.type = TYPE_UNKNOWN};

	if (!node || node->child_count < 2)
		return type_unknown;

	ASTNode *array_node = node->children[0];
	ASTNode *index_node = node->children[1];

	CompleteType array_type = get_expression_type(array_node, table);
	if (array_type.type != TYPE_ARRAY)
	{
		semantic_error(node, "Invalid array access on '%s'", array_node->token.text ? array_node->token.text : "unknown");
		return type_unknown;
	}

	CompleteType index_type = get_expression_type(index_node, table);
	if (index_type.type != TYPE_INT)
	{
		semantic_error(node, "Array index must be an integer (got non-integer)");
		return type_unknown;
	}

	return *array_type.points_to;
}

CompleteType analyze_increment_expr(ASTNode *node, SymbolTable *table)
{
	CompleteType Unknown_type;
	Unknown_type.type = TYPE_UNKNOWN;

	if (!node || node->child_count < 2)
		return Unknown_type;

	ASTNode *target = node->children[0];
	CompleteType target_type = get_expression_type(target, table);

	if (!is_numeric_type(target_type))
	{
		semantic_error(node, SemanticErrorMessages[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES],
					   type_to_string(target_type), "numeric", node->children[1]->token.text);
		return Unknown_type;
	}

	return target_type;
}

CompleteType analyze_cast_expr(ASTNode *node, SymbolTable *table)
{
	CompleteType Unknown_type;
	Unknown_type.type = TYPE_UNKNOWN;
	if (!node || node->child_count < 2)
		return Unknown_type;

	ASTNode *type_node = node->children[0];
	ASTNode *expr_node = node->children[1];

	CompleteType to = get_expression_type(type_node, table);
	CompleteType from = get_expression_type(expr_node, table);

	check_type_cast_validity(from, to, node);

	annotate_node_type(node, to);
	return to;
}

CompleteType analyze_expression(ASTNode *expr_node, SymbolTable *table)
{
	CompleteType Unknown_type;
	Unknown_type.type = TYPE_UNKNOWN;

	if (!expr_node)
	{
		return Unknown_type;
	}
	CompleteType result = Unknown_type;

	if (expr_node->type == AST_BINARY_OP || expr_node->type == AST_BINARY_EXPR ||
		expr_node->type == AST_COMPARISON || expr_node->type == AST_LOGICAL_OP)
	{
		result = analyze_binary_op(expr_node, table);
	}
	else if (expr_node->type == AST_UNARY_OP || expr_node->type == AST_UNARY_EXPR)
	{
		result = analyze_unary_op(expr_node, table);
	}
	else if (expr_node->type == AST_ARRAY_ACCESS)
	{
		result = analyze_array_access(expr_node, table);
	}
	else if (expr_node->type == AST_INCREMENT_EXPR)
	{
		result = analyze_increment_expr(expr_node, table);
	}
	else if (expr_node->type == AST_CAST_EXPR)
	{
		result = analyze_cast_expr(expr_node, table);
	}
	else
	{
		result = get_expression_type(expr_node, table);
	}

	annotate_node_type(expr_node, result);
	return result;
}

CompleteType analyze_binary_op(ASTNode *binary_node, SymbolTable *table)
{
	CompleteType Unknown_type;
	Unknown_type.type = TYPE_UNKNOWN;

	CompleteType String_type;
	String_type.type = TYPE_STRING;

	CompleteType boolean_type;
	boolean_type.type = TYPE_BOOLEAN;

	if (!binary_node || binary_node->child_count < 2)
	{
		return Unknown_type;
	}
	CompleteType left_type = get_expression_type(binary_node->children[0], table);
	CompleteType right_type = get_expression_type(binary_node->children[1], table);

	if (left_type.type == TYPE_UNKNOWN || right_type.type == TYPE_UNKNOWN)
	{
		return Unknown_type;
	}

	//
	//  STRICT: No void in expressions
	//
	if (left_type.type == TYPE_VOID || right_type.type == TYPE_VOID)
	{
		semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_VOID_IN_EXPRESSION]);
		return Unknown_type;
	}

	//
	//  STRICT: No null in arithmetic
	//
	if (left_type.type == TYPE_NULL || right_type.type == TYPE_NULL)
	{
		semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_NULL_IN_ARITHMETIC]);
		return Unknown_type;
	}

	//
	//  STRICT: No arrays in arithmetic
	//
	if (left_type.type == TYPE_ARRAY || right_type.type == TYPE_ARRAY)
	{
		semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_ARRAY_IN_ARITHMETIC]);
		return Unknown_type;
	}

	TokenType op_type = binary_node->token.type;

	//
	//  Arithmetic operators: +, -, *, /, %, **
	//  Bitwise operators: |, ^, &, <<, >>, >>>, ~
	//
	if (op_type == TOKEN_PLUS || op_type == TOKEN_MINUS || op_type == TOKEN_ASTERISK ||
		op_type == TOKEN_SLASH || op_type == TOKEN_PERCENT || op_type == TOKEN_CARET ||
		op_type == TOKEN_EXPONENT || op_type == TOKEN_AMPERSAND || op_type == TOKEN_EXPONENT ||
		op_type == TOKEN_LEFT_SHIFT || op_type == TOKEN_RIGHT_SHIFT || op_type == TOKEN_BITWISE_OR ||
		op_type == TOKEN_BITWISE_AND || op_type == TOKEN_BITWISE_XOR || op_type == TOKEN_BITWISE_ZERO_FILL_LEFT_SHIFT ||
		op_type == TOKEN_BITWISE_ZERO_FILL_RIGHT_SHIFT || op_type == TOKEN_BITWISE_SIGNED_RIGHT_SHIFT || op_type == TOKEN_BITWISE_NOT)
	{

		if (op_type == TOKEN_PLUS && (left_type.type == TYPE_STRING || right_type.type == TYPE_STRING))
		{
			annotate_node_type(binary_node, String_type);
			return String_type;
		}

		//
		//  STRICT: String (non-plus) in arithmetic
		//
		if (left_type.type == TYPE_STRING || right_type.type == TYPE_STRING)
		{
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_STRING_NUMERIC_OPERATION]);
			return Unknown_type;
		}

		//
		//  STRICT: Boolean in arithmetic
		//
		if (left_type.type == TYPE_BOOLEAN || right_type.type == TYPE_BOOLEAN)
		{
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_BOOLEAN_NUMERIC_OPERATION]);
			return Unknown_type;
		}

		//
		//  STRICT: Both operands must be numeric
		//
		if (!is_numeric_type(left_type) || !is_numeric_type(right_type))
		{
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES],
						   type_to_string(left_type), type_to_string(right_type), binary_node->token.text);
			return Unknown_type;
		}

		//
		//  STRICT: No implicit int/float conversion - both must be same type
		//
		if (left_type.type != right_type.type)
		{
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_INT_FLOAT_MISMATCH]);
			return Unknown_type;
		}

		//
		//  Check for division by zero
		//
		if (op_type == TOKEN_SLASH)
		{
			if (binary_node->children[1]->type == AST_LITERAL &&
				strcmp(binary_node->children[1]->token.text, "0") == 0)
			{
				semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_DIVISION_BY_ZERO]);
			}
		}

		return left_type;
	}

	//
	//  Comparison operators: ==, !=, >, <, >=, <=
	//
	if (op_type == TOKEN_EQUALS || op_type == TOKEN_NOT_EQUALS ||
		op_type == TOKEN_GREATER || op_type == TOKEN_LESS ||
		op_type == TOKEN_GREATER_EQUALS || op_type == TOKEN_LESS_EQUALS)
	{

		//
		//  STRICT: Both operands must be numeric for comparison
		//
		if (!is_numeric_type(left_type) || !is_numeric_type(right_type))
		{
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES],
						   type_to_string(left_type), type_to_string(right_type), binary_node->token.text);
			return Unknown_type;
		}

		//
		//  STRICT: Must be same type for comparison
		//
		if (left_type.type != right_type.type)
		{
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_NO_IMPLICIT_CONVERSION],
						   type_to_string(left_type), type_to_string(right_type));
			return Unknown_type;
		}

		return boolean_type;
	}

	//
	//  Logical operators: && and ||
	//
	if (op_type == TOKEN_AND || op_type == TOKEN_OR)
	{
		//
		//  STRICT: Both operands must be boolean
		//
		if (!is_boolean_type(left_type) || !is_boolean_type(right_type))
		{
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES],
						   type_to_string(left_type), type_to_string(right_type), binary_node->token.text);
			return Unknown_type;
		}
		return boolean_type;
	}

	return Unknown_type;
}

CompleteType analyze_unary_op(ASTNode *unary_node, SymbolTable *table)
{
	CompleteType Unknown_type;
	Unknown_type.type = TYPE_UNKNOWN;

	CompleteType boolean_type;
	boolean_type.type = TYPE_BOOLEAN;

	if (!unary_node || unary_node->child_count < 1)
	{
		return Unknown_type;
	}

	CompleteType operand_type = get_expression_type(unary_node->children[0], table);

	if (operand_type.type == TYPE_UNKNOWN)
	{
		return Unknown_type;
	}

	TokenType op_type = unary_node->token.type;

	if (op_type == TOKEN_NOT)
	{
		if (!is_boolean_type(operand_type))
		{
			semantic_error(unary_node, SemanticErrorMessages[SEMANTIC_UNARY_OP_INVALID_TYPE], unary_node->token.text, "boolean");
			return Unknown_type;
		}
		return boolean_type;
	}

	if (op_type == TOKEN_MINUS)
	{
		if (!is_numeric_type(operand_type))
		{
			semantic_error(unary_node, SemanticErrorMessages[SEMANTIC_UNARY_OP_INVALID_TYPE], unary_node->token.text, "numeric");
			return Unknown_type;
		}
		return operand_type;
	}

	if (op_type == TOKEN_BITWISE_NOT)
	{
		if (!is_numeric_type(operand_type))
		{
			semantic_error(unary_node, SemanticErrorMessages[SEMANTIC_UNARY_OP_INVALID_TYPE], unary_node->token.text, "numeric");
			return Unknown_type;
		}
		return operand_type;
	}

	return Unknown_type;
}

//
//  Type System / Helper functions
//
CompleteType parse_type_recursive(const char *type_str)
{
	CompleteType Array_type = {.type = TYPE_ARRAY, .points_to = NULL};
	CompleteType Int_type = {.type = TYPE_INT};
	CompleteType Float_type = {.type = TYPE_FLOAT};
	CompleteType String_type = {.type = TYPE_STRING};
	CompleteType Char_type = {.type = TYPE_CHAR};
	CompleteType boolean_type = {.type = TYPE_BOOLEAN};
	CompleteType Null_type = {.type = TYPE_NULL};
	CompleteType Unknown_type = {.type = TYPE_UNKNOWN};
	CompleteType Void_type = {.type = TYPE_VOID};

	int type_len = strlen(type_str);
	if (type_len >= 2 && type_str[type_len - 2] == '[' && type_str[type_len - 1] == ']')
	{
		char inner[64];
		strncpy(inner, type_str, type_len - 2);
		inner[type_len - 2] = '\0';
		CompleteType inner_type = parse_type_recursive(inner);
		CompleteType arr;
		arr.type = TYPE_ARRAY;
		arr.points_to = malloc(sizeof(CompleteType));
		*arr.points_to = inner_type;
		return arr;
	}

	if (strcmp(type_str, "int") == 0)
		return Int_type;
	if (strcmp(type_str, "float") == 0)
		return Float_type;
	if (strcmp(type_str, "bool") == 0)
		return boolean_type;
	if (strcmp(type_str, "char") == 0)
		return Char_type;
	if (strcmp(type_str, "string") == 0)
		return String_type;
	return Unknown_type;
}

CompleteType get_expression_type(ASTNode *expr_node, SymbolTable *table)
{
	CompleteType Array_type = {.type = TYPE_ARRAY, .points_to = NULL};
	CompleteType Int_type = {.type = TYPE_INT};
	CompleteType Float_type = {.type = TYPE_FLOAT};
	CompleteType String_type = {.type = TYPE_STRING};
	CompleteType Char_type = {.type = TYPE_CHAR};
	CompleteType boolean_type = {.type = TYPE_BOOLEAN};
	CompleteType Null_type = {.type = TYPE_NULL};
	CompleteType Unknown_type = {.type = TYPE_UNKNOWN};
	CompleteType Void_type = {.type = TYPE_VOID};

	if (!expr_node)
		return Unknown_type;

	if (expr_node->type == AST_TYPE)
	{
		const char *text = expr_node->token.text;
		int len = text ? strlen(text) : 0;

		if (len >= 2 && text[len - 2] == '[' && text[len - 1] == ']')
		{
			CompleteType elem = Unknown_type;
			char base[64];
			strncpy(base, text, len - 2);
			base[len - 2] = '\0';

			CompleteType arr_type = parse_type_recursive(text);
			annotate_node_type(expr_node, arr_type);
			return arr_type;
		}

		switch (expr_node->token.type)
		{
		case TOKEN_INT:
			annotate_node_type(expr_node, Int_type);
			return Int_type;
		case TOKEN_FLOAT:
			annotate_node_type(expr_node, Float_type);
			return Float_type;
		case TOKEN_STRING:
			annotate_node_type(expr_node, String_type);
			return String_type;
		case TOKEN_BOOLEAN:
			annotate_node_type(expr_node, boolean_type);
			return boolean_type;
		case TOKEN_CHAR:
			annotate_node_type(expr_node, Char_type);
			return Char_type;
		case TOKEN_NULL:
			annotate_node_type(expr_node, Null_type);
			return Null_type;
		case TOKEN_VOID:
			annotate_node_type(expr_node, Void_type);
			return Void_type;
		default:
			annotate_node_type(expr_node, Unknown_type);
			return Unknown_type;
		}
	}

	if (expr_node->type == AST_ARRAY_LITERAL)
	{
		CompleteType arr;
		arr.type = TYPE_ARRAY;
		arr.points_to = NULL;

		if (expr_node->child_count > 0)
		{
			CompleteType first_elem_type = get_expression_type(expr_node->children[0], table);

			arr.points_to = malloc(sizeof(CompleteType));
			*arr.points_to = first_elem_type;

			for (int i = 1; i < expr_node->child_count; i++)
			{
				CompleteType elem_type = get_expression_type(expr_node->children[i], table);

				if (first_elem_type.type != elem_type.type)
				{
					semantic_error(expr_node->children[i],
                       SemanticErrorMessages[SEMANTIC_MISMATCHED_ARRAY_ELEMENT_TYPE],
                       i,
                       type_to_string(elem_type),
                       type_to_string(first_elem_type));
					
					exit(1);
				}
			}
		}

		annotate_node_type(expr_node, arr);
		return arr;
	}

	if (expr_node->type == AST_LITERAL)
	{
		CompleteType inferred = Unknown_type;
		switch (expr_node->token.type)
		{
		case TOKEN_INT_LITERAL:
			inferred = Int_type;
			break;

		case TOKEN_FLOAT_LITERAL:
			inferred = Float_type;
			break;

		case TOKEN_STRING:
			inferred = String_type;
			break;

		case TOKEN_TRUE:
		case TOKEN_FALSE:
		case TOKEN_BOOLEAN:
			inferred = boolean_type;
			break;

		case TOKEN_CHAR:
			inferred = Char_type;
			break;

		case TOKEN_NULL:
			inferred = Null_type;
			break;

		default:
			inferred = Unknown_type;
			break;
		}
		annotate_node_type(expr_node, inferred);
		return inferred;
	}

	if (expr_node->type == AST_IDENTIFIER && table)
	{
		Symbol *s = lookup_symbol(table, expr_node->token.text);
		CompleteType t = s ? s->type : Unknown_type;
		annotate_node_type(expr_node, t);
		return t;
	}

	if (expr_node->child_count > 0)
	{
		CompleteType t = get_expression_type(expr_node->children[0], table);
		annotate_node_type(expr_node, t);
		return t;
	}

	annotate_node_type(expr_node, Unknown_type);
	return Unknown_type;
}

bool check_type_compatibility(CompleteType expected, CompleteType actual)
{
	if (expected.type == TYPE_UNKNOWN || actual.type == TYPE_UNKNOWN)
		return false;

	if (expected.type == TYPE_ARRAY || actual.type == TYPE_ARRAY)
	{
		if (expected.type != TYPE_ARRAY || actual.type != TYPE_ARRAY)
			return false;

		if (!expected.points_to || !actual.points_to)
			return false;

		return check_type_compatibility(*expected.points_to, *actual.points_to);
	}

	if (expected.type == actual.type)
		return true;

	if (is_numeric_type(expected) && is_numeric_type(actual))
		return true;

	if (is_numeric_type(expected) && actual.type == TYPE_CHAR)
		return true;
		
	if (actual.type == TYPE_NULL &&
		(expected.type == TYPE_STRING ||
		 expected.type == TYPE_ARRAY ||
		 expected.type == TYPE_NULL))
		return true;

	return false;
}

bool is_numeric_type(CompleteType type)
{
	return type.type == TYPE_INT || type.type == TYPE_FLOAT;
}

bool is_boolean_type(CompleteType type)
{
	return type.type == TYPE_BOOLEAN;
}

//
//  Convert Type enum to string for error messages
//
const char *type_to_string(CompleteType c_type)
{
	Type type = c_type.type;

	switch (type)
	{
	case TYPE_INT:
		return "int";

	case TYPE_FLOAT:
		return "float";

	case TYPE_STRING:
		return "string";

	case TYPE_BOOLEAN:
		return "bool";

	case TYPE_CHAR:
		return "char";

	case TYPE_NULL:
		return "null";

	case TYPE_VOID:
		return "void";

	case TYPE_ARRAY:
		return "array";

	case TYPE_UNKNOWN:
		return "unknown";

	default:
		return "<invalid>";
	}
}

//
//  Error Handling (Errors, Warnings, Tips, etc.)
//
void semantic_error(ASTNode *node, const char *fmt, ...)
{
	if (node == NULL)
		return;
	semantic_error_count++;

	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	log_semantic_error(node, "%s", buffer);
}

void semantic_warning(ASTNode *node, const char *fmt, ...)
{
	if (node == NULL)
		return;
	semantic_warning_count++;

	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	log_semantic_warning(node, "%s", buffer);
}

void semantic_tip(ASTNode *node, const char *fmt, ...)
{
	if (node == NULL)
		return;
	semantic_tip_count++;

	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	log_semantic_tip(node, "%s", buffer);
}

//
//  AST Annotation and Utilities
//
void annotate_node_type(ASTNode *node, CompleteType type)
{
	if (!node)
		return;
	node->annotated_type = type;
}

void free_symbol_table(SymbolTable *table)
{
	if (!table)
		return;
	if (table->buckets)
	{
		for (int index = 0; index < table->bucket_count; index++)
		{
			Symbol *current_symbol = table->buckets[index];

			while (current_symbol)
			{
				Symbol *next_symbol = current_symbol->next;
				if (current_symbol->name)
					free(current_symbol->name);
				free(current_symbol);
				current_symbol = next_symbol;
			}
		}

		free(table->buckets);
		table->buckets = NULL;
	}

	free(table);
}
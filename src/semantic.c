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

// Track diagnostics so we can report multiple messages before halting.
static int semantic_error_count = 0;
static int semantic_warning_count = 0;
static int semantic_tip_count = 0;

int semantic_get_error_count() { return semantic_error_count; }
int semantic_get_warning_count() { return semantic_warning_count; }
int semantic_get_tip_count() { return semantic_tip_count; }

const char* type_to_string(Type type);

SymbolTable* init_symbol_table() {
	SymbolTable* table = malloc(sizeof(SymbolTable));

	table->bucket_count = 64;
	table->buckets = calloc(table->bucket_count, sizeof(Symbol*));
	table->parent = NULL;
	table->loop_depth = 0;
	table->current_return_type = TYPE_VOID;

	return table;
}

void enter_scope(SymbolTable* table) {
	SymbolTable* saved = malloc(sizeof(SymbolTable));
	if (!saved) return;

	*saved = *table;

	table->parent = saved;
	table->buckets = calloc(saved->bucket_count, sizeof(Symbol*));
	table->loop_depth = saved->loop_depth;
	table->current_return_type = saved->current_return_type;
}

void exit_scope(SymbolTable* table) {
	if (!table->parent) return;

	for (int i = 0; i < table->bucket_count; i++) {
		Symbol* sym = table->buckets[i];
		while (sym) {
			Symbol* next = sym->next;
			free(sym->name);
			free(sym);
			sym = next;
		}
	}

	free(table->buckets);

	SymbolTable* parent = table->parent;
	*table = *parent;
	free(parent);
}

bool add_symbol(SymbolTable* table, const char* name, Type type, ASTNode* node) {
	if (!table || !name) return false;

	unsigned long hash = hash_string(name);
	int index = (int)(hash % table->bucket_count);

	//
	//  Check for duplicates in the current scope
	//
	Symbol* current_symbol = table->buckets[index];
	while (current_symbol) {
		if (strcmp(current_symbol->name, name) == 0) return false;
		current_symbol = current_symbol->next;
	}

	// 
	//  Allocate a new symbol
	// 
	Symbol* new_symbol = malloc(sizeof(Symbol));
	if (!new_symbol) return false;

	new_symbol->name = strdup(name);
	if (!new_symbol->name) {
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

Symbol* lookup_symbol(SymbolTable* table, const char* name) {
	if (!table || !name) return NULL;

	while (table) {
		if (!table->buckets || table->bucket_count <= 0) {
			table = table->parent;
			continue;
		}

		unsigned long hash = hash_string(name);
		int index = (int)(hash % table->bucket_count);
		
		Symbol* current_symbol = table->buckets[index];
		while (current_symbol) {
			if (strcmp(current_symbol->name, name) == 0) {
				current_symbol->usage_count++;
				return current_symbol;
			}
			current_symbol = current_symbol->next;
		}
		table = table->parent;
	}
	
	return NULL;
}

bool symbol_in_scope(SymbolTable* table, const char* name) {
	if (!table || !name) return false;

	unsigned long hash = hash_string(name);
	int index = (int)(hash % table->bucket_count);

	Symbol* current_symbol = table->buckets[index];
	while (current_symbol) {
		if (strcmp(current_symbol->name, name) == 0) return true;
		current_symbol = current_symbol->next;
	}

	return false;
}

// 
//  AST Traversal / Semantic Checks
//
void analyze_block(ASTNode* block, SymbolTable* table) {
	if (block == NULL || table == NULL) return;
	if (block->type != AST_BLOCK) return;

	for (int i = 0; i < block->child_count; i++) {
		analyze_statement(block->children[i], table);
	}
}

void analyze_statement(ASTNode* statement, SymbolTable* table) {
	if (statement == NULL || table == NULL) return;
	switch (statement->type) {
		case AST_ASSIGNMENT:
			analyze_assignment(statement, table);
			break;

		case AST_DECLARATION:
			analyze_declaration(statement, table);
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
		
		case AST_BLOCK:
			analyze_block(statement, table);
			break;

		case AST_INCLUDE:
			analyze_include(statement, table);
			break;

		case AST_ARRAY_ACCESS:
			analyze_array_access(statement, table);
			break;
		
		case AST_EXPRESSION:
		case AST_BINARY_OP:
		case AST_BINARY_EXPR:
		case AST_UNARY_OP:
		case AST_UNARY_EXPR:
		case AST_COMPARISON:
		case AST_LOGICAL_OP:
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

void analyze_file(ASTNode* file_node, SymbolTable* table) {
	if (!file_node || !table) return;
	if (file_node->type != AST_FILE) return;
	
	for (int i = 0; i < file_node->child_count; i++) {
		ASTNode* child = file_node->children[i];
		if (child->type == AST_INCLUDE) {
			analyze_include(child, table);
		} else if (child->type == AST_PROGRAM) {
			analyze_program(child, table);
		}
	}
}

void analyze_program(ASTNode* func_node, SymbolTable* table) {
	if (!func_node || !table) return;
	if (func_node->type != AST_PROGRAM) return;
	ASTNode* return_type_node = func_node->children[0];
	ASTNode* func_name_node = func_node->children[1];
	ASTNode* params_node = func_node->children[2];
	ASTNode* block_node = func_node->children[3];

	Type return_type = get_expression_type(return_type_node, table);
	table->current_return_type = return_type;

	if (!add_symbol(table, func_name_node->token.text, return_type, func_node)) {
		semantic_error(func_name_node, SemanticErrorMessages[SEMANTIC_DUPLICATE_SYMBOL], func_name_node->token.text);
		return;
	}

	enter_scope(table);
	if (params_node && params_node->type == AST_PARAMS) {
		for (int i = 0; i < params_node->child_count; i++) {
			ASTNode* param = params_node->children[i];
			Type param_type = get_expression_type(param->children[1], table);

			const char* param_name = param->children[0]->token.text;
			if (!add_symbol(table, param_name, param_type, param)) {
				semantic_error(param, SemanticErrorMessages[SEMANTIC_DUPLICATE_SYMBOL], param_name);
			}
		}
	}

	if (block_node && block_node->type == AST_BLOCK) {
		analyze_block(block_node, table);
	}

	if (return_type != TYPE_VOID) {
		bool has_return = false;
		for (int i = 0; i < block_node->child_count; i++) {
			ASTNode* stmt = block_node->children[i];
			if (stmt->type == AST_RETURN) {
				has_return = true;
				Type actual = get_expression_type(stmt->children[0], table);
				if (!check_type_compatibility(return_type, actual)) {
					semantic_error(stmt, SemanticErrorMessages[SEMANTIC_RETURN_TYPE_MISMATCH], return_type_node->token.text, stmt->children[0]->token.text);
				}
			}
		}
		if (!has_return) {
			semantic_error(func_node, SemanticErrorMessages[SEMANTIC_MISSING_RETURN]);
		}
	}

	exit_scope(table);
}

void analyze_for(ASTNode* for_node, SymbolTable* table) {
	if (!for_node || !table) return;
	if (for_node->type != AST_FOR) return;

	ASTNode* assignment_node = for_node->children[0];
	ASTNode* condition_node = for_node->children[1];
	ASTNode* increment_node = for_node->children[2];
	ASTNode* block_node = for_node->children[3];

	if (assignment_node == NULL) return;

	enter_scope(table);
	analyze_statement(assignment_node, table);

	Type cond_type = get_expression_type(condition_node, table);
	if (cond_type != TYPE_BOOLEAN) {
		semantic_error(for_node, SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH], type_to_string(cond_type), type_to_string(TYPE_BOOLEAN));
	}

	if (increment_node == NULL) {
		semantic_tip(for_node, SemanticTipMessages[SEMANTIC_TIP_PREFER_WHILE_LOOP]);
	} else {
		analyze_statement(increment_node, table);
	}

	if (block_node && block_node->type == AST_BLOCK) {
		table->loop_depth++;
		analyze_block(block_node, table);
		table->loop_depth--;
	}

	exit_scope(table);
}

void analyze_if(ASTNode* if_node, SymbolTable* table) {
	if (!if_node || !table) return;
	if (if_node->type != AST_IF) return;

	ASTNode* condition_node = if_node->children[0];
	ASTNode* block_node = if_node->children[1];

	if (condition_node == NULL) return;

	enter_scope(table);
	analyze_statement(condition_node, table);

	Type cond_type = get_expression_type(condition_node, table);
	if (cond_type != TYPE_BOOLEAN) {
		semantic_error(if_node, SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH], type_to_string(cond_type), type_to_string(TYPE_BOOLEAN));
	}

	if (block_node && block_node->type == AST_BLOCK) {
		analyze_block(block_node, table);
	}

	exit_scope(table);

	if (if_node->child_count >= 3) {
		ASTNode* else_block = if_node->children[2];
		if (else_block && else_block->type == AST_BLOCK) {
			enter_scope(table);
			analyze_block(else_block, table);
			exit_scope(table);
		} else if (else_block && else_block->type == AST_IF) {
			analyze_statement(else_block, table);
		}
	}
}

void analyze_while(ASTNode* while_node, SymbolTable* table) {
	if (!while_node || !table) return;
	if (while_node->type != AST_WHILE) return;

	ASTNode* condition_node = while_node->children[0];
	ASTNode* block_node = while_node->children[1];

	if (condition_node == NULL) return;

	enter_scope(table);
	analyze_statement(condition_node, table);

	Type cond_type = get_expression_type(condition_node, table);
	if (cond_type != TYPE_BOOLEAN) {
		semantic_error(while_node, SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH], type_to_string(cond_type), type_to_string(TYPE_BOOLEAN));
	}

	if (block_node && block_node->type == AST_BLOCK) {
		table->loop_depth++;
		analyze_block(block_node, table);
		table->loop_depth--;
	}

	exit_scope(table);
}

void analyze_return(ASTNode* return_node, SymbolTable* table) {
	if (!return_node || !table) return;
	if (return_node->type != AST_RETURN) return;

	Type return_type = table->current_return_type;
	if (return_node->child_count > 0) {
		ASTNode* child_node = return_node->children[0];
	
		analyze_expression(child_node, table);
	
		Type actual_type = get_expression_type(child_node, table);
		if (return_type != actual_type) {
			semantic_error(return_node, SemanticErrorMessages[SEMANTIC_RETURN_VALUE_TYPE_MISMATCH], 
				type_to_string(actual_type), type_to_string(return_type));
		}
	} else {
		if (return_type != TYPE_VOID) {
			semantic_error(return_node, SemanticErrorMessages[SEMANTIC_NON_VOID_RETURN_WITHOUT_VALUE], 
				type_to_string(return_type));
		}
	}
}

void analyze_break(ASTNode* break_node, SymbolTable* table) {
	if (!break_node || !table) return;
	if (break_node->type != AST_BREAK) return;

	int depth = table->loop_depth;
	if (depth <= 0) {
		semantic_error(break_node, SemanticErrorMessages[SEMANTIC_BREAK_OUTSIDE_LOOP]);
	}
}

void analyze_declaration(ASTNode* declaration_node, SymbolTable* table) {
	if (!declaration_node || !table) return;

	ASTNode* identifier_node = declaration_node->children[0];
	ASTNode* type_node = declaration_node->children[1];

	Type expected_type = get_expression_type(type_node, table);
	if (declaration_node->child_count >= 3) {
		ASTNode* expression_node = declaration_node->children[2];
	
		analyze_expression(expression_node, table);
	
		Type actual_type = get_expression_type(expression_node, table);
		if (actual_type != expected_type) {
			semantic_error(declaration_node, SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH], type_node->token.text, expression_node->token.text);
			return;
		}
	}

	add_symbol(table, identifier_node->token.text, expected_type, declaration_node);
}

// 
//  See the visualized form (Tree): `../diagrams/variable_assignment.txt`
// 
void analyze_assignment(ASTNode* assignment_node, SymbolTable* table) {
	if (!assignment_node || !table) return;
	
	ASTNode* identifier_node = assignment_node->children[0];
	ASTNode* expression_node = assignment_node->children[1];

	analyze_expression(expression_node, table);

	Symbol* identifier_symbol = lookup_symbol(table, identifier_node->token.text);
	Type expression_type = get_expression_type(expression_node, table);
	
	if (identifier_symbol == NULL) {
		semantic_error(assignment_node, SemanticErrorMessages[SEMANTIC_UNKNOWN_VARIABLE], identifier_node->token.text);
		return;
	}

	// 
	//  STRICT: No implicit type conversion in assignment
	// 
	if (identifier_symbol->type != expression_type) {
		return;
	}
}

// 
//  include <publisher>.<package>;
// 
//  include adan.io; // Import ADAN's native I/O library.
// 
void analyze_include(ASTNode* include_node, SymbolTable* table) {
	if (!include_node || !table) return;
	
	if (include_node->type != AST_INCLUDE) return;
	if (include_node->child_count < 2) return;
	
	ASTNode* publisher_node = include_node->children[0];
	ASTNode* package_node = include_node->children[1];
	
	if (!publisher_node || !package_node) return;
	if (!publisher_node->token.text || !package_node->token.text) return;
	
	extern LibraryRegistry* global_library_registry;
	
	Library* lib = load_library(global_library_registry, 
								publisher_node->token.text, 
								package_node->token.text);
	
	if (!lib) {
		log_semantic_error(include_node, "Failed to load library %s.%s", 
						  publisher_node->token.text, package_node->token.text);
		return;
	}
	
	if (!import_library_symbols(lib, table)) {
		log_semantic_error(include_node, "Failed to import symbols from %s.%s",
						  publisher_node->token.text, package_node->token.text);
	}
}

void analyze_function_call(ASTNode* call_node, SymbolTable* table) {
	if (!call_node || !table) return;
	if (call_node->type != AST_FUNCTION_CALL) return;
	if (call_node->child_count < 1) return;

	ASTNode* func_name_node = call_node->children[0];
	Symbol* func_symbol = lookup_symbol(table, func_name_node->token.text);

	if (func_symbol == NULL) {
		semantic_error(call_node, SemanticErrorMessages[SEMANTIC_FUNCTION_NOT_FOUND], func_name_node->token.text);
		return;
	}

	int arg_count = 0;
	Type first_arg_type = TYPE_UNKNOWN;
	if (call_node->child_count > 1) {
		ASTNode* params_node = call_node->children[1];
		if (params_node && params_node->type == AST_PARAMS) {
			arg_count = params_node->child_count;
			for (int i = 0; i < arg_count; i++) {
				ASTNode* arg = params_node->children[i];
				analyze_expression(arg, table);
				if (i == 0) {
					first_arg_type = get_expression_type(arg, table);
				}
			}
		}
	}

	// Auto-resolve print/println to type-specific variants
	const char* resolved_name = func_name_node->token.text;
	char resolved_buffer[256];
	if ((strcmp(resolved_name, "print") == 0 || strcmp(resolved_name, "println") == 0) && arg_count == 1) {
		if (first_arg_type == TYPE_INT) {
			snprintf(resolved_buffer, sizeof(resolved_buffer), "%s_int", resolved_name);
			resolved_name = resolved_buffer;
			// Update the function name in the AST
			free((char*)func_name_node->token.text);
			func_name_node->token.text = strdup(resolved_name);
		} else if (first_arg_type == TYPE_FLOAT) {
			snprintf(resolved_buffer, sizeof(resolved_buffer), "%s_float", resolved_name);
			resolved_name = resolved_buffer;
			// Update the function name in the AST
			free((char*)func_name_node->token.text);
			func_name_node->token.text = strdup(resolved_name);
		}
	}

	extern LibraryRegistry* global_library_registry;
	if (global_library_registry) {
		Library* lib = global_library_registry->libraries;
		while (lib) {
			LibraryFunction* func = lib->functions;
			while (func) {
				if (strcmp(func->name, resolved_name) == 0) {
					if (arg_count != func->param_count) {
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

void check_entry_point(SymbolTable* table) {
	if (!table) return;

	Symbol* main_symbol = lookup_symbol(table, "main");

	if (main_symbol == NULL) return;
}

void analyze_array_literal(ASTNode* node, SymbolTable* table) {
	if (!node || node->type != AST_ARRAY_LITERAL) return;
	if (!validate_array_element_types(node, table)) semantic_error(node, SemanticErrorMessages[SEMANTIC_ARRAY_MIXED_TYPES]);
}

bool validate_array_element_types(ASTNode* array_node, SymbolTable* table) {
	if (array_node->child_count == 0) return true;

	Type first_type = get_expression_type(array_node->children[0], table);
	for (int i = 1; i < array_node->child_count; i++) {
		Type current_type = get_expression_type(array_node->children[i], table);
		if (current_type != first_type) return false;
	}

	return true;
}

void check_unreachable_code(ASTNode* block, SymbolTable* table) {
	if (!block || block->type != AST_BLOCK) return;
	for (int i = 0; i < block->child_count - 1; i++) {
		ASTNode* statement = block->children[i];
		
		if (statement->type == AST_RETURN || statement->type == AST_BREAK) {
			ASTNode* next_statement = block->children[i + 1];
			semantic_warning(next_statement, SemanticWarningMessages[SEMANTIC_UNREACHABLE_CODE], next_statement->token.text);
		}
	}
}

bool has_all_paths_return(ASTNode* block, Type return_type) {
	if (!block || block->type != AST_BLOCK) return false;
	for (int i = block->child_count - 1; i >= 0; i--) {
		ASTNode* statement = block->children[i];

		if (statement->type == AST_RETURN) return true;
		if (statement->type == AST_IF) {
			if (statement->child_count >= 3) {
				ASTNode* then_block = statement->children[1];
				ASTNode* else_block = statement->children[2];

				if (has_all_paths_return(then_block, return_type) &&
					has_all_paths_return(else_block, return_type)) {
						return true;
				}
			}
		}
	}

	return false;
}

void analyze_variable_usage(SymbolTable* table) {
	if (!table || !table->buckets) return;
	
	for (int i = 0; i < table->bucket_count; i++) {
		Symbol* symbol = table->buckets[i];
		while (symbol) {
			if (symbol->usage_count == 0 && symbol->node) {
				semantic_warning(symbol->node, SemanticWarningMessages[SEMANTIC_UNUSED_VARIABLE], symbol->name);
			}
			symbol = symbol->next;
		}
	}
	
	if (table->parent) {
		analyze_variable_usage(table->parent);
	}
}

// 
//  Note: || = Disallow
//        && = Allow
// 
void check_type_cast_validity(Type from, Type to, ASTNode* node) {
	// 
	//  Same types are always valid
	// 
	if (from == to) return;

	// 
	//  Disallow IMPLICIT conversions between numeric types (int <-> float)
	// 
	if (is_numeric_type(from) && is_numeric_type(to)) return;
	
	// 
	//  Disallow casting from char to int (ASCII/UNICODE value conversion)
	// 
	if (from == TYPE_CHAR && to == TYPE_INT) return;
	
	// 
	//  Disallow casting from int to char (truncate to ASCII/UNICODE character)
	// 
	if (from == TYPE_INT && to == TYPE_CHAR) return;
	
	// 
	//  Disallow casting from boolean to int (false = 0, true = 1)
	// 
	if (from == TYPE_BOOLEAN && to == TYPE_INT) return;
	
	// 
	//  Disallow casting from int to boolean (0 = false, non-zero = true)
	// 
	if (from == TYPE_INT && to == TYPE_BOOLEAN) return;
	
	// 
	//  Disallow casting from/to void (void is not a value type)
	// 
	if (from == TYPE_VOID || to == TYPE_VOID) {
		semantic_error(node, "Cannot cast to or from void type");
		return;
	}
	
	// 
	//  Disallow casting from/to null in most cases (null is special)
	// 
	if (from == TYPE_NULL || to == TYPE_NULL) {
		semantic_error(node, "Cannot cast to or from null type");
		return;
	}
	
	// 
	//  Disallow casting between arrays and primitives
	// 
	if (from == TYPE_ARRAY || to == TYPE_ARRAY) {
		semantic_error(node, "Cannot cast arrays to or from other types");
		return;
	}
	
	// 
	//  Disallow casting between string and numeric types directly
	// 
	if ((from == TYPE_STRING && is_numeric_type(to)) || 
		(is_numeric_type(from) && to == TYPE_STRING)) {
		semantic_error(node, "Cannot cast between string and numeric types");
		return;
	}
	
	// 
	//  Disallow casting between string and boolean
	// 
	if ((from == TYPE_STRING && to == TYPE_BOOLEAN) || 
		(from == TYPE_BOOLEAN && to == TYPE_STRING)) {
		semantic_error(node, "Cannot cast between string and boolean");
		return;
	}
	
	// 
	//  Disallow unknown type casts
	// 
	if (from == TYPE_UNKNOWN || to == TYPE_UNKNOWN) {
		semantic_error(node, "Cannot cast unknown types");
		return;
	}
}

// ...existing code...

Type analyze_array_access(ASTNode* node, SymbolTable* table) {
	if (!node || node->child_count < 2) {
		return TYPE_UNKNOWN;
	}

	ASTNode* array_node = node->children[0];
	ASTNode* index_node = node->children[1];

	Type array_type = get_expression_type(array_node, table);

	if (array_type != TYPE_ARRAY) {
		semantic_error(node, SemanticErrorMessages[SEMANTIC_INVALID_ARRAY_ACCESS], array_node->token.text ? array_node->token.text : "unknown");
		return TYPE_UNKNOWN;
	}

	Type index_type = get_expression_type(index_node, table);

	if (index_type != TYPE_INT) {
		semantic_error(node, SemanticErrorMessages[SEMANTIC_ARRAY_INDEX_NOT_INTEGER], "non-integer");
		return TYPE_UNKNOWN;
	}

	return TYPE_UNKNOWN;
}

Type analyze_increment_expr(ASTNode* node, SymbolTable* table) {
	if (!node || node->child_count < 2) return TYPE_UNKNOWN;

	ASTNode* target = node->children[0];
	Type target_type = get_expression_type(target, table);

	if (!is_numeric_type(target_type)) {
		semantic_error(node, SemanticErrorMessages[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES],
					type_to_string(target_type), "numeric", node->children[1]->token.text);
		return TYPE_UNKNOWN;
	}

	return target_type;
}

Type analyze_expression(ASTNode* expr_node, SymbolTable* table) {
	if (!expr_node) {
		return TYPE_UNKNOWN;
	}
	Type result = TYPE_UNKNOWN;

	if (expr_node->type == AST_BINARY_OP || expr_node->type == AST_BINARY_EXPR ||
		expr_node->type == AST_COMPARISON || expr_node->type == AST_LOGICAL_OP) {
		result = analyze_binary_op(expr_node, table);
	} else if (expr_node->type == AST_UNARY_OP || expr_node->type == AST_UNARY_EXPR) {
		result = analyze_unary_op(expr_node, table);
	} else if (expr_node->type == AST_ARRAY_ACCESS) {
		result = analyze_array_access(expr_node, table);
	} else if (expr_node->type == AST_INCREMENT_EXPR) {
		result = analyze_increment_expr(expr_node, table);
	} else {
		result = get_expression_type(expr_node, table);
	}

	annotate_node_type(expr_node, result);
	return result;
}

Type analyze_binary_op(ASTNode* binary_node, SymbolTable* table) {
	if (!binary_node || binary_node->child_count < 2) {
		return TYPE_UNKNOWN;
	}
	Type left_type = get_expression_type(binary_node->children[0], table);
	Type right_type = get_expression_type(binary_node->children[1], table);

	if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
		return TYPE_UNKNOWN;
	}

	// 
	//  STRICT: No void in expressions
	// 
	if (left_type == TYPE_VOID || right_type == TYPE_VOID) {
		semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_VOID_IN_EXPRESSION]);
		return TYPE_UNKNOWN;
	}

	// 
	//  STRICT: No null in arithmetic
	// 
	if (left_type == TYPE_NULL || right_type == TYPE_NULL) {
		semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_NULL_IN_ARITHMETIC]);
		return TYPE_UNKNOWN;
	}

	// 
	//  STRICT: No arrays in arithmetic
	// 
	if (left_type == TYPE_ARRAY || right_type == TYPE_ARRAY) {
		semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_ARRAY_IN_ARITHMETIC]);
		return TYPE_UNKNOWN;
	}

	TokenType op_type = binary_node->token.type;

	// 
	//  Arithmetic operators: +, -, *, /, %, ^
	// 
	if (op_type == TOKEN_PLUS || op_type == TOKEN_MINUS || op_type == TOKEN_ASTERISK ||
		op_type == TOKEN_SLASH || op_type == TOKEN_PERCENT || op_type == TOKEN_CAROT) {
		
		// 
		//  STRICT: String in arithmetic
		// 
		if (left_type == TYPE_STRING || right_type == TYPE_STRING) {
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_STRING_NUMERIC_OPERATION]);
			return TYPE_UNKNOWN;
		}

		// 
		//  STRICT: Boolean in arithmetic
		// 
		if (left_type == TYPE_BOOLEAN || right_type == TYPE_BOOLEAN) {
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_BOOLEAN_NUMERIC_OPERATION]);
			return TYPE_UNKNOWN;
		}

		// 
		//  STRICT: Both operands must be numeric
		// 
		if (!is_numeric_type(left_type) || !is_numeric_type(right_type)) {
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES],
				type_to_string(left_type), type_to_string(right_type), binary_node->token.text);
			return TYPE_UNKNOWN;
		}

		// 
		//  STRICT: No implicit int/float conversion - both must be same type
		// 
		if (left_type != right_type) {
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_INT_FLOAT_MISMATCH]);
			return TYPE_UNKNOWN;
		}

		// 
		//  Check for division by zero
		// 
		if (op_type == TOKEN_SLASH) {
			if (binary_node->children[1]->type == AST_LITERAL &&
				strcmp(binary_node->children[1]->token.text, "0") == 0) {
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
		op_type == TOKEN_GREATER_EQUALS || op_type == TOKEN_LESS_EQUALS) {
		
		// 
		//  STRICT: Both operands must be numeric for comparison
		// 
		if (!is_numeric_type(left_type) || !is_numeric_type(right_type)) {
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES],
				type_to_string(left_type), type_to_string(right_type), binary_node->token.text);
			return TYPE_UNKNOWN;
		}

		// 
		//  STRICT: Must be same type for comparison
		// 
		if (left_type != right_type) {
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_NO_IMPLICIT_CONVERSION],
				type_to_string(left_type), type_to_string(right_type));
			return TYPE_UNKNOWN;
		}

		return TYPE_BOOLEAN;
	}

	// 
	//  Logical operators: &&
	// 
	if (op_type == TOKEN_AND) {
		// 
		//  STRICT: Both operands must be boolean
		// 
		if (!is_boolean_type(left_type) || !is_boolean_type(right_type)) {
			semantic_error(binary_node, SemanticErrorMessages[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES],
				type_to_string(left_type), type_to_string(right_type), binary_node->token.text);
			return TYPE_UNKNOWN;
		}
		return TYPE_BOOLEAN;
	}

	return TYPE_UNKNOWN;
}

Type analyze_unary_op(ASTNode* unary_node, SymbolTable* table) {
	if (!unary_node || unary_node->child_count < 1) {
		return TYPE_UNKNOWN;
	}

	Type operand_type = get_expression_type(unary_node->children[0], table);

	if (operand_type == TYPE_UNKNOWN) {
		return TYPE_UNKNOWN;
	}

	TokenType op_type = unary_node->token.type;

	if (op_type == TOKEN_NOT) {
		if (!is_boolean_type(operand_type)) {
			semantic_error(unary_node, SemanticErrorMessages[SEMANTIC_UNARY_OP_INVALID_TYPE], unary_node->token.text, "boolean");
			return TYPE_UNKNOWN;
		}
		return TYPE_BOOLEAN;
	}

	if (op_type == TOKEN_MINUS) {
		if (!is_numeric_type(operand_type)) {
			semantic_error(unary_node, SemanticErrorMessages[SEMANTIC_UNARY_OP_INVALID_TYPE], unary_node->token.text, "numeric");
			return TYPE_UNKNOWN;
		}
		return operand_type;
	}

	return TYPE_UNKNOWN;
}

// 
//  Type System / Helper functions
// 
Type get_expression_type(ASTNode* expr_node, SymbolTable* table) {
	if (!expr_node) return TYPE_UNKNOWN;
	if (expr_node->type == AST_TYPE) {
		switch (expr_node->token.type) {
			case TOKEN_INT:
				annotate_node_type(expr_node, TYPE_INT);
				return TYPE_INT;

			case TOKEN_FLOAT:
				annotate_node_type(expr_node, TYPE_FLOAT);
				return TYPE_FLOAT;

			case TOKEN_STRING:
				annotate_node_type(expr_node, TYPE_STRING);
				return TYPE_STRING;

			case TOKEN_BOOLEAN:
				annotate_node_type(expr_node, TYPE_BOOLEAN);
				return TYPE_BOOLEAN;

			case TOKEN_CHAR:
				annotate_node_type(expr_node, TYPE_CHAR);
				return TYPE_CHAR;

			case TOKEN_NULL:
				annotate_node_type(expr_node, TYPE_NULL);
				return TYPE_NULL;

			case TOKEN_VOID:
				annotate_node_type(expr_node, TYPE_VOID);
				return TYPE_VOID;

			case TOKEN_ARRAY:
				annotate_node_type(expr_node, TYPE_ARRAY);
				return TYPE_ARRAY;

			default:
				annotate_node_type(expr_node, TYPE_UNKNOWN);
				return TYPE_UNKNOWN;
		}
	}
	if (expr_node->type == AST_LITERAL) {
		TokenType tt = expr_node->token.type;
		Type inferred = TYPE_UNKNOWN;

		switch (tt) {
			case TOKEN_INT_LITERAL:
				inferred = TYPE_INT;
				break;

			case TOKEN_FLOAT_LITERAL:
				inferred = TYPE_FLOAT;
				break;

			case TOKEN_STRING:
				inferred = TYPE_STRING;
				break;

			case TOKEN_TRUE:
			case TOKEN_FALSE:
			case TOKEN_BOOLEAN:
				inferred = TYPE_BOOLEAN;
				break;

			case TOKEN_CHAR:
				inferred = TYPE_CHAR;
				break;
			
				case TOKEN_NULL:
				inferred = TYPE_NULL;
				break;
		}

		annotate_node_type(expr_node, inferred);
		return inferred;
	}

	if (expr_node->type == AST_IDENTIFIER) {
		if (expr_node->token.text && table) {
			Symbol* s = lookup_symbol(table, expr_node->token.text);
			Type t = s ? s->type : TYPE_UNKNOWN;
			annotate_node_type(expr_node, t);
			return t;
		}

		annotate_node_type(expr_node, TYPE_UNKNOWN);
		return TYPE_UNKNOWN;
	}

	if (expr_node->type == AST_BINARY_OP || expr_node->type == AST_BINARY_EXPR ||
		expr_node->type == AST_COMPARISON || expr_node->type == AST_LOGICAL_OP) {
		if (expr_node->child_count < 2) {
			annotate_node_type(expr_node, TYPE_UNKNOWN);
			return TYPE_UNKNOWN;
		}

		Type L = get_expression_type(expr_node->children[0], table);
		Type R = get_expression_type(expr_node->children[1], table);

		if (expr_node->type == AST_COMPARISON || expr_node->type == AST_LOGICAL_OP) {
			annotate_node_type(expr_node, TYPE_BOOLEAN);
			return TYPE_BOOLEAN;
		}

		if (is_numeric_type(L) && is_numeric_type(R)) {
			Type res = (L == TYPE_FLOAT || R == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
			annotate_node_type(expr_node, res);
			return res;
		}

		if (L == R && L != TYPE_UNKNOWN) {
			annotate_node_type(expr_node, L);
			return L;
		}

		annotate_node_type(expr_node, TYPE_UNKNOWN);
		return TYPE_UNKNOWN;
	}

	if (expr_node->type == AST_UNARY_OP || expr_node->type == AST_UNARY_EXPR) {
		if (expr_node->child_count < 1) {
			annotate_node_type(expr_node, TYPE_UNKNOWN);
			return TYPE_UNKNOWN;
		}
		
		if (expr_node->token.type == TOKEN_NOT) {
			annotate_node_type(expr_node, TYPE_BOOLEAN);
			return TYPE_BOOLEAN;
		}
		
		Type inner = get_expression_type(expr_node->children[0], table);
		annotate_node_type(expr_node, inner);
		
		return inner;
	}

	if (expr_node->type == AST_INCREMENT_EXPR) {
		Type t = get_expression_type(expr_node->children[0], table);
		annotate_node_type(expr_node, t);
		return t;
	}

	if (expr_node->type == AST_ARRAY_LITERAL) {
		annotate_node_type(expr_node, TYPE_ARRAY);
		return TYPE_ARRAY;
	}

	if (expr_node->type == AST_FUNCTION_CALL) {
		if (expr_node->child_count < 1) {
			annotate_node_type(expr_node, TYPE_UNKNOWN);
			return TYPE_UNKNOWN;
		}
		ASTNode* func_name_node = expr_node->children[0];
		Symbol* func_symbol = lookup_symbol(table, func_name_node->token.text);
		Type ret_type = func_symbol ? func_symbol->type : TYPE_UNKNOWN;
		annotate_node_type(expr_node, ret_type);
		return ret_type;
	}

	if (expr_node->child_count > 0) {
		Type t = get_expression_type(expr_node->children[0], table);
		annotate_node_type(expr_node, t);
		return t;
	}
 
	annotate_node_type(expr_node, TYPE_UNKNOWN);
	return TYPE_UNKNOWN;
}

bool check_type_compatibility(Type expected, Type actual) {
	if (expected == TYPE_UNKNOWN || actual == TYPE_UNKNOWN) return false;
	if (expected == actual) return true;
	if (is_numeric_type(expected) && is_numeric_type(actual)) return true;
	if (is_numeric_type(expected) && actual == TYPE_CHAR) return true;
	if (actual == TYPE_NULL && (expected == TYPE_STRING || expected == TYPE_ARRAY || expected == TYPE_NULL)) return true;

	return false;
}

bool is_numeric_type(Type type) {
	return type == TYPE_INT || type == TYPE_FLOAT;
}

bool is_boolean_type(Type type) {
	return type == TYPE_BOOLEAN;
}

// 
//  Convert Type enum to string for error messages
// 
const char* type_to_string(Type type) {
	switch (type) {
		case TYPE_INT:
			return "int";
		
		case TYPE_FLOAT:
			return "float";
		
		case TYPE_STRING:
			return "string";
		
		case TYPE_BOOLEAN:
			return "boolean";
		
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
void semantic_error(ASTNode* node, const char* fmt, ...) {
	if (node == NULL) return;
	semantic_error_count++;

	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	
	log_semantic_error(node, "%s", buffer);
}

void semantic_warning(ASTNode* node, const char* fmt, ...) {
	if (node == NULL) return;
	semantic_warning_count++;

	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	
	log_semantic_warning(node, "%s", buffer);
}

void semantic_tip(ASTNode* node, const char* fmt, ...) {
	if (node == NULL) return;
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
void annotate_node_type(ASTNode* node, Type type) {
	if (!node) return;
	node->annotated_type = type;
}

void free_symbol_table(SymbolTable* table) {
	if (!table) return;
	if (table->buckets) {
		for (int index = 0; index < table->bucket_count; index++) {
			Symbol* current_symbol = table->buckets[index];

			while (current_symbol) {
				Symbol* next_symbol = current_symbol->next;
				if (current_symbol->name) free(current_symbol->name);
				free(current_symbol);
				current_symbol = next_symbol;
			}
		}

		free(table->buckets);
		table->buckets = NULL;
	}

	free(table);
}
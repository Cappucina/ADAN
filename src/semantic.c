#include "semantic.h"
#include "util.h"
#include "ast.h"
#include <string.h>
#include "logs.h"

SymbolTable* init_symbol_table() {
	SymbolTable* table = malloc(sizeof(SymbolTable));

	table->bucket_count = 64;
	table->buckets = calloc(table->bucket_count, sizeof(Symbol*));
	table->parent = NULL;

	return table;
}

void enter_scope(SymbolTable* table) {
	SymbolTable* new_table = init_symbol_table();
	new_table->parent = table;
	*table = *new_table;
	free(new_table);
}

void exit_scope(SymbolTable* table) {
	if (table->parent) {
		SymbolTable* parent = table->parent;
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
		*table = *parent;
	}
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
	if (!new_symbol->name) {
		free(new_symbol);
		return false;
	}

	new_symbol->type = type;
	new_symbol->node = node;
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
			break;
		
		case AST_BLOCK:
			analyze_block(statement, table);
			break;
		
		case AST_INCLUDE:
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
		
		case AST_FUNCTION_CALL:
			break;
		
		case AST_IDENTIFIER:
		case AST_LITERAL:
			break;
		
		default:
			break;
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

	if (!add_symbol(table, func_name_node->token.text, return_type, func_node)) {
		semantic_error(func_name_node, SemanticErrorMessages[SEMANTIC_DUPLICATE_SYMBOL], func_name_node->token.text);
		return;
	}

	enter_scope(table);
	if (params_node && params_node->type == AST_PARAMS) {
		for (int i = 0; i < params_node->child_count; i++) {
			ASTNode* param = params_node->children[i];
			Type param_type = get_expression_type(param->children[0], table);

			const char* param_name = param->children[1]->token.text;
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

    enter_scope(table);

    if (assignment_node == NULL) return;

    analyze_statement(assignment_node, table);
    if (get_expression_type(condition_node, table) != TYPE_BOOLEAN) {
        semantic_error(for_node, SemanticErrorMessages[SEMANTIC_TYPE_MISMATCH], for_node->token.text, TYPE_BOOLEAN);
    }

    // do later
}

void analyze_if(ASTNode* if_node, SymbolTable* table) {
    if (!if_node || !table) return;
    if (if_node->type != AST_IF) return;

}

void analyze_while(ASTNode* while_node, SymbolTable* table) {
    if (!while_node || !table) return;
    if (while_node->type != AST_WHILE) return;

}

void analyze_return(ASTNode* return_node, SymbolTable* table) {
    if (!return_node || !table) return;
    if (return_node->type != AST_RETURN) return;

}

void analyze_assignment(ASTNode* assignment_node, SymbolTable* table) {

}

Type analyze_expression(ASTNode* expr_node, SymbolTable* table) {

}

Type analyze_binary_op(ASTNode* binary_node, SymbolTable* table) {

}

Type analyze_unary_op(ASTNode* unary_node, SymbolTable* table) {

}

// 
//  Type System / Helper functions
// 
Type get_expression_type(ASTNode* expr_node, SymbolTable* table) {
	if (!expr_node) return TYPE_UNKNOWN;
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

		if (is_numeric_type(L) && is_numeric_type(R)) {
			Type res = (L == TYPE_FLOAT || R == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
			annotate_node_type(expr_node, res);
			return res;
		}

		if (expr_node->type == AST_COMPARISON || expr_node->type == AST_LOGICAL_OP) {
			annotate_node_type(expr_node, TYPE_BOOLEAN);
			return TYPE_BOOLEAN;
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

	if (expr_node->type == AST_ARRAY_LITERAL) {
		annotate_node_type(expr_node, TYPE_ARRAY);
		return TYPE_ARRAY;
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
//  Error Handling (Errors, Warnings, Tips, etc.)
// 
void semantic_error(ASTNode* node, const char* fmt, ...) {

}

void semantic_warning(ASTNode* node, const char* fmt, ...) {

}

void semantic_tip(ASTNode* node, const char* fmt, ...) {

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
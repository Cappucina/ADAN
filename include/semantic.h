#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

typedef struct Symbol {
	char* name;
	Type type;
	ASTNode* node;
	int usage_count;
	struct Symbol* next;
} Symbol;

typedef struct SymbolTable {
	Symbol** buckets;
	int bucket_count;
	struct SymbolTable* parent;
	int loop_depth;
	Type current_return_type;
} SymbolTable;

SymbolTable* init_symbol_table();

void enter_scope(SymbolTable* table);

void exit_scope(SymbolTable* table);

bool add_symbol(SymbolTable* table, const char* name, Type type, ASTNode* node);

Symbol* lookup_symbol(SymbolTable* table, const char* name);

bool symbol_in_scope(SymbolTable* table, const char* name);

// 
//  AST Traversal / Semantic Checks
// 
void analyze_program(ASTNode* root, SymbolTable* table);

void analyze_file(ASTNode* root, SymbolTable* table);

void analyze_block(ASTNode* block, SymbolTable* table);

void analyze_statement(ASTNode* statement, SymbolTable* table);

void analyze_for(ASTNode* for_node, SymbolTable* table);

void analyze_if(ASTNode* if_node, SymbolTable* table);

void analyze_while(ASTNode* while_node, SymbolTable* table);

void analyze_return(ASTNode* return_node, SymbolTable* table);

void analyze_break(ASTNode* break_node, SymbolTable* table);

void analyze_declaration(ASTNode* declaration_node, SymbolTable* table);

void analyze_assignment(ASTNode* assignment_node, SymbolTable* table);

void analyze_include(ASTNode* include_node, SymbolTable* table);

void analyze_function_call(ASTNode* call_node, SymbolTable* table);

void check_entry_point(SymbolTable* table);

void analyze_array_literal(ASTNode* node, SymbolTable* table);

bool validate_array_element_types(ASTNode* array_node, SymbolTable* table);

void check_unreachable_code(ASTNode* block, SymbolTable* table);

bool has_all_paths_return(ASTNode* block, Type return_type);

void analyze_variable_usage(SymbolTable* table);

void check_type_cast_validity(Type from, Type to, ASTNode* node);

Type analyze_array_access(ASTNode* node, SymbolTable* table);

Type analyze_expression(ASTNode* expr_node, SymbolTable* table);

Type analyze_binary_op(ASTNode* binary_node, SymbolTable* table);

Type analyze_unary_op(ASTNode* unary_node, SymbolTable* table);

// 
//  Type System / Helper functions
// 
Type get_expression_type(ASTNode* expr_node, SymbolTable* table);

bool check_type_compatibility(Type expected, Type actual);

bool is_numeric_type(Type type);

bool is_boolean_type(Type type);

//
//  Error Handling (Errors, Warnings, Tips, etc.)
// 
void semantic_error(ASTNode* node, const char* fmt, ...);

void semantic_warning(ASTNode* node, const char* fmt, ...);

void semantic_tip(ASTNode* node, const char* fmt, ...);

// 
//  AST Annotation and Utilities
// 
void annotate_node_type(ASTNode* node, Type type);

void free_symbol_table(SymbolTable* table);

#endif
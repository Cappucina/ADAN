#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h>

#include "../ast/tree.h"
#include "../../stm.h"

typedef struct SemanticAnalyzer
{
	ASTNode* ast;
	SymbolTableStack* symbol_table_stack;
	int error_count;
	int warning_count;
	bool has_errors;
	const char* current_function_return_type;
} SemanticAnalyzer;

SemanticAnalyzer* semantic_init(ASTNode* ast, SymbolTableStack* symbol_table_stack);

void semantic_free(SemanticAnalyzer* analyzer);

bool semantic_analyze(SemanticAnalyzer* analyzer);

const char* semantic_get_bundle_paths(void);

bool semantic_types_compatible(const char* expected, const char* actual);

void semantic_error(SemanticAnalyzer* analyzer, ASTNode* node, const char* message);

bool is_integer_type(const char* name);

bool is_float_type(const char* name);

bool is_integer_type_promotable(const char* name);

void semantic_warning(SemanticAnalyzer* analyzer, ASTNode* node, const char* message);

#endif
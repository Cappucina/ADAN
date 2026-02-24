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

bool semantic_types_compatible(const char* expected, const char* actual);

void semantic_error(SemanticAnalyzer* analyzer, ASTNode* node, const char* message);

void semantic_warning(SemanticAnalyzer* analyzer, ASTNode* node, const char* message);

#endif
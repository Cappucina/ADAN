#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semantic.h"
#include "validator.h"

SemanticAnalyzer* semantic_init(ASTNode* ast, SymbolTableStack* symbol_table_stack)
{
	SemanticAnalyzer* analyzer = malloc(sizeof(SemanticAnalyzer));
	if (!analyzer)
	{
		fprintf(stderr, "Couldn't allocate memory for SemanticAnalyzer! (Error)\n");
		return NULL;
	}
	analyzer->ast = ast;
	analyzer->symbol_table_stack = symbol_table_stack;
	analyzer->error_count = 0;
	analyzer->warning_count = 0;
	analyzer->has_errors = false;
	analyzer->current_function_return_type = NULL;
	return analyzer;
}

void semantic_free(SemanticAnalyzer* analyzer)
{
	if (!analyzer)
		return;
	validator_cleanup();
	free(analyzer);
}

bool semantic_analyze(SemanticAnalyzer* analyzer)
{
	if (!analyzer || !analyzer->ast)
	{
		return false;
	}

	analyzer->error_count = 0;
	analyzer->warning_count = 0;
	analyzer->has_errors = false;

	validate_node(analyzer, analyzer->ast);
	return !analyzer->has_errors;
}

bool semantic_types_compatible(const char* expected, const char* actual)
{
	if (!expected || !actual)
	{
		return false;
	}
	return strcmp(expected, actual) == 0;
}

void semantic_error(SemanticAnalyzer* analyzer, ASTNode* node, const char* message)
{
	if (node)
	{
		fprintf(stderr, "Semantic Error at line %zu, column %zu: %s (Error)\n", node->line,
		        node->column, message);
	}
	else
	{
		fprintf(stderr, "Semantic Error: %s (Error)\n", message);
	}
	analyzer->error_count++;
	analyzer->has_errors = true;
}

void semantic_warning(SemanticAnalyzer* analyzer, ASTNode* node, const char* message)
{
	if (node)
	{
		fprintf(stderr, "Semantic Warning at line %zu, column %zu: %s (Warning)\n",
		        node->line, node->column, message);
	}
	else
	{
		fprintf(stderr, "Semantic Warning: %s (Warning)\n", message);
	}
	analyzer->warning_count++;
}
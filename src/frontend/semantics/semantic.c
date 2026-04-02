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
	analyzer->loop_depth = 0;
	return analyzer;
}

void semantic_free(SemanticAnalyzer* analyzer)
{
	if (!analyzer)
	{
		return;
	}
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

const char* semantic_get_bundle_paths(void)
{
	return validator_get_bundle_paths();
}

const char* semantic_get_embedded_modules(void)
{
	return validator_get_embedded_modules();
}

const char* semantic_get_native_libraries(void)
{
	return validator_get_native_libraries();
}

const char* semantic_get_native_search_paths(void)
{
	return validator_get_native_search_paths();
}

bool is_integer_type(const char* name)
{
	if (!name)
		return false;
	return strcmp(name, "i8") == 0 || strcmp(name, "u8") == 0 || strcmp(name, "i32") == 0 ||
	       strcmp(name, "u32") == 0 || strcmp(name, "i64") == 0 || strcmp(name, "u64") == 0;
}

bool is_float_type(const char* name)
{
	if (!name)
		return false;
	return strcmp(name, "f32") == 0 || strcmp(name, "f64") == 0;
}

static bool starts_with(const char* text, const char* prefix)
{
	return text && prefix && strncmp(text, prefix, strlen(prefix)) == 0;
}

static bool is_array_type_name(const char* name)
{
	return starts_with(name, "array<") && name[strlen(name) - 1] == '>';
}

static bool is_object_type_name(const char* name)
{
	return starts_with(name, "object{") && name[strlen(name) - 1] == '}';
}

static const char* extract_array_element_type(const char* array_type)
{
	static char slots[8][256];
	static size_t slot = 0;
	if (!is_array_type_name(array_type))
	{
		return NULL;
	}
	slot = (slot + 1) % 8;
	size_t out = 0;
	size_t depth = 0;
	for (size_t i = 6; array_type[i] != '\0'; i++)
	{
		char current = array_type[i];
		if (current == '<')
		{
			depth++;
		}
		else if (current == '>')
		{
			if (depth == 0)
			{
				break;
			}
			depth--;
		}
		if (out + 1 < sizeof(slots[slot]))
		{
			slots[slot][out++] = current;
		}
	}
	slots[slot][out] = '\0';
	return slots[slot];
}

bool semantic_types_compatible(const char* expected, const char* actual)
{
	if (!expected || !actual)
	{
		return false;
	}
	if (strcmp(expected, "any") == 0 || strcmp(actual, "any") == 0)
	{
		return true;
	}
	if (strcmp(expected, actual) == 0)
	{
		return true;
	}
	if (strcmp(expected, "object") == 0 && is_object_type_name(actual))
	{
		return true;
	}
	if (strcmp(actual, "object") == 0 && is_object_type_name(expected))
	{
		return true;
	}
	if (strcmp(expected, "array") == 0 && is_array_type_name(actual))
	{
		return true;
	}
	if (strcmp(actual, "array") == 0 && is_array_type_name(expected))
	{
		return true;
	}
	if (is_array_type_name(expected) && is_array_type_name(actual))
	{
		const char* expected_element = extract_array_element_type(expected);
		const char* actual_element = extract_array_element_type(actual);
		return semantic_types_compatible(expected_element, actual_element);
	}
	if (is_integer_type(expected) && is_integer_type(actual))
	{
		return true;
	}
	if (is_float_type(expected) && is_integer_type(actual))
	{
		return true;
	}
	if (is_float_type(expected) && is_float_type(actual))
	{
		return true;
	}
	if (is_float_type(expected) && is_integer_type(actual))
	{
		return true;
	}
	return false;
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
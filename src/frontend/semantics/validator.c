#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "validator.h"
#include "../../helper.h"
#include "../ast/tree.h"
#include "../parser/parser.h"
#include "../scanner/scanner.h"

// Forward declarations

void validate_program(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_function_declaration(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_variable_declaration(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_import_statement(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_parameter(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_block(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_call_expression(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_return_statement(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_expression_statement(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_identifier(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_string_literal(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_number_literal(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_type_node(SemanticAnalyzer* analyzer, ASTNode* node);

// Primary validator stuff

void validate_node(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!node)
	{
		return;
	}

	switch (node->type)
	{
		case AST_PROGRAM:
			validate_program(analyzer, node);
			break;
		case AST_FUNCTION_DECLARATION:
			validate_function_declaration(analyzer, node);
			break;
		case AST_VARIABLE_DECLARATION:
			validate_variable_declaration(analyzer, node);
			break;
		case AST_IMPORT_STATEMENT:
			validate_import_statement(analyzer, node);
			break;
		case AST_PARAMETER:
			validate_parameter(analyzer, node);
			break;
		case AST_CALL:
			validate_call_expression(analyzer, node);
			break;
		case AST_IDENTIFIER:
			validate_identifier(analyzer, node);
			break;
		case AST_STRING_LITERAL:
			validate_string_literal(analyzer, node);
			break;
		case AST_NUMBER_LITERAL:
			validate_number_literal(analyzer, node);
			break;
		case AST_TYPE:
			validate_type_node(analyzer, node);
			break;
		case AST_BLOCK:
			validate_block(analyzer, node);
			break;
		case AST_RETURN_STATEMENT:
			validate_return_statement(analyzer, node);
			break;
		case AST_EXPRESSION_STATEMENT:
			validate_expression_statement(analyzer, node);
			break;
		default:
			fprintf(stderr, "Unknown AST node type! (Error)\n");
			analyzer->error_count++;
			analyzer->has_errors = true;
	}
}

// Validation helper stuff

static char** imported_paths = NULL;
static size_t imported_count = 0;
static size_t imported_capacity = 0;

typedef struct
{
	char* name;
	char** param_types;
	size_t param_count;
	char* return_type;
} FunctionSignature;

static FunctionSignature* function_signatures = NULL;

static size_t function_signature_count = 0;
static size_t function_signatures_capacity = 0;

void validator_cleanup()
{
	for (size_t i = 0; i < imported_count; i++)
	{
		if (imported_paths[i])
		{
			free(imported_paths[i]);
			imported_paths[i] = NULL;
		}
	}
	free(imported_paths);
	imported_paths = NULL;
	imported_count = 0;
	imported_capacity = 0;

	for (size_t i = 0; i < function_signature_count; i++)
	{
		FunctionSignature* sig = &function_signatures[i];
		if (sig->name)
		{
			free(sig->name);
			sig->name = NULL;
		}
		if (sig->return_type)
		{
			free(sig->return_type);
			sig->return_type = NULL;
		}
		if (sig->param_types)
		{
			for (size_t j = 0; j < sig->param_count; j++)
			{
				if (sig->param_types[j])
				{
					free(sig->param_types[j]);
				}
			}
			free(sig->param_types);
			sig->param_types = NULL;
		}
		sig->param_count = 0;
	}
	free(function_signatures);
	function_signatures = NULL;
	function_signature_count = 0;
	function_signatures_capacity = 0;
}

static FunctionSignature* find_function_signature(const char* name)
{
	if (!name)
	{
		return NULL;
	}

	for (size_t i = 0; i < function_signature_count; i++)
	{
		if (function_signatures[i].name && strcmp(function_signatures[i].name, name) == 0)
		{
			return &function_signatures[i];
		}
	}

	return NULL;
}

static void register_function_signature(ASTNode* decl)
{
	if (!decl || decl->type != AST_FUNCTION_DECLARATION)
	{
		return;
	}

	if (!decl->func_decl.name || !decl->func_decl.return_type ||
	    !decl->func_decl.return_type->type_node.name)
	{
		return;
	}

	if (find_function_signature(decl->func_decl.name) != NULL)
	{
		return;
	}

	if (function_signature_count + 1 > function_signatures_capacity)
	{
		size_t new_cap =
		    function_signatures_capacity == 0 ? 8 : function_signatures_capacity * 2;
		FunctionSignature* resized =
		    realloc(function_signatures, new_cap * sizeof(FunctionSignature));
		if (!resized)
		{
			fprintf(stderr, "Failed to allocate memory for function signatures.\n");
			return;
		}
		function_signatures = resized;
		function_signatures_capacity = new_cap;
	}

	FunctionSignature* signature = &function_signatures[function_signature_count++];
	signature->name = clone_string(decl->func_decl.name, strlen(decl->func_decl.name));
	signature->return_type = clone_string(decl->func_decl.return_type->type_node.name,
	                                      strlen(decl->func_decl.return_type->type_node.name));
	signature->param_count = decl->func_decl.param_count;
	signature->param_types = NULL;

	if (!signature->name || !signature->return_type)
	{
		if (signature->name)
			free(signature->name);
		if (signature->return_type)
			free(signature->return_type);
		signature->name = NULL;
		signature->return_type = NULL;
		function_signature_count--;
		return;
	}

	if (signature->param_count > 0)
	{
		signature->param_types = calloc(signature->param_count, sizeof(char*));
		if (!signature->param_types)
		{
			if (signature->name)
				free(signature->name);
			if (signature->return_type)
				free(signature->return_type);
			signature->name = NULL;
			signature->return_type = NULL;
			function_signature_count--;
			return;
		}

		for (size_t i = 0; i < signature->param_count; i++)
		{
			ASTNode* param = decl->func_decl.params[i];
			if (param && param->param.type && param->param.type->type_node.name)
			{
				signature->param_types[i] =
				    clone_string(param->param.type->type_node.name,
				                 strlen(param->param.type->type_node.name));
				if (!signature->param_types[i])
				{
					// cleanup on failure
					for (size_t j = 0; j < i; j++)
					{
						if (signature->param_types[j])
							free(signature->param_types[j]);
					}
					free(signature->param_types);
					signature->param_types = NULL;
					if (signature->name)
						free(signature->name);
					if (signature->return_type)
						free(signature->return_type);
					signature->name = NULL;
					signature->return_type = NULL;
					function_signature_count--;
					return;
				}
			}
		}
	}
}

static const char* resolve_expression_type(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!node)
	{
		return NULL;
	}

	switch (node->type)
	{
		case AST_STRING_LITERAL:
			return "string";
		case AST_NUMBER_LITERAL:
			return "i32";
		case AST_IDENTIFIER:
			if (!analyzer || !analyzer->symbol_table_stack ||
			    !analyzer->symbol_table_stack->current_scope || !node->identifier.name)
			{
				return NULL;
			}
			{
				SymbolEntry* entry =
				    stm_lookup(analyzer->symbol_table_stack->current_scope,
				               node->identifier.name);
				return entry ? entry->type : NULL;
			}
		case AST_CALL:
			if (node->call.callee)
			{
				FunctionSignature* signature =
				    find_function_signature(node->call.callee);
				return signature ? signature->return_type : NULL;
			}
			return NULL;
		default:
			return NULL;
	}
}

static bool is_known_type_name(const char* name)
{
	if (!name)
		return false;
	static const char* types[] = {"string", "i32", "i64", "u32", "u64", "void"};
	for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++)
	{
		if (strcmp(types[i], name) == 0)
		{
			return true;
		}
	}
	return false;
}

static void declare_symbol(SemanticAnalyzer* analyzer, ASTNode* node, const char* name,
                           const char* type)
{
	if (!analyzer || !analyzer->symbol_table_stack ||
	    !analyzer->symbol_table_stack->current_scope)
	{
		return;
	}

	if (!name || !type)
	{
		semantic_error(analyzer, node, "Missing symbol name or type.");
		return;
	}

	if (stm_lookup_local(analyzer->symbol_table_stack->current_scope, name) != NULL)
	{
		semantic_error(analyzer, node, "Duplicate declaration in current scope.");
		return;
	}

	char line_buffer[32];
	snprintf(line_buffer, sizeof(line_buffer), "%zu", node->line);
	stm_insert(analyzer->symbol_table_stack->current_scope, (char*)name, (char*)type, 0,
	           line_buffer, "0", "0");
}

static bool normalize_import_path(const char* raw, char* output, size_t output_size)
{
	if (!raw || !output || output_size == 0)
	{
		return false;
	}

	size_t len = strlen(raw);
	const char* start = raw;
	if (len >= 2 && raw[0] == '"' && raw[len - 1] == '"')
	{
		start = raw + 1;
		len -= 2;
	}

	if (len + 1 > output_size)
	{
		return false;
	}

	memcpy(output, start, len);
	output[len] = '\0';
	return true;
}

static bool has_imported_path(const char* path)
{
	if (!path)
		return false;

	for (size_t i = 0; i < imported_count; i++)
	{
		if (imported_paths[i] && strcmp(imported_paths[i], path) == 0)
		{
			return true;
		}
	}

	return false;
}

static void mark_imported_path(const char* path)
{
	if (!path)
		return;

	if (imported_count + 1 > imported_capacity)
	{
		size_t new_cap = imported_capacity == 0 ? 8 : imported_capacity * 2;
		char** resized = realloc(imported_paths, new_cap * sizeof(char*));
		if (!resized)
		{
			fprintf(stderr, "Failed to allocate import path cache.\n");
			return;
		}
		imported_paths = resized;
		imported_capacity = new_cap;
	}

	imported_paths[imported_count++] = clone_string(path, strlen(path));
}

static bool build_lib_path(const char* import_path, char* output, size_t output_size)
{
	const char* prefix = "adan/";
	size_t prefix_len = strlen(prefix);
	if (strncmp(import_path, prefix, prefix_len) != 0)
	{
		return false;
	}

	const char* rel = import_path + prefix_len;
	if (!rel || rel[0] == '\0')
	{
		return false;
	}

	const char* last = strrchr(rel, '/');
	const char* file = last ? last + 1 : rel;

	char dir_buffer[256];
	if (last)
	{
		size_t dir_len = (size_t)(last - rel);
		if (dir_len + 1 > sizeof(dir_buffer))
		{
			return false;
		}
		memcpy(dir_buffer, rel, dir_len);
		dir_buffer[dir_len] = '\0';
	}
	else
	{
		if (strlen(rel) + 1 > sizeof(dir_buffer))
		{
			return false;
		}
		strcpy(dir_buffer, rel);
	}

	int written = snprintf(output, output_size, "libs/%s/%s.adn", dir_buffer, file);
	return written > 0 && (size_t)written < output_size;
}

static void declare_imported_symbols(SemanticAnalyzer* analyzer, ASTNode* ast)
{
	if (!analyzer || !ast || ast->type != AST_PROGRAM)
	{
		return;
	}

	for (size_t i = 0; i < ast->program.count; i++)
	{
		ASTNode* decl = ast->program.decls[i];
		if (!decl)
		{
			continue;
		}

		switch (decl->type)
		{
			case AST_FUNCTION_DECLARATION:
				if (decl->func_decl.name && decl->func_decl.return_type &&
				    decl->func_decl.return_type->type_node.name)
				{
					declare_symbol(analyzer, decl, decl->func_decl.name,
					               decl->func_decl.return_type->type_node.name);
					register_function_signature(decl);
				}
				break;
			case AST_VARIABLE_DECLARATION:
				if (decl->var_decl.name && decl->var_decl.type &&
				    decl->var_decl.type->type_node.name)
				{
					declare_symbol(analyzer, decl, decl->var_decl.name,
					               decl->var_decl.type->type_node.name);
				}
				break;
			default:
				break;
		}
	}
}

void validate_program(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	for (size_t i = 0; i < node->program.count; i++)
	{
		validate_node(analyzer, node->program.decls[i]);
	}
}

void validate_function_declaration(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->func_decl.name || !node->func_decl.return_type)
	{
		semantic_error(analyzer, node, "Function declaration missing name or return type.");
		return;
	}

	validate_type_node(analyzer, node->func_decl.return_type);
	declare_symbol(analyzer, node, node->func_decl.name,
	               node->func_decl.return_type->type_node.name);
	register_function_signature(node);

	if (node->func_decl.params)
	{
		for (size_t i = 0; i < node->func_decl.param_count; i++)
		{
			validate_parameter(analyzer, node->func_decl.params[i]);
		}
	}

	if (node->func_decl.body)
	{
		const char* previous = analyzer->current_function_return_type;
		analyzer->current_function_return_type =
		    node->func_decl.return_type->type_node.name;
		validate_block(analyzer, node->func_decl.body);
		analyzer->current_function_return_type = previous;
	}
}

void validate_variable_declaration(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->var_decl.name || !node->var_decl.type)
	{
		semantic_error(analyzer, node, "Variable declaration missing name or type.");
		return;
	}

	validate_type_node(analyzer, node->var_decl.type);
	declare_symbol(analyzer, node, node->var_decl.name, node->var_decl.type->type_node.name);

	if (node->var_decl.initializer)
	{
		validate_node(analyzer, node->var_decl.initializer);
		const char* initializer_type =
		    resolve_expression_type(analyzer, node->var_decl.initializer);
		if (initializer_type && node->var_decl.type->type_node.name &&
		    !semantic_types_compatible(node->var_decl.type->type_node.name,
		                               initializer_type))
		{
			semantic_error(analyzer, node,
			               "Initializer type does not match variable type.");
		}
	}
}

void validate_import_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->import.path || node->import.path[0] == '\0')
	{
		semantic_error(analyzer, node, "Import path is missing.");
		return;
	}

	char normalized[256];
	if (!normalize_import_path(node->import.path, normalized, sizeof(normalized)))
	{
		semantic_error(analyzer, node, "Import path is invalid.");
		return;
	}

	if (strncmp(normalized, "adan/", 5) != 0)
	{
		return;
	}

	if (has_imported_path(normalized))
	{
		return;
	}

	char lib_path[512];
	if (!build_lib_path(normalized, lib_path, sizeof(lib_path)))
	{
		semantic_error(analyzer, node, "Failed to resolve standard library path.");
		return;
	}

	char* source = read_file(lib_path);
	if (!source)
	{
		semantic_error(analyzer, node, "Failed to load standard library source.");
		return;
	}

	Scanner* scanner = scanner_init(source);
	Parser* parser = scanner ? parser_init(scanner) : NULL;
	if (!parser)
	{
		if (scanner)
		{
			scanner_free(scanner);
		}
		free(source);
		semantic_error(analyzer, node, "Failed to initialize parser for standard library.");
		return;
	}

	parser->allow_undefined_symbols = true;
	ASTNode* lib_ast = parser_parse_program(parser);
	parser_free(parser);
	scanner_free(scanner);
	free(source);

	if (!lib_ast)
	{
		semantic_error(analyzer, node, "Failed to parse standard library.");
		return;
	}

	declare_imported_symbols(analyzer, lib_ast);
	ast_free(lib_ast);
	mark_imported_path(normalized);
}

void validate_parameter(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->param.name || !node->param.type)
	{
		semantic_error(analyzer, node, "Parameter missing name or type.");
		return;
	}

	validate_type_node(analyzer, node->param.type);
	declare_symbol(analyzer, node, node->param.name, node->param.type->type_node.name);
}

void validate_block(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (analyzer->symbol_table_stack)
	{
		sts_push_scope(analyzer->symbol_table_stack);
	}

	for (size_t i = 0; i < node->block.count; i++)
	{
		validate_node(analyzer, node->block.statements[i]);
	}

	if (analyzer->symbol_table_stack)
	{
		sts_pop_scope(analyzer->symbol_table_stack);
	}
}

void validate_call_expression(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->call.callee)
	{
		semantic_error(analyzer, node, "Call expression missing callee.");
		return;
	}

	if (!analyzer->symbol_table_stack ||
	    stm_lookup(analyzer->symbol_table_stack->current_scope, node->call.callee) == NULL)
	{
		semantic_error(analyzer, node, "Call to undefined symbol.");
	}

	FunctionSignature* signature = find_function_signature(node->call.callee);
	if (!signature)
	{
		semantic_error(analyzer, node, "Call target is not a known function.");
	}
	else if (node->call.arg_count != signature->param_count)
	{
		semantic_error(analyzer, node, "Call argument count does not match.");
	}

	if (node->call.args)
	{
		for (size_t i = 0; i < node->call.arg_count; i++)
		{
			validate_node(analyzer, node->call.args[i]);
			if (signature && i < signature->param_count)
			{
				const char* arg_type =
				    resolve_expression_type(analyzer, node->call.args[i]);
				const char* param_type = signature->param_types[i];
				if (arg_type && param_type &&
				    !semantic_types_compatible(param_type, arg_type))
				{
					char message[128];
					snprintf(message, sizeof(message),
					         "Argument %zu type does not match parameter type.",
					         i + 1);
					semantic_error(analyzer, node, message);
				}
			}
		}
	}
}

void validate_identifier(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->identifier.name)
	{
		semantic_error(analyzer, node, "Identifier missing name.");
		return;
	}

	if (!analyzer->symbol_table_stack ||
	    stm_lookup(analyzer->symbol_table_stack->current_scope, node->identifier.name) == NULL)
	{
		semantic_error(analyzer, node, "Undefined identifier.");
	}
}

void validate_string_literal(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->string_literal.value)
	{
		semantic_error(analyzer, node, "String literal missing value.");
	}
}

void validate_number_literal(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->number_literal.value)
	{
		semantic_error(analyzer, node, "Number literal missing value.");
	}
}

void validate_type_node(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->type_node.name)
	{
		semantic_error(analyzer, node, "Type node missing name.");
		return;
	}

	if (!is_known_type_name(node->type_node.name))
	{
		semantic_error(analyzer, node, "Unknown type.");
	}
}

void validate_return_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
		return;

	if (node->type != AST_RETURN_STATEMENT)
		return;

	if (!analyzer->current_function_return_type)
	{
		semantic_error(analyzer, node, "Return statement outside of a function.");
		return;
	}

	ASTNode* expr = node->ret.expr;
	const char* func_ret = analyzer->current_function_return_type;
	if (expr)
	{
		validate_node(analyzer, expr);
		const char* expr_type = resolve_expression_type(analyzer, expr);
		if (expr_type == NULL)
		{
			semantic_error(analyzer, node,
			               "Unable to determine return expression type.");
			return;
		}
		if (func_ret && strcmp(func_ret, "void") == 0)
		{
			semantic_error(analyzer, node, "Void function must not return a value.");
			return;
		}
		if (!semantic_types_compatible(func_ret, expr_type))
		{
			semantic_error(
			    analyzer, node,
			    "Return expression type does not match function return type.");
		}
	}
	else
	{
		if (func_ret && strcmp(func_ret, "void") == 0)
		{
			// empty return in void function is allowed
			return;
		}
		semantic_error(analyzer, node, "Empty return in non-void function is not allowed.");
	}
}

void validate_expression_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
		return;

	if (node->type != AST_EXPRESSION_STATEMENT)
		return;

	ASTNode* expr = node->expr_stmt.expr;
	if (expr)
	{
		validate_node(analyzer, expr);
	}
}
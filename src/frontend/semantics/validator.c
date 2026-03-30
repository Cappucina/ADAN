#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "validator.h"
#include "semantic.h"
#include "../../helper.h"
#include "../../embedded_libs.h"
#include "../ast/tree.h"
#include "../parser/parser.h"
#include "../scanner/scanner.h"

void validate_program(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_function_declaration(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_type_declaration(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_if_statement(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_while_statement(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_for_statement(SemanticAnalyzer* analyzer, ASTNode* node);

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

void validate_binary_op(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_assignment(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_cast(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_break_statement(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_continue_statement(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_object_literal(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_array_literal(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_member_access(SemanticAnalyzer* analyzer, ASTNode* node);

void validate_array_access(SemanticAnalyzer* analyzer, ASTNode* node);

static const char* resolve_expression_type(SemanticAnalyzer* analyzer, ASTNode* node);

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
		case AST_TYPE_DECLARATION:
			validate_type_declaration(analyzer, node);
			break;
		case AST_IF_STATEMENT:
			validate_if_statement(analyzer, node);
			break;
		case AST_WHILE_STMT:
			validate_while_statement(analyzer, node);
			break;
		case AST_FOR_STMT:
			validate_for_statement(analyzer, node);
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
		case AST_BOOLEAN_LITERAL:
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
		case AST_BINARY_OP:
			validate_binary_op(analyzer, node);
			break;
		case AST_ASSIGNMENT:
			validate_assignment(analyzer, node);
			break;
		case AST_CAST:
			validate_cast(analyzer, node);
			break;
		case AST_BREAK_STATEMENT:
			validate_break_statement(analyzer, node);
			break;
		case AST_CONTINUE_STATEMENT:
			validate_continue_statement(analyzer, node);
			break;
		case AST_OBJECT_LITERAL:
			validate_object_literal(analyzer, node);
			break;
		case AST_ARRAY_LITERAL:
			validate_array_literal(analyzer, node);
			break;
		case AST_MEMBER_ACCESS:
			validate_member_access(analyzer, node);
			break;
		case AST_ARRAY_ACCESS:
			validate_array_access(analyzer, node);
			break;
		default:
			fprintf(stderr, "Unknown AST node type! (Error)\n");
			analyzer->error_count++;
			analyzer->has_errors = true;
	}
}

static char** imported_paths = NULL;
static size_t imported_count = 0;
static size_t imported_capacity = 0;

static char** bundle_paths = NULL;
static size_t bundle_path_count = 0;
static size_t bundle_path_capacity = 0;
static char* bundle_paths_csv = NULL;

static char** embedded_modules_used = NULL;
static size_t embedded_modules_count = 0;
static size_t embedded_modules_capacity = 0;
static char* embedded_modules_csv = NULL;

typedef struct
{
	char* name;
	char** param_types;
	size_t param_count;
	bool is_variadic;
	char* variadic_type;
	char* return_type;
} FunctionSignature;

typedef struct
{
	char* module_name;
	char* public_name;
	char* internal_name;
	char* type;
	bool is_function;
} ModuleExport;

static FunctionSignature* function_signatures = NULL;

static size_t function_signature_count = 0;
static size_t function_signatures_capacity = 0;

static ModuleExport* module_exports = NULL;

static size_t module_export_count = 0;
static size_t module_export_capacity = 0;

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

	for (size_t i = 0; i < bundle_path_count; i++)
	{
		free(bundle_paths[i]);
	}
	free(bundle_paths);
	bundle_paths = NULL;
	bundle_path_count = 0;
	bundle_path_capacity = 0;
	free(bundle_paths_csv);
	bundle_paths_csv = NULL;

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
		if (sig->variadic_type)
		{
			free(sig->variadic_type);
			sig->variadic_type = NULL;
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

	for (size_t i = 0; i < module_export_count; i++)
	{
		free(module_exports[i].module_name);
		free(module_exports[i].public_name);
		free(module_exports[i].internal_name);
		free(module_exports[i].type);
	}
	free(module_exports);
	module_exports = NULL;
	module_export_count = 0;
	module_export_capacity = 0;

	for (size_t i = 0; i < embedded_modules_count; i++)
	{
		free(embedded_modules_used[i]);
	}
	free(embedded_modules_used);
	embedded_modules_used = NULL;
	embedded_modules_count = 0;
	embedded_modules_capacity = 0;
	free(embedded_modules_csv);
	embedded_modules_csv = NULL;
}

const char* validator_get_embedded_modules()
{
	if (embedded_modules_count == 0)
	{
		return NULL;
	}

	free(embedded_modules_csv);
	embedded_modules_csv = NULL;

	size_t total = 0;
	for (size_t i = 0; i < embedded_modules_count; i++)
	{
		total += strlen(embedded_modules_used[i]) + 1;
	}

	embedded_modules_csv = malloc(total + 1);
	if (!embedded_modules_csv)
	{
		return NULL;
	}

	embedded_modules_csv[0] = '\0';
	for (size_t i = 0; i < embedded_modules_count; i++)
	{
		if (i > 0)
		{
			strcat(embedded_modules_csv, ",");
		}
		strcat(embedded_modules_csv, embedded_modules_used[i]);
	}
	return embedded_modules_csv;
}

const char* validator_get_bundle_paths()
{
	if (bundle_path_count == 0)
	{
		return NULL;
	}

	free(bundle_paths_csv);
	bundle_paths_csv = NULL;

	size_t total = 0;
	for (size_t i = 0; i < bundle_path_count; i++)
	{
		total += strlen(bundle_paths[i]) + 1;
	}

	bundle_paths_csv = malloc(total + 1);
	if (!bundle_paths_csv)
	{
		return NULL;
	}

	bundle_paths_csv[0] = '\0';
	for (size_t i = 0; i < bundle_path_count; i++)
	{
		if (i > 0)
		{
			strcat(bundle_paths_csv, ",");
		}
		strcat(bundle_paths_csv, bundle_paths[i]);
	}

	return bundle_paths_csv;
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
	signature->is_variadic = decl->func_decl.is_variadic;
	signature->variadic_type = NULL;
	signature->param_types = NULL;

	if (!signature->name || !signature->return_type)
	{
		if (signature->name)
		{
			free(signature->name);
		}
		if (signature->return_type)
		{
			free(signature->return_type);
		}
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
			{
				free(signature->name);
			}
			if (signature->return_type)
			{
				free(signature->return_type);
			}
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
					for (size_t j = 0; j < i; j++)
					{
						if (signature->param_types[j])
						{
							free(signature->param_types[j]);
						}
					}
					free(signature->param_types);
					signature->param_types = NULL;
					if (signature->name)
					{
						free(signature->name);
					}
					if (signature->return_type)
					{
						free(signature->return_type);
					}
					signature->name = NULL;
					signature->return_type = NULL;
					function_signature_count--;
					return;
				}
			}
		}
	}
	if (signature->is_variadic && decl->func_decl.variadic_type &&
	    decl->func_decl.variadic_type->type_node.name)
	{
		signature->variadic_type =
		    clone_string(decl->func_decl.variadic_type->type_node.name,
		                 strlen(decl->func_decl.variadic_type->type_node.name));
	}
}

static bool starts_with(const char* text, const char* prefix)
{
	return text && prefix && strncmp(text, prefix, strlen(prefix)) == 0;
}

static bool is_internal_import_name(const char* name)
{
	return starts_with(name, "__ns_");
}

static bool is_module_call_name(const char* name)
{
	return starts_with(name, "__modulecall_");
}

static bool is_module_type_name(const char* name)
{
	return starts_with(name, "module{") && name[strlen(name) - 1] == '}';
}

static bool derive_import_namespace(const char* import_path, char* output, size_t output_size)
{
	if (!import_path || !output || output_size == 0)
	{
		return false;
	}

	const char* last = strrchr(import_path, '/');
	const char* name = last ? last + 1 : import_path;
	if (!name[0] || strlen(name) + 1 > output_size)
	{
		return false;
	}

	strcpy(output, name);
	return true;
}

static bool build_internal_import_name(const char* module_name, const char* public_name,
	                                   char* output, size_t output_size)
{
	if (!module_name || !public_name || !output || output_size == 0)
	{
		return false;
	}

	int written = snprintf(output, output_size, "__ns_%s_%s", module_name, public_name);
	return written > 0 && (size_t)written < output_size;
}

static bool parse_module_call_name(const char* call_name, char* module_name,
	                               size_t module_name_size, char* member_name,
	                               size_t member_name_size)
{
	const char* prefix = "__modulecall_";
	if (!starts_with(call_name, prefix) || !module_name || !member_name)
	{
		return false;
	}

	const char* cursor = call_name + strlen(prefix);
	const char* split = strstr(cursor, "__");
	if (!split)
	{
		return false;
	}

	size_t module_length = (size_t)(split - cursor);
	size_t member_length = strlen(split + 2);
	if (module_length == 0 || member_length == 0 || module_length + 1 > module_name_size ||
	    member_length + 1 > member_name_size)
	{
		return false;
	}

	memcpy(module_name, cursor, module_length);
	module_name[module_length] = '\0';
	strcpy(member_name, split + 2);
	return true;
}

static ModuleExport* find_module_export(const char* module_name, const char* public_name)
{
	if (!module_name || !public_name)
	{
		return NULL;
	}

	for (size_t i = 0; i < module_export_count; i++)
	{
		if (module_exports[i].module_name && module_exports[i].public_name &&
		    strcmp(module_exports[i].module_name, module_name) == 0 &&
		    strcmp(module_exports[i].public_name, public_name) == 0)
		{
			return &module_exports[i];
		}
	}

	return NULL;
}

static ModuleExport* find_module_export_by_internal(const char* internal_name)
{
	if (!internal_name)
	{
		return NULL;
	}

	for (size_t i = 0; i < module_export_count; i++)
	{
		if (module_exports[i].internal_name &&
		    strcmp(module_exports[i].internal_name, internal_name) == 0)
		{
			return &module_exports[i];
		}
	}

	return NULL;
}

static void register_module_export(const char* module_name, const char* public_name,
	                               const char* internal_name, const char* type,
	                               bool is_function)
{
	if (!module_name || !public_name || !internal_name || !type)
	{
		return;
	}

	if (find_module_export(module_name, public_name) != NULL)
	{
		return;
	}

	if (module_export_count + 1 > module_export_capacity)
	{
		size_t next_capacity = module_export_capacity == 0 ? 16 : module_export_capacity * 2;
		ModuleExport* resized = realloc(module_exports, sizeof(ModuleExport) * next_capacity);
		if (!resized)
		{
			return;
		}
		module_exports = resized;
		module_export_capacity = next_capacity;
	}

	ModuleExport* export_entry = &module_exports[module_export_count++];
	export_entry->module_name = clone_string(module_name, strlen(module_name));
	export_entry->public_name = clone_string(public_name, strlen(public_name));
	export_entry->internal_name = clone_string(internal_name, strlen(internal_name));
	export_entry->type = clone_string(type, strlen(type));
	export_entry->is_function = is_function;
}

static const char* cache_type_string(const char* text);

static const char* find_composite_property_type(const char* composite_type, const char* prefix,
	                                            const char* property_name)
{
	if (!composite_type || !prefix || !property_name || !starts_with(composite_type, prefix) ||
	    composite_type[strlen(composite_type) - 1] != '}')
	{
		return NULL;
	}

	const char* cursor = composite_type + strlen(prefix);
	while (*cursor && *cursor != '}')
	{
		char key[128];
		size_t key_length = 0;
		while (*cursor && *cursor != ':' && *cursor != '}')
		{
			if (key_length + 1 < sizeof(key))
			{
				key[key_length++] = *cursor;
			}
			cursor++;
		}
		key[key_length] = '\0';

		if (*cursor != ':')
		{
			break;
		}
		cursor++;

		char value[256];
		size_t value_length = 0;
		size_t brace_depth = 0;
		size_t angle_depth = 0;
		while (*cursor)
		{
			char current = *cursor;
			if (current == '{')
			{
				brace_depth++;
			}
			else if (current == '}')
			{
				if (brace_depth == 0 && angle_depth == 0)
				{
					break;
				}
				brace_depth--;
			}
			else if (current == '<')
			{
				angle_depth++;
			}
			else if (current == '>')
			{
				if (angle_depth > 0)
				{
					angle_depth--;
				}
			}
			else if (current == ',' && brace_depth == 0 && angle_depth == 0)
			{
				break;
			}

			if (value_length + 1 < sizeof(value))
			{
				value[value_length++] = current;
			}
			cursor++;
		}
		value[value_length] = '\0';

		if (strcmp(key, property_name) == 0)
		{
			return cache_type_string(value);
		}

		if (*cursor == ',')
		{
			cursor++;
		}
	}

	return NULL;
}

static const char* member_method_name(const char* callee)
{
	const char* prefix = "__member_";
	return starts_with(callee, prefix) ? callee + strlen(prefix) : NULL;
}

static bool is_array_member_method_name(const char* method_name)
{
	return method_name &&
	       (strcmp(method_name, "length") == 0 || strcmp(method_name, "len") == 0 ||
	        strcmp(method_name, "slice") == 0 || strcmp(method_name, "remove") == 0 ||
	        strcmp(method_name, "insert") == 0);
}

static bool is_array_type_name(const char* name)
{
	return starts_with(name, "array<") && name[strlen(name) - 1] == '>';
}

static bool is_object_type_name(const char* name)
{
	return starts_with(name, "object{") && name[strlen(name) - 1] == '}';
}

static const char* cache_type_string(const char* text)
{
	static char slots[16][256];
	static size_t slot = 0;
	if (!text)
	{
		return NULL;
	}
	slot = (slot + 1) % 16;
	snprintf(slots[slot], sizeof(slots[slot]), "%s", text);
	return slots[slot];
}

static const char* extract_array_element_type(const char* array_type)
{
	if (!is_array_type_name(array_type))
	{
		return NULL;
	}

	char buffer[256];
	size_t depth = 0;
	size_t out = 0;
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
		if (out + 1 < sizeof(buffer))
		{
			buffer[out++] = current;
		}
	}
	buffer[out] = '\0';
	return cache_type_string(buffer);
}

static const char* find_object_property_type(const char* object_type, const char* property_name)
{
	return find_composite_property_type(object_type, "object{", property_name);
}

static bool is_known_type_name(const char* name);

static void mark_bundle_path(const char* path)
{
	if (!path || path[0] == '\0')
	{
		return;
	}

	for (size_t i = 0; i < bundle_path_count; i++)
	{
		if (strcmp(bundle_paths[i], path) == 0)
		{
			return;
		}
	}

	if (bundle_path_count + 1 > bundle_path_capacity)
	{
		size_t next_capacity = bundle_path_capacity == 0 ? 8 : bundle_path_capacity * 2;
		char** resized = realloc(bundle_paths, sizeof(char*) * next_capacity);
		if (!resized)
		{
			return;
		}
		bundle_paths = resized;
		bundle_path_capacity = next_capacity;
	}

	bundle_paths[bundle_path_count++] = clone_string(path, strlen(path));
}

static bool build_lib_dir(const char* import_path, char* output, size_t output_size)
{
	const char* prefix = "adan/";
	if (!starts_with(import_path, prefix))
	{
		return false;
	}

	const char* rel = import_path + strlen(prefix);
	int written = snprintf(output, output_size, "libs/%s", rel);
	return written > 0 && (size_t)written < output_size;
}

static bool bundle_path_exists(const char* path)
{
	struct stat st;
	return path && stat(path, &st) == 0;
}

static void append_imported_program(ASTNode* target_program, ASTNode* imported_program)
{
	if (!target_program || !imported_program || target_program->type != AST_PROGRAM ||
	    imported_program->type != AST_PROGRAM || imported_program->program.count == 0)
	{
		return;
	}

	size_t imported_decl_count = 0;
	for (size_t i = 0; i < imported_program->program.count; i++)
	{
		ASTNode* decl = imported_program->program.decls[i];
		if (decl && decl->type != AST_IMPORT_STATEMENT)
		{
			imported_decl_count++;
		}
	}

	if (imported_decl_count == 0)
	{
		ast_free(imported_program);
		return;
	}

	size_t next_count = target_program->program.count + imported_decl_count;
	ASTNode** resized = realloc(target_program->program.decls, sizeof(ASTNode*) * next_count);
	if (!resized)
	{
		return;
	}

	target_program->program.decls = resized;
	size_t out_index = target_program->program.count;
	for (size_t i = 0; i < imported_program->program.count; i++)
	{
		ASTNode* decl = imported_program->program.decls[i];
		if (!decl || decl->type == AST_IMPORT_STATEMENT)
		{
			continue;
		}
		target_program->program.decls[out_index++] = decl;
		imported_program->program.decls[i] = NULL;
	}
	target_program->program.count = next_count;
	free(imported_program->program.decls);
	imported_program->program.decls = NULL;
	imported_program->program.count = 0;
	ast_free(imported_program);
}

static const char* infer_array_literal_type(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!node || node->type != AST_ARRAY_LITERAL)
	{
		return NULL;
	}

	const char* element_type = NULL;
	for (size_t i = 0; i < node->array_literal.count; i++)
	{
		const char* current =
		    resolve_expression_type(analyzer, node->array_literal.elements[i]);
		if (!current)
		{
			return "array<any>";
		}
		if (!element_type)
		{
			element_type = current;
		}
		else if (!semantic_types_compatible(element_type, current))
		{
			return "array<any>";
		}
	}

	if (!element_type)
	{
		return "array<any>";
	}

	char buffer[256];
	snprintf(buffer, sizeof(buffer), "array<%s>", element_type);
	return cache_type_string(buffer);
}

static const char* resolve_expression_type(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!node)
	{
		return NULL;
	}

	switch (node->type)
	{
		case AST_BOOLEAN_LITERAL:
			return "bool";
		case AST_STRING_LITERAL:
			return "string";
		case AST_NUMBER_LITERAL:
		{
			if (node->number_literal.value && strchr(node->number_literal.value, '.'))
			{
				return "f64";
			}
			return "i32";
		}
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
				const char* member_method = member_method_name(node->call.callee);
				if (member_method)
				{
					const char* receiver_type =
					    node->call.arg_count > 0
					        ? resolve_expression_type(analyzer, node->call.args[0])
					        : NULL;
					const char* element_type = extract_array_element_type(receiver_type);
					if (is_array_type_name(receiver_type) &&
					    is_array_member_method_name(member_method))
					{
						if (strcmp(member_method, "length") == 0 ||
						    strcmp(member_method, "len") == 0)
						{
							return "i32";
						}
						if (strcmp(member_method, "slice") == 0)
						{
							return receiver_type ? receiver_type : "array<any>";
						}
						if (strcmp(member_method, "remove") == 0)
						{
							return element_type ? element_type : "any";
						}
						return "void";
					}

					if (node->call.arg_count > 0 && node->call.args[0] &&
					    node->call.args[0]->type == AST_IDENTIFIER)
					{
						char fallback_name[256];
						const char* module_name = node->call.args[0]->identifier.name;
						ModuleExport* export_entry =
						    find_module_export(module_name, member_method);
						if (export_entry)
						{
							return export_entry->type;
						}
						if (build_internal_import_name(module_name, member_method, fallback_name,
						                             sizeof(fallback_name)) &&
						    analyzer && analyzer->symbol_table_stack &&
						    analyzer->symbol_table_stack->current_scope)
						{
							SymbolEntry* entry =
							    stm_lookup(analyzer->symbol_table_stack->current_scope, fallback_name);
							if (entry)
							{
								return entry->type;
							}
						}
					}

					FunctionSignature* signature = find_function_signature(member_method);
					return signature ? signature->return_type : NULL;
				}

				if (strcmp(node->call.callee, "__string_format") == 0)
				{
					return "string";
				}
				if (starts_with(node->call.callee, "__array_"))
				{
					const char* array_type =
					    node->call.arg_count > 0
					        ? resolve_expression_type(analyzer,
					                                  node->call.args[0])
					        : NULL;
					const char* element_type =
					    extract_array_element_type(array_type);
					if (strcmp(node->call.callee, "__array_length") == 0)
					{
						return "i32";
					}
					if (strcmp(node->call.callee, "__array_slice") == 0)
					{
						return array_type ? array_type : "array<any>";
					}
					if (strcmp(node->call.callee, "__array_pop") == 0 ||
					    strcmp(node->call.callee, "__array_remove") == 0)
					{
						return element_type ? element_type : "any";
					}
					return "void";
				}
				if (strcmp(node->call.callee, "__array_length") == 0)
				{
					return "i32";
				}
				if (starts_with(node->call.callee, "adn_"))
				{
					if (strcmp(node->call.callee, "adn_process_args") == 0 ||
					    strcmp(node->call.callee, "adn_process_env_keys") == 0)
					{
						return "array<string>";
					}
					if (strcmp(node->call.callee, "adn_process_run_capture") == 0 ||
					    strcmp(node->call.callee, "adn_process_run_capture_args") == 0)
					{
						return "object{code:i32,stdout:string,stderr:string,ok:bool}";
					}
					if (strcmp(node->call.callee, "adn_input") == 0 ||
					    strcmp(node->call.callee, "adn_string_format") == 0 ||
					    strcmp(node->call.callee, "adn_read_file") == 0 ||
					    strcmp(node->call.callee, "adn_string_char_at") == 0 ||
					    strcmp(node->call.callee, "adn_process_name") == 0 ||
					    strcmp(node->call.callee, "adn_process_arg") == 0 ||
					    strcmp(node->call.callee, "adn_process_env") == 0 ||
					    strcmp(node->call.callee, "adn_process_cwd") == 0 ||
					    strcmp(node->call.callee, "adn_process_os") == 0 ||
					    strcmp(node->call.callee, "adn_process_arch") == 0 ||
					    strcmp(node->call.callee, "adn_process_executable_path") == 0 ||
					    strcmp(node->call.callee, "adn_process_home_dir") == 0 ||
					    strcmp(node->call.callee, "adn_process_temp_dir") == 0 ||
					    strcmp(node->call.callee, "adn_string_from_code") == 0 ||
					    strstr(node->call.callee, "_to_string") ||
					    strstr(node->call.callee, "_get_string"))
					{
						return "string";
					}
					if (strstr(node->call.callee, "_to_f64") ||
					    strstr(node->call.callee, "_get_f64"))
					{
						return "f64";
					}
					if (strcmp(node->call.callee, "adn_string_code_at") == 0 ||
					    strcmp(node->call.callee, "adn_process_id") == 0 ||
					    strcmp(node->call.callee, "adn_process_parent_id") == 0 ||
					    strcmp(node->call.callee, "adn_process_arg_count") == 0 ||
					    strcmp(node->call.callee, "adn_process_run") == 0 ||
					    strcmp(node->call.callee, "adn_process_run_args") == 0 ||
					    strcmp(node->call.callee, "adn_process_spawn") == 0 ||
					    strcmp(node->call.callee, "adn_process_set_env") == 0 ||
					    strcmp(node->call.callee, "adn_process_has_env") == 0 ||
					    strcmp(node->call.callee, "adn_process_chdir") == 0 ||
					    strcmp(node->call.callee, "adn_process_kill") == 0 ||
					    strcmp(node->call.callee, "adn_process_is_running") == 0 ||
					    strcmp(node->call.callee, "adn_process_is_windows") == 0 ||
					    strcmp(node->call.callee, "adn_process_is_linux") == 0 ||
					    strcmp(node->call.callee, "adn_process_is_macos") == 0 ||
					    strstr(node->call.callee, "_to_i32") ||
					    strstr(node->call.callee, "_get_i64") ||
					    strstr(node->call.callee, "_length"))
					{
						return "i32";
					}
					if (strstr(node->call.callee, "create") ||
					    strstr(node->call.callee, "_get_ptr"))
					{
						return "any";
					}
					return "void";
				}
				FunctionSignature* signature =
				    find_function_signature(node->call.callee);
				if (!signature && starts_with(node->call.callee, "__"))
				{
					for (size_t i = 0; i < module_export_count; i++)
					{
						if (module_exports[i].public_name && module_exports[i].internal_name &&
						    strcmp(module_exports[i].public_name, node->call.callee) == 0)
						{
							signature =
							    find_function_signature(module_exports[i].internal_name);
							if (signature)
							{
								break;
							}
						}
					}
				}
				return signature ? signature->return_type : NULL;
			}
			return NULL;
		case AST_BINARY_OP:
		{
			const char* op = node->binary_op.op;
			if (op)
			{
				if (strcmp(op, "==") == 0 || strcmp(op, "!==") == 0 ||
				    strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
				    strcmp(op, ">") == 0 || strcmp(op, ">=") == 0 ||
				    strcmp(op, "and") == 0 || strcmp(op, "or") == 0 ||
				    strcmp(op, "not") == 0)
				{
					return "bool";
				}

				const char* lt =
				    resolve_expression_type(analyzer, node->binary_op.left);
				const char* rt =
				    resolve_expression_type(analyzer, node->binary_op.right);
				if ((lt && strcmp(lt, "string") == 0) ||
				    (rt && strcmp(rt, "string") == 0))
				{
					return "string";
				}
				if (lt && rt)
				{
					if (strcmp(lt, "f64") == 0 || strcmp(rt, "f64") == 0)
					{
						return "f64";
					}
					if (strcmp(lt, "f32") == 0 || strcmp(rt, "f32") == 0)
					{
						return "f32";
					}
					if (strcmp(lt, "i64") == 0 || strcmp(rt, "i64") == 0 ||
					    strcmp(lt, "u64") == 0 || strcmp(rt, "u64") == 0)
					{
						return "i64";
					}
				}
			}
			return "i32";
		}
		case AST_CAST:
		{
			if (node->cast.target_type && node->cast.target_type->type_node.name)
			{
				return node->cast.target_type->type_node.name;
			}
			return NULL;
		}
		case AST_OBJECT_LITERAL:
			return "object";
		case AST_ARRAY_LITERAL:
			return infer_array_literal_type(analyzer, node);
		case AST_MEMBER_ACCESS:
		{
			if (node->member_access.object && node->member_access.object->type == AST_IDENTIFIER &&
			    node->member_access.property &&
			    node->member_access.property->type == AST_IDENTIFIER)
			{
				ModuleExport* export_entry = find_module_export(
				    node->member_access.object->identifier.name,
				    node->member_access.property->identifier.name);
				if (export_entry)
				{
					return export_entry->type;
				}
				char fallback_name[256];
				if (build_internal_import_name(node->member_access.object->identifier.name,
				                             node->member_access.property->identifier.name,
				                             fallback_name, sizeof(fallback_name)) &&
				    analyzer && analyzer->symbol_table_stack &&
				    analyzer->symbol_table_stack->current_scope)
				{
					SymbolEntry* entry =
					    stm_lookup(analyzer->symbol_table_stack->current_scope, fallback_name);
					if (entry)
					{
						return entry->type;
					}
				}
			}
			const char* object_type =
			    resolve_expression_type(analyzer, node->member_access.object);
			if (object_type && node->member_access.property &&
			    node->member_access.property->type == AST_IDENTIFIER)
			{
				const char* property_type = is_module_type_name(object_type)
				                                ? find_composite_property_type(
				                                      object_type, "module{",
				                                      node->member_access.property->identifier.name)
				                                : find_object_property_type(
				                                      object_type,
				                                      node->member_access.property->identifier.name);
				return property_type ? property_type : "any";
			}
			return "any";
		}
		case AST_ARRAY_ACCESS:
		{
			const char* array_type =
			    resolve_expression_type(analyzer, node->array_access.array);
			const char* element_type = extract_array_element_type(array_type);
			return element_type ? element_type : "any";
		}
		default:
			return NULL;
	}
}

static bool is_known_type_name(const char* name)
{
	if (!name)
	{
		return false;
	}
	if (is_array_type_name(name))
	{
		const char* element_type = extract_array_element_type(name);
		return element_type && is_known_type_name(element_type);
	}
	if (is_object_type_name(name))
	{
		const char* cursor = name + 7;
		while (*cursor && *cursor != '}')
		{
			while (*cursor && *cursor != ':')
			{
				cursor++;
			}
			if (*cursor != ':')
			{
				return false;
			}
			cursor++;

			char value[256];
			size_t value_length = 0;
			size_t brace_depth = 0;
			size_t angle_depth = 0;
			while (*cursor)
			{
				char current = *cursor;
				if (current == '{')
				{
					brace_depth++;
				}
				else if (current == '}')
				{
					if (brace_depth == 0 && angle_depth == 0)
					{
						break;
					}
					brace_depth--;
				}
				else if (current == '<')
				{
					angle_depth++;
				}
				else if (current == '>')
				{
					if (angle_depth > 0)
					{
						angle_depth--;
					}
				}
				else if (current == ',' && brace_depth == 0 && angle_depth == 0)
				{
					break;
				}

				if (value_length + 1 < sizeof(value))
				{
					value[value_length++] = current;
				}
				cursor++;
			}
			value[value_length] = '\0';
			if (!is_known_type_name(value))
			{
				return false;
			}
			if (*cursor == ',')
			{
				cursor++;
			}
		}
		return true;
	}
	if (is_module_type_name(name))
	{
		return true;
	}
	static const char* types[] = {"string", "bool", "i8",  "u8",   "i32",    "i64",   "u32",
	                              "u64",    "f32",  "f64", "void", "object", "array", "any"};
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
                           const char* type, bool is_mutable)
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
		SymbolEntry* existing =
		    stm_lookup_local(analyzer->symbol_table_stack->current_scope, name);
		if (existing && existing->type && strcmp(existing->type, type) == 0)
		{
			return;
		}
		semantic_error(analyzer, node, "Duplicate declaration in current scope.");
		return;
	}

	char line_buffer[32];
	snprintf(line_buffer, sizeof(line_buffer), "%zu", node->line);
	stm_insert(analyzer->symbol_table_stack->current_scope, (char*)name, (char*)type,
	           is_mutable, 0, line_buffer, "0", "0");
}

static bool normalize_import_path(const char* raw, char* output, size_t output_size)
{
	if (!raw || !output || output_size == 0)
	{
		return false;
	}

	size_t len = strlen(raw);
	const char* start = raw;
	if (len >= 2 &&
	    ((raw[0] == '"' && raw[len - 1] == '"') || (raw[0] == '\'' && raw[len - 1] == '\'') ||
	     (raw[0] == '`' && raw[len - 1] == '`')))
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
	{
		return false;
	}

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

typedef struct
{
	char* public_name;
	char* internal_name;
	ASTNodeType decl_type;
} ImportBinding;

static const char* find_import_binding_internal(ImportBinding* bindings, size_t binding_count,
	                                            const char* public_name,
	                                            ASTNodeType expected_type)
{
	if (!bindings || !public_name)
	{
		return NULL;
	}

	for (size_t i = 0; i < binding_count; i++)
	{
		if (bindings[i].public_name && strcmp(bindings[i].public_name, public_name) == 0 &&
		    bindings[i].decl_type == expected_type)
		{
			return bindings[i].internal_name;
		}
	}

	return NULL;
}

static void replace_with_clone(char** target, const char* value)
{
	if (!target || !*target || !value || strcmp(*target, value) == 0)
	{
		return;
	}

	char* cloned = clone_string(value, strlen(value));
	if (!cloned)
	{
		return;
	}

	free(*target);
	*target = cloned;
}

static void rewrite_import_bindings_in_node(ASTNode* node, ImportBinding* bindings,
	                                        size_t binding_count)
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
				rewrite_import_bindings_in_node(node->program.decls[i], bindings, binding_count);
			}
			break;
		case AST_FUNCTION_DECLARATION:
		{
			const char* internal_name = find_import_binding_internal(
			    bindings, binding_count, node->func_decl.name, AST_FUNCTION_DECLARATION);
			if (internal_name)
			{
				replace_with_clone(&node->func_decl.name, internal_name);
			}
			for (size_t i = 0; i < node->func_decl.param_count; i++)
			{
				rewrite_import_bindings_in_node(node->func_decl.params[i], bindings, binding_count);
			}
			rewrite_import_bindings_in_node(node->func_decl.variadic_type, bindings, binding_count);
			rewrite_import_bindings_in_node(node->func_decl.return_type, bindings, binding_count);
			rewrite_import_bindings_in_node(node->func_decl.body, bindings, binding_count);
			break;
		}
		case AST_VARIABLE_DECLARATION:
		{
			const char* internal_name = find_import_binding_internal(
			    bindings, binding_count, node->var_decl.name, AST_VARIABLE_DECLARATION);
			if (internal_name)
			{
				replace_with_clone(&node->var_decl.name, internal_name);
			}
			rewrite_import_bindings_in_node(node->var_decl.type, bindings, binding_count);
			rewrite_import_bindings_in_node(node->var_decl.initializer, bindings, binding_count);
			break;
		}
		case AST_TYPE_DECLARATION:
		{
			const char* internal_name = find_import_binding_internal(
			    bindings, binding_count, node->type_decl.name, AST_TYPE_DECLARATION);
			if (internal_name)
			{
				replace_with_clone(&node->type_decl.name, internal_name);
			}
			rewrite_import_bindings_in_node(node->type_decl.value_type, bindings, binding_count);
			break;
		}
		case AST_PARAMETER:
			rewrite_import_bindings_in_node(node->param.type, bindings, binding_count);
			break;
		case AST_BLOCK:
			for (size_t i = 0; i < node->block.count; i++)
			{
				rewrite_import_bindings_in_node(node->block.statements[i], bindings, binding_count);
			}
			break;
		case AST_CALL:
		{
			if (node->call.callee && !starts_with(node->call.callee, "adn_") &&
			    !starts_with(node->call.callee, "__array_") &&
			    !starts_with(node->call.callee, "__member_") &&
			    !starts_with(node->call.callee, "__modulecall_"))
			{
				const char* internal_name = find_import_binding_internal(
				    bindings, binding_count, node->call.callee, AST_FUNCTION_DECLARATION);
				if (internal_name)
				{
					replace_with_clone(&node->call.callee, internal_name);
				}
			}
			for (size_t i = 0; i < node->call.arg_count; i++)
			{
				rewrite_import_bindings_in_node(node->call.args[i], bindings, binding_count);
			}
			break;
		}
		case AST_IDENTIFIER:
		{
			const char* internal_name = find_import_binding_internal(
			    bindings, binding_count, node->identifier.name, AST_VARIABLE_DECLARATION);
			if (!internal_name)
			{
				internal_name = find_import_binding_internal(bindings, binding_count,
				                                           node->identifier.name,
				                                           AST_FUNCTION_DECLARATION);
			}
			if (internal_name)
			{
				replace_with_clone(&node->identifier.name, internal_name);
			}
			break;
		}
		case AST_RETURN_STATEMENT:
			rewrite_import_bindings_in_node(node->ret.expr, bindings, binding_count);
			break;
		case AST_EXPRESSION_STATEMENT:
			rewrite_import_bindings_in_node(node->expr_stmt.expr, bindings, binding_count);
			break;
		case AST_BINARY_OP:
			rewrite_import_bindings_in_node(node->binary_op.left, bindings, binding_count);
			rewrite_import_bindings_in_node(node->binary_op.right, bindings, binding_count);
			break;
		case AST_ASSIGNMENT:
		{
			const char* internal_name = find_import_binding_internal(
			    bindings, binding_count, node->assignment.name, AST_VARIABLE_DECLARATION);
			if (internal_name)
			{
				replace_with_clone(&node->assignment.name, internal_name);
			}
			rewrite_import_bindings_in_node(node->assignment.value, bindings, binding_count);
			break;
		}
		case AST_CAST:
			rewrite_import_bindings_in_node(node->cast.target_type, bindings, binding_count);
			rewrite_import_bindings_in_node(node->cast.expr, bindings, binding_count);
			break;
		case AST_WHILE_STMT:
			rewrite_import_bindings_in_node(node->while_stmt.condition, bindings, binding_count);
			rewrite_import_bindings_in_node(node->while_stmt.body, bindings, binding_count);
			break;
		case AST_FOR_STMT:
			rewrite_import_bindings_in_node(node->for_stmt.var_decl, bindings, binding_count);
			rewrite_import_bindings_in_node(node->for_stmt.condition, bindings, binding_count);
			rewrite_import_bindings_in_node(node->for_stmt.increment, bindings, binding_count);
			rewrite_import_bindings_in_node(node->for_stmt.body, bindings, binding_count);
			break;
		case AST_IF_STATEMENT:
			rewrite_import_bindings_in_node(node->if_stmt.condition, bindings, binding_count);
			rewrite_import_bindings_in_node(node->if_stmt.then_branch, bindings, binding_count);
			rewrite_import_bindings_in_node(node->if_stmt.else_branch, bindings, binding_count);
			break;
		case AST_OBJECT_LITERAL:
			for (size_t i = 0; i < node->object_literal.count; i++)
			{
				rewrite_import_bindings_in_node(node->object_literal.properties[i].value, bindings,
				                             binding_count);
			}
			break;
		case AST_ARRAY_LITERAL:
			for (size_t i = 0; i < node->array_literal.count; i++)
			{
				rewrite_import_bindings_in_node(node->array_literal.elements[i], bindings,
				                             binding_count);
			}
			break;
		case AST_MEMBER_ACCESS:
			rewrite_import_bindings_in_node(node->member_access.object, bindings, binding_count);
			break;
		case AST_ARRAY_ACCESS:
			rewrite_import_bindings_in_node(node->array_access.array, bindings, binding_count);
			rewrite_import_bindings_in_node(node->array_access.index, bindings, binding_count);
			break;
		case AST_TYPE:
		{
			const char* internal_name = find_import_binding_internal(bindings, binding_count,
			                                                       node->type_node.name,
			                                                       AST_TYPE_DECLARATION);
			if (internal_name)
			{
				replace_with_clone(&node->type_node.name, internal_name);
			}
			break;
		}
		default:
			break;
	}
}

static void collect_module_exports(ASTNode* ast, const char* module_name,
	                               ImportBinding** out_bindings, size_t* out_binding_count)
{
	if (!ast || ast->type != AST_PROGRAM || !module_name || !out_bindings || !out_binding_count)
	{
		return;
	}

	ImportBinding* bindings = NULL;
	size_t binding_count = 0;
	size_t binding_capacity = 0;

	for (size_t i = 0; i < ast->program.count; i++)
	{
		ASTNode* decl = ast->program.decls[i];
		if (!decl)
		{
			continue;
		}

		if (decl->type == AST_IMPORT_STATEMENT && decl->import.path)
		{
			char normalized[512];
			char imported_namespace[128];
			if (normalize_import_path(decl->import.path, normalized, sizeof(normalized)) &&
			    derive_import_namespace(normalized, imported_namespace,
			                         sizeof(imported_namespace)))
			{
				for (size_t j = 0; j < module_export_count; j++)
				{
					ModuleExport* export_entry = &module_exports[j];
					if (!export_entry->module_name || !export_entry->public_name ||
					    !export_entry->internal_name || !export_entry->type)
					{
						continue;
					}

					if (strcmp(export_entry->module_name, imported_namespace) == 0)
					{
						register_module_export(module_name, export_entry->public_name,
						                       export_entry->internal_name,
						                       export_entry->type,
						                       export_entry->is_function);
					}
				}
			}
			continue;
		}

		if (decl->type == AST_FUNCTION_DECLARATION && decl->func_decl.name)
		{
			if (is_internal_import_name(decl->func_decl.name))
			{
				ModuleExport* existing = find_module_export_by_internal(decl->func_decl.name);
				if (existing)
				{
					register_module_export(module_name, existing->public_name,
					                       existing->internal_name, existing->type, true);
				}
				continue;
			}

			char internal_name[256];
			if (!build_internal_import_name(module_name, decl->func_decl.name, internal_name,
			                               sizeof(internal_name)))
			{
				continue;
			}

			register_module_export(module_name, decl->func_decl.name, internal_name,
			                       decl->func_decl.return_type &&
			                               decl->func_decl.return_type->type_node.name
			                           ? decl->func_decl.return_type->type_node.name
			                           : "any",
			                       true);

			if (binding_count + 1 > binding_capacity)
			{
				size_t next_capacity = binding_capacity == 0 ? 8 : binding_capacity * 2;
				ImportBinding* resized = realloc(bindings, sizeof(ImportBinding) * next_capacity);
				if (!resized)
				{
					continue;
				}
				bindings = resized;
				binding_capacity = next_capacity;
			}

			bindings[binding_count].public_name =
			    clone_string(decl->func_decl.name, strlen(decl->func_decl.name));
			bindings[binding_count].internal_name =
			    clone_string(internal_name, strlen(internal_name));
			bindings[binding_count].decl_type = AST_FUNCTION_DECLARATION;
			binding_count++;
		}
		else if (decl->type == AST_VARIABLE_DECLARATION && decl->var_decl.name)
		{
			if (is_internal_import_name(decl->var_decl.name))
			{
				ModuleExport* existing = find_module_export_by_internal(decl->var_decl.name);
				if (existing)
				{
					register_module_export(module_name, existing->public_name,
					                       existing->internal_name, existing->type, false);
				}
				continue;
			}

			char internal_name[256];
			if (!build_internal_import_name(module_name, decl->var_decl.name, internal_name,
			                               sizeof(internal_name)))
			{
				continue;
			}

			register_module_export(module_name, decl->var_decl.name, internal_name,
			                       decl->var_decl.type && decl->var_decl.type->type_node.name
			                           ? decl->var_decl.type->type_node.name
			                           : "any",
			                       false);

			if (binding_count + 1 > binding_capacity)
			{
				size_t next_capacity = binding_capacity == 0 ? 8 : binding_capacity * 2;
				ImportBinding* resized = realloc(bindings, sizeof(ImportBinding) * next_capacity);
				if (!resized)
				{
					continue;
				}
				bindings = resized;
				binding_capacity = next_capacity;
			}

			bindings[binding_count].public_name =
			    clone_string(decl->var_decl.name, strlen(decl->var_decl.name));
			bindings[binding_count].internal_name =
			    clone_string(internal_name, strlen(internal_name));
			bindings[binding_count].decl_type = AST_VARIABLE_DECLARATION;
			binding_count++;
		}
		else if (decl->type == AST_TYPE_DECLARATION && decl->type_decl.name)
		{
			if (is_internal_import_name(decl->type_decl.name))
			{
				continue;
			}

			char internal_name[256];
			if (!build_internal_import_name(module_name, decl->type_decl.name, internal_name,
			                               sizeof(internal_name)))
			{
				continue;
			}

			if (binding_count + 1 > binding_capacity)
			{
				size_t next_capacity = binding_capacity == 0 ? 8 : binding_capacity * 2;
				ImportBinding* resized = realloc(bindings, sizeof(ImportBinding) * next_capacity);
				if (!resized)
				{
					continue;
				}
				bindings = resized;
				binding_capacity = next_capacity;
			}

			bindings[binding_count].public_name =
			    clone_string(decl->type_decl.name, strlen(decl->type_decl.name));
			bindings[binding_count].internal_name =
			    clone_string(internal_name, strlen(internal_name));
			bindings[binding_count].decl_type = AST_TYPE_DECLARATION;
			binding_count++;
		}
	}

	*out_bindings = bindings;
	*out_binding_count = binding_count;
}

static void free_import_bindings(ImportBinding* bindings, size_t binding_count)
{
	if (!bindings)
	{
		return;
	}

	for (size_t i = 0; i < binding_count; i++)
	{
		free(bindings[i].public_name);
		free(bindings[i].internal_name);
	}
	free(bindings);
}

static const char* module_type_for_name(const char* module_name)
{
	static char slots[8][2048];
	static size_t slot = 0;
	if (!module_name)
	{
		return NULL;
	}

	slot = (slot + 1) % 8;
	char* buffer = slots[slot];
	size_t written = 0;
	written += snprintf(buffer + written, sizeof(slots[slot]) - written, "module{");
	bool first = true;
	for (size_t i = 0; i < module_export_count && written + 2 < sizeof(slots[slot]); i++)
	{
		ModuleExport* export_entry = &module_exports[i];
		if (!export_entry->module_name || strcmp(export_entry->module_name, module_name) != 0)
		{
			continue;
		}
		written += snprintf(buffer + written, sizeof(slots[slot]) - written, "%s%s:%s",
		                    first ? "" : ",", export_entry->public_name,
		                    export_entry->is_function ? "any" : export_entry->type);
		first = false;
	}
	snprintf(buffer + written, sizeof(slots[slot]) - written, "}");
	return buffer;
}

static void redeclare_known_module_import(SemanticAnalyzer* analyzer, ASTNode* node,
	                                      const char* module_name)
{
	if (!analyzer || !node || !module_name)
	{
		return;
	}

	declare_symbol(analyzer, node, module_name, module_type_for_name(module_name), false);

	for (size_t i = 0; i < module_export_count; i++)
	{
		ModuleExport* export_entry = &module_exports[i];
		if (!export_entry->module_name || strcmp(export_entry->module_name, module_name) != 0 ||
		    !export_entry->internal_name || !export_entry->type)
		{
			continue;
		}

		declare_symbol(analyzer, node, export_entry->internal_name, export_entry->type, false);
		if (export_entry->public_name && starts_with(export_entry->public_name, "__"))
		{
			declare_symbol(analyzer, node, export_entry->public_name, export_entry->type, false);
		}
	}
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
					               decl->func_decl.return_type->type_node.name,
					               false);
					register_function_signature(decl);
					ModuleExport* export_entry = find_module_export_by_internal(decl->func_decl.name);
					if (export_entry && export_entry->public_name &&
					    starts_with(export_entry->public_name, "__"))
					{
						declare_symbol(analyzer, decl, export_entry->public_name,
						               decl->func_decl.return_type->type_node.name,
						               false);
					}
				}
				break;
			case AST_VARIABLE_DECLARATION:
				if (decl->var_decl.name && decl->var_decl.type &&
				    decl->var_decl.type->type_node.name)
				{
					declare_symbol(analyzer, decl, decl->var_decl.name,
					               decl->var_decl.type->type_node.name,
					               decl->var_decl.is_mutable);
					ModuleExport* export_entry = find_module_export_by_internal(decl->var_decl.name);
					if (export_entry && export_entry->public_name &&
					    starts_with(export_entry->public_name, "__"))
					{
						declare_symbol(analyzer, decl, export_entry->public_name,
						               decl->var_decl.type->type_node.name,
						               decl->var_decl.is_mutable);
					}
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

	size_t initial_count = node->program.count;
	for (size_t i = 0; i < initial_count; i++)
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
	               node->func_decl.return_type->type_node.name, false);
	register_function_signature(node);
	sts_push_scope(analyzer->symbol_table_stack);

	if (node->func_decl.params)
	{
		for (size_t i = 0; i < node->func_decl.param_count; i++)
		{
			validate_parameter(analyzer, node->func_decl.params[i]);
		}
	}
	if (node->func_decl.is_variadic && node->func_decl.variadic_name &&
	    node->func_decl.variadic_type && node->func_decl.variadic_type->type_node.name)
	{
		char array_type[256];
		validate_type_node(analyzer, node->func_decl.variadic_type);
		snprintf(array_type, sizeof(array_type), "array<%s>",
		         node->func_decl.variadic_type->type_node.name);
		declare_symbol(analyzer, node, node->func_decl.variadic_name, array_type, false);
	}

	if (node->func_decl.body)
	{
		const char* previous = analyzer->current_function_return_type;
		analyzer->current_function_return_type =
		    node->func_decl.return_type->type_node.name;
		validate_block(analyzer, node->func_decl.body);
		analyzer->current_function_return_type = previous;
	}
	sts_pop_scope(analyzer->symbol_table_stack);
}

void validate_type_declaration(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node || node->type != AST_TYPE_DECLARATION)
	{
		return;
	}

	if (!node->type_decl.name || !node->type_decl.value_type)
	{
		semantic_error(analyzer, node, "Type declaration missing name or value type.");
		return;
	}

	validate_type_node(analyzer, node->type_decl.value_type);
}

void validate_if_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->if_stmt.condition || !node->if_stmt.then_branch)
	{
		semantic_error(analyzer, node, "If statement missing condition or then branch.");
		return;
	}

	validate_node(analyzer, node->if_stmt.condition);
	validate_block(analyzer, node->if_stmt.then_branch);

	if (node->if_stmt.else_branch)
	{
		validate_node(analyzer, node->if_stmt.else_branch);
	}
}

void validate_while_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->while_stmt.condition || !node->while_stmt.body)
	{
		semantic_error(analyzer, node, "While statement missing condition or body.");
		return;
	}

	validate_node(analyzer, node->while_stmt.condition);
	analyzer->loop_depth++;
	validate_node(analyzer, node->while_stmt.body);
	analyzer->loop_depth--;
}

void validate_for_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (!node->for_stmt.var_decl || !node->for_stmt.condition || !node->for_stmt.increment ||
	    !node->for_stmt.body)
	{
		semantic_error(
		    analyzer, node,
		    "For statement missing variable declaration, condition, increment, or body.");
		return;
	}

	sts_push_scope(analyzer->symbol_table_stack);

	validate_variable_declaration(analyzer, node->for_stmt.var_decl);
	validate_node(analyzer, node->for_stmt.condition);
	validate_node(analyzer, node->for_stmt.increment);

	analyzer->loop_depth++;
	validate_node(analyzer, node->for_stmt.body);
	analyzer->loop_depth--;

	sts_pop_scope(analyzer->symbol_table_stack);
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
	declare_symbol(analyzer, node, node->var_decl.name, node->var_decl.type->type_node.name,
	               node->var_decl.is_mutable);

	if (node->var_decl.initializer)
	{
		validate_node(analyzer, node->var_decl.initializer);
		const char* initializer_type =
		    resolve_expression_type(analyzer, node->var_decl.initializer);
		if (initializer_type && node->var_decl.type->type_node.name)
		{
			const char* target = node->var_decl.type->type_node.name;
			if (node->var_decl.initializer->type == AST_OBJECT_LITERAL &&
			    is_object_type_name(target))
			{
				for (size_t i = 0;
				     i < node->var_decl.initializer->object_literal.count; i++)
				{
					ASTObjectProperty property =
					    node->var_decl.initializer->object_literal
					        .properties[i];
					const char* property_type =
					    find_object_property_type(target, property.key);
					const char* value_type =
					    resolve_expression_type(analyzer, property.value);
					if (!property_type)
					{
						semantic_error(
						    analyzer, property.value,
						    "Object literal contains an unknown property.");
					}
					else if (value_type && !semantic_types_compatible(
					                           property_type, value_type))
					{
						semantic_error(analyzer, property.value,
						               "Object property type does not "
						               "match declared type.");
					}
				}
			}
			else if (node->var_decl.initializer->type == AST_ARRAY_LITERAL &&
			         is_array_type_name(target))
			{
				const char* element_type = extract_array_element_type(target);
				for (size_t i = 0;
				     i < node->var_decl.initializer->array_literal.count; i++)
				{
					const char* value_type = resolve_expression_type(
					    analyzer,
					    node->var_decl.initializer->array_literal.elements[i]);
					if (element_type && value_type &&
					    !semantic_types_compatible(element_type, value_type))
					{
						semantic_error(analyzer,
						               node->var_decl.initializer
						                   ->array_literal.elements[i],
						               "Array element type does not match "
						               "declared array type.");
					}
				}
			}
			else if (strcmp(target, initializer_type) != 0 &&
			         semantic_types_compatible(target, initializer_type))
			{
				node->var_decl.initializer = ast_create_cast(
				    ast_create_type(target, node->line, node->column),
				    node->var_decl.initializer, node->line, node->column);
			}
			else if (!semantic_types_compatible(target, initializer_type))
			{
				semantic_error(analyzer, node,
				               "Initializer type does not match variable type.");
			}
		}
	}
}

void validate_break_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node || node->type != AST_BREAK_STATEMENT)
	{
		return;
	}

	if (analyzer->loop_depth <= 0)
	{
		semantic_error(analyzer, node, "Break statement must appear inside a loop.");
	}
}

void validate_continue_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node || node->type != AST_CONTINUE_STATEMENT)
	{
		return;
	}

	if (analyzer->loop_depth <= 0)
	{
		semantic_error(analyzer, node, "Continue statement must appear inside a loop.");
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
		char namespace_name[128];
		if (derive_import_namespace(normalized, namespace_name, sizeof(namespace_name)))
		{
			redeclare_known_module_import(analyzer, node, namespace_name);
		}
		return;
	}

	char lib_path[512];
	if (!build_lib_path(normalized, lib_path, sizeof(lib_path)))
	{
		semantic_error(analyzer, node, "Failed to resolve standard library path.");
		return;
	}
	char* source = NULL;
	const char* embedded = embedded_lib_get_adn_source(normalized);
	if (embedded)
	{
		source = strdup(embedded);
		if (embedded_modules_count + 1 > embedded_modules_capacity)
		{
			size_t nc =
			    embedded_modules_capacity == 0 ? 8 : embedded_modules_capacity * 2;
			char** r = realloc(embedded_modules_used, nc * sizeof(char*));
			if (r)
			{
				embedded_modules_used = r;
				embedded_modules_capacity = nc;
			}
		}
		if (embedded_modules_count < embedded_modules_capacity)
		{
			embedded_modules_used[embedded_modules_count++] = strdup(normalized);
		}
	}
	else
	{
		source = read_file(lib_path);
	}
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

	char namespace_name[128];
	if (!derive_import_namespace(normalized, namespace_name, sizeof(namespace_name)))
	{
		semantic_error(analyzer, node, "Failed to determine import namespace.");
		ast_free(lib_ast);
		return;
	}

	char bundle_path[512];
	if (build_lib_dir(normalized, bundle_path, sizeof(bundle_path)) &&
	    bundle_path_exists(bundle_path))
	{
		mark_bundle_path(bundle_path);
	}
	mark_imported_path(normalized);
	ASTNode* previous_ast = analyzer->ast;
	sts_push_scope(analyzer->symbol_table_stack);
	declare_imported_symbols(analyzer, lib_ast);
	analyzer->ast = lib_ast;
	validate_program(analyzer, lib_ast);
	analyzer->ast = previous_ast;
	sts_pop_scope(analyzer->symbol_table_stack);

	ImportBinding* bindings = NULL;
	size_t binding_count = 0;
	collect_module_exports(lib_ast, namespace_name, &bindings, &binding_count);
	rewrite_import_bindings_in_node(lib_ast, bindings, binding_count);
	free_import_bindings(bindings, binding_count);

	declare_symbol(analyzer, node, namespace_name, module_type_for_name(namespace_name), false);
	declare_imported_symbols(analyzer, lib_ast);
	append_imported_program(previous_ast, lib_ast);
}

void validate_object_literal(SemanticAnalyzer* analyzer, ASTNode* node)
{
	for (size_t i = 0; i < node->object_literal.count; i++)
	{
		validate_node(analyzer, node->object_literal.properties[i].value);
	}
}

void validate_array_literal(SemanticAnalyzer* analyzer, ASTNode* node)
{
	for (size_t i = 0; i < node->array_literal.count; i++)
	{
		validate_node(analyzer, node->array_literal.elements[i]);
	}
}

void validate_member_access(SemanticAnalyzer* analyzer, ASTNode* node)
{
	validate_node(analyzer, node->member_access.object);
	if (node->member_access.object && node->member_access.object->type == AST_IDENTIFIER &&
	    node->member_access.property && node->member_access.property->type == AST_IDENTIFIER)
	{
		ModuleExport* export_entry = find_module_export(
		    node->member_access.object->identifier.name,
		    node->member_access.property->identifier.name);
		if (export_entry)
		{
			if (!export_entry->is_function)
			{
				char* internal_name = clone_string(export_entry->internal_name,
				                                 strlen(export_entry->internal_name));
				if (internal_name)
				{
					ast_free(node->member_access.object);
					ast_free(node->member_access.property);
					node->type = AST_IDENTIFIER;
					node->identifier.name = internal_name;
				}
			}
			return;
		}

		char fallback_name[256];
		if (build_internal_import_name(node->member_access.object->identifier.name,
		                             node->member_access.property->identifier.name,
		                             fallback_name, sizeof(fallback_name)) &&
		    analyzer && analyzer->symbol_table_stack &&
		    analyzer->symbol_table_stack->current_scope)
		{
			SymbolEntry* entry =
			    stm_lookup(analyzer->symbol_table_stack->current_scope, fallback_name);
			if (entry)
			{
				char* internal_name = clone_string(fallback_name, strlen(fallback_name));
				if (internal_name)
				{
					ast_free(node->member_access.object);
					ast_free(node->member_access.property);
					node->type = AST_IDENTIFIER;
					node->identifier.name = internal_name;
				}
				return;
			}
		}
	}
	if (node->member_access.property && node->member_access.property->type == AST_IDENTIFIER)
	{
		const char* object_type =
		    resolve_expression_type(analyzer, node->member_access.object);
		if (object_type && is_object_type_name(object_type) &&
		    !find_object_property_type(object_type,
		                               node->member_access.property->identifier.name))
		{
			semantic_error(analyzer, node, "Unknown object property.");
		}
	}
}

void validate_array_access(SemanticAnalyzer* analyzer, ASTNode* node)
{
	validate_node(analyzer, node->array_access.array);
	validate_node(analyzer, node->array_access.index);
	const char* array_type = resolve_expression_type(analyzer, node->array_access.array);
	const char* index_type = resolve_expression_type(analyzer, node->array_access.index);
	if (array_type && !is_array_type_name(array_type) && strcmp(array_type, "array") != 0)
	{
		semantic_error(analyzer, node, "Indexed expression is not an array.");
	}
	if (index_type && !is_integer_type(index_type))
	{
		semantic_error(analyzer, node, "Array index must be an integer.");
	}
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
	declare_symbol(analyzer, node, node->param.name, node->param.type->type_node.name, false);
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

	if (is_module_call_name(node->call.callee))
	{
		char module_name[128];
		char member_name[128];
		if (!parse_module_call_name(node->call.callee, module_name, sizeof(module_name),
		                            member_name, sizeof(member_name)))
		{
			semantic_error(analyzer, node, "Invalid namespaced call.");
			return;
		}

		ModuleExport* export_entry = find_module_export(module_name, member_name);
		if (!export_entry || !export_entry->is_function)
		{
			char fallback_name[256];
			if (!build_internal_import_name(module_name, member_name, fallback_name,
			                               sizeof(fallback_name)) ||
			    !analyzer->symbol_table_stack ||
			    !stm_lookup(analyzer->symbol_table_stack->current_scope, fallback_name))
			{
				semantic_error(analyzer, node, "Unknown namespaced function.");
				return;
			}
			replace_with_clone(&node->call.callee, fallback_name);
		}
		else
		{
			replace_with_clone(&node->call.callee, export_entry->internal_name);
		}
	}

	const char* member_method = member_method_name(node->call.callee);
	if (member_method && node->call.arg_count > 0 && node->call.args[0] &&
	    node->call.args[0]->type == AST_IDENTIFIER)
	{
		const char* module_name = node->call.args[0]->identifier.name;
		char fallback_name[256];
		ModuleExport* export_entry = find_module_export(module_name, member_method);
		const char* resolved_name = export_entry ? export_entry->internal_name : NULL;
		if (!resolved_name && build_internal_import_name(module_name, member_method, fallback_name,
		                                               sizeof(fallback_name)) &&
		    analyzer->symbol_table_stack &&
		    stm_lookup(analyzer->symbol_table_stack->current_scope, fallback_name))
		{
			resolved_name = fallback_name;
		}
		if (resolved_name)
		{
			replace_with_clone(&node->call.callee, resolved_name);
			ast_free(node->call.args[0]);
			for (size_t i = 1; i < node->call.arg_count; i++)
			{
				node->call.args[i - 1] = node->call.args[i];
			}
			node->call.arg_count--;
			member_method = NULL;
		}
	}
	const char* lookup_name = member_method ? member_method : node->call.callee;
	if (!member_method && starts_with(lookup_name, "__"))
	{
		for (size_t i = 0; i < module_export_count; i++)
		{
			if (module_exports[i].public_name && module_exports[i].internal_name &&
			    strcmp(module_exports[i].public_name, lookup_name) == 0)
			{
				replace_with_clone(&node->call.callee, module_exports[i].internal_name);
				lookup_name = node->call.callee;
				break;
			}
		}
	}
	const char* receiver_type =
	    member_method && node->call.arg_count > 0
	        ? resolve_expression_type(analyzer, node->call.args[0])
	        : NULL;
	bool is_member_array_call = is_array_type_name(receiver_type) &&
	                           is_array_member_method_name(member_method);
	bool is_runtime_call = starts_with(node->call.callee, "adn_");
	bool is_internal_call = starts_with(node->call.callee, "__array_") ||
	                        strcmp(node->call.callee, "__string_format") == 0 ||
	                        is_member_array_call;
	if (is_runtime_call || is_internal_call)
	{
		for (size_t i = 0; i < node->call.arg_count; i++)
		{
			validate_node(analyzer, node->call.args[i]);
		}
		if (strcmp(node->call.callee, "__string_format") == 0 && node->call.arg_count > 0)
		{
			const char* format_type =
			    resolve_expression_type(analyzer, node->call.args[0]);
			if (format_type && strcmp(format_type, "string") != 0)
			{
				semantic_error(analyzer, node,
				               "String format receiver must be a string.");
			}
		}
		if (is_member_array_call)
		{
			if ((strcmp(member_method, "length") == 0 || strcmp(member_method, "len") == 0 ||
			     strcmp(member_method, "remove") == 0) &&
			    node->call.arg_count != 2 && strcmp(member_method, "remove") == 0)
			{
				semantic_error(analyzer, node, "Array remove expects one index argument.");
			}
			else if ((strcmp(member_method, "length") == 0 ||
			          strcmp(member_method, "len") == 0) &&
			         node->call.arg_count != 1)
			{
				semantic_error(analyzer, node, "Array length expects no additional arguments.");
			}
			else if (strcmp(member_method, "slice") == 0 &&
			         !(node->call.arg_count == 2 || node->call.arg_count == 3))
			{
				semantic_error(analyzer, node,
				               "Array slice expects one or two index arguments.");
			}
			else if (strcmp(member_method, "insert") == 0 && node->call.arg_count != 3)
			{
				semantic_error(analyzer, node,
				               "Array insert expects an index and a value.");
			}
		}
		return;
	}

	if (!analyzer->symbol_table_stack ||
	    stm_lookup(analyzer->symbol_table_stack->current_scope, lookup_name) == NULL)
	{
		semantic_error(analyzer, node, "Call to undefined symbol.");
	}

	FunctionSignature* signature = find_function_signature(lookup_name);
	if (!signature && starts_with(lookup_name, "__"))
	{
		for (size_t i = 0; i < module_export_count; i++)
		{
			if (module_exports[i].public_name && module_exports[i].internal_name &&
			    strcmp(module_exports[i].public_name, lookup_name) == 0)
			{
				signature = find_function_signature(module_exports[i].internal_name);
				if (signature)
				{
					break;
				}
			}
		}
	}
	if (!signature)
	{
		semantic_error(analyzer, node, "Call target is not a known function.");
	}
	else if ((!signature->is_variadic && node->call.arg_count != signature->param_count) ||
	         (signature->is_variadic && node->call.arg_count < signature->param_count))
	{
		semantic_error(analyzer, node, "Call argument count does not match.");
	}

	if (node->call.args)
	{
		for (size_t i = 0; i < node->call.arg_count; i++)
		{
			validate_node(analyzer, node->call.args[i]);
			if (signature)
			{
				const char* arg_type =
				    resolve_expression_type(analyzer, node->call.args[i]);
				const char* param_type = NULL;
				if (i < signature->param_count)
				{
					param_type = signature->param_types[i];
				}
				else if (signature->is_variadic)
				{
					param_type = signature->variadic_type;
				}
				if (arg_type && param_type &&
				    !semantic_types_compatible(param_type, arg_type))
				{
					if (strcmp(param_type, "string") == 0 &&
					    (strcmp(arg_type, "i32") == 0 ||
					     strcmp(arg_type, "i64") == 0))
					{
						ASTNode* cast_type = ast_create_type(
						    "string", node->call.args[i]->line,
						    node->call.args[i]->column);
						node->call.args[i] =
						    ast_create_cast(cast_type, node->call.args[i],
						                    node->call.args[i]->line,
						                    node->call.args[i]->column);
					}
					else
					{
						char message[128];
						snprintf(message, sizeof(message),
						         "Argument %zu type does not match "
						         "parameter type.",
						         i + 1);
						semantic_error(analyzer, node, message);
					}
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

void validate_binary_op(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (node->type != AST_BINARY_OP)
	{
		return;
	}

	if (node->binary_op.left)
	{
		validate_node(analyzer, node->binary_op.left);
	}
	else
	{
		semantic_error(analyzer, node, "Binary operation missing left operand.");
	}

	if (node->binary_op.right)
	{
		validate_node(analyzer, node->binary_op.right);
	}
	else
	{
		semantic_error(analyzer, node, "Binary operation missing right operand.");
	}

	const char* lt = resolve_expression_type(analyzer, node->binary_op.left);
	const char* rt = resolve_expression_type(analyzer, node->binary_op.right);
	const char* target = resolve_expression_type(analyzer, node);

	if ((lt && strcmp(lt, "string") == 0) || (rt && strcmp(rt, "string") == 0))
	{
		if (lt && rt && strcmp(lt, "string") == 0 && is_integer_type(rt) &&
		    node->binary_op.op && strcmp(node->binary_op.op, "+") == 0)
		{
			ASTNode* cast_type = ast_create_type("string", node->binary_op.right->line,
			                                     node->binary_op.right->column);
			node->binary_op.right = ast_create_cast(cast_type, node->binary_op.right,
			                                        node->binary_op.right->line,
			                                        node->binary_op.right->column);
		}
		else if (lt && rt && strcmp(rt, "string") == 0 && is_integer_type(lt) &&
		         node->binary_op.op && strcmp(node->binary_op.op, "+") == 0)
		{
			ASTNode* cast_type = ast_create_type("string", node->binary_op.left->line,
			                                     node->binary_op.left->column);
			node->binary_op.left = ast_create_cast(cast_type, node->binary_op.left,
			                                       node->binary_op.left->line,
			                                       node->binary_op.left->column);
		}
		else if (!lt || !rt || strcmp(lt, "string") != 0 || strcmp(rt, "string") != 0)
		{
			semantic_error(
			    analyzer, node,
			    "Type mismatch: cannot mix string and non-string in binary operation.");
		}
		else if (!node->binary_op.op || (strcmp(node->binary_op.op, "+") != 0 &&
		                                 strcmp(node->binary_op.op, "==") != 0 &&
		                                 strcmp(node->binary_op.op, "!==") != 0))
		{
			semantic_error(analyzer, node,
			               "Operator not supported for string type; only '+', '==', "
			               "and '!==' are allowed.");
		}
	}
	else if (lt && rt && target && (is_integer_type(target) || is_float_type(target)))
	{
		if (strcmp(lt, target) != 0)
		{
			ASTNode* cast_type =
			    ast_create_type((char*)target, node->binary_op.left->line,
			                    node->binary_op.left->column);
			node->binary_op.left = ast_create_cast(cast_type, node->binary_op.left,
			                                       node->binary_op.left->line,
			                                       node->binary_op.left->column);
		}
		if (strcmp(rt, target) != 0)
		{
			ASTNode* cast_type =
			    ast_create_type((char*)target, node->binary_op.right->line,
			                    node->binary_op.right->column);
			node->binary_op.right = ast_create_cast(cast_type, node->binary_op.right,
			                                        node->binary_op.right->line,
			                                        node->binary_op.right->column);
		}
	}
}

void validate_assignment(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node || node->type != AST_ASSIGNMENT)
	{
		return;
	}

	const char* name = node->assignment.name;
	if (!name)
	{
		semantic_error(analyzer, node, "Assignment missing variable name.");
		return;
	}

	if (!analyzer->symbol_table_stack || !analyzer->symbol_table_stack->current_scope)
	{
		return;
	}

	SymbolEntry* entry = stm_lookup(analyzer->symbol_table_stack->current_scope, name);
	if (!entry)
	{
		semantic_error(analyzer, node, "Assignment to undeclared variable.");
		return;
	}
	if (!entry->is_mutable)
	{
		semantic_error(analyzer, node, "Cannot assign to an immutable variable.");
		return;
	}

	if (node->assignment.value)
	{
		validate_node(analyzer, node->assignment.value);
		const char* val_type = resolve_expression_type(analyzer, node->assignment.value);
		if (val_type && entry->type)
		{
			if (strcmp(entry->type, val_type) != 0 &&
			    semantic_types_compatible(entry->type, val_type))
			{
				node->assignment.value = ast_create_cast(
				    ast_create_type(entry->type, node->line, node->column),
				    node->assignment.value, node->line, node->column);
			}
			else if (!semantic_types_compatible(entry->type, val_type))
			{
				semantic_error(
				    analyzer, node,
				    "Assignment value type does not match variable type.");
			}
		}
	}
	else
	{
		semantic_error(analyzer, node, "Assignment missing value.");
	}
}

void validate_return_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (node->type != AST_RETURN_STATEMENT)
	{
		return;
	}

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
			return;
		}
		semantic_error(analyzer, node, "Empty return in non-void function is not allowed.");
	}
}

void validate_expression_statement(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node)
	{
		return;
	}

	if (node->type != AST_EXPRESSION_STATEMENT)
	{
		return;
	}

	ASTNode* expr = node->expr_stmt.expr;
	if (expr)
	{
		validate_node(analyzer, expr);
	}
}

void validate_cast(SemanticAnalyzer* analyzer, ASTNode* node)
{
	if (!analyzer || !node || node->type != AST_CAST)
	{
		return;
	}

	if (node->cast.target_type)
	{
		validate_type_node(analyzer, node->cast.target_type);
	}
	else
	{
		semantic_error(analyzer, node, "Cast expression missing target type.");
	}

	if (node->cast.expr)
	{
		validate_node(analyzer, node->cast.expr);
	}
	else
	{
		semantic_error(analyzer, node, "Cast expression missing inner expression.");
	}
}
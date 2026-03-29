#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lower.h"
#include "ir/ir.h"

static IRFunction* current_function = NULL;

static IRType* lower_type(Program* program, ASTNode* node);

static IRBlock* current_block = NULL;

static IRBlock* loop_break_targets[128];

static IRBlock* loop_continue_targets[128];

static size_t loop_target_depth = 0;

static SymEntry* sym_table = NULL;

typedef struct ReachableFunctionName
{
	char* name;
	struct ReachableFunctionName* next;
} ReachableFunctionName;

static void emit_global_inits(IRBlock* block, ASTNode* root, Program* program,
                              IRFunction* init_func);

static SymEntry* sym_get(const char* name);

static IRValue* lower_expression(Program* program, ASTNode* node);

static const char* infer_expression_type(Program* program, ASTNode* node);

static IRFunction* find_ir_function(IRModule* module, const char* name);

static ASTNode* find_function_declaration(ASTNode* root, const char* name);

static void collect_called_functions(Program* program, ASTNode* node,
                                     ReachableFunctionName** reachable_functions);

static void mark_function_reachable(Program* program, ReachableFunctionName** reachable_functions,
                                    const char* name);

static IRFunction* ensure_program_function(Program* program, const char* name);

static IRFunction* ensure_runtime_function(Program* program, const char* name);

static IRValue* lower_string_conversion(Program* program, ASTNode* node, IRValue* value,
                                        const char* type_name);

static IRValue* lower_array_method_call(Program* program, ASTNode* node, IRValue** args,
                                        size_t nargs);

static IRValue* lower_string_method_call(Program* program, ASTNode* node, IRValue** args,
                                         size_t nargs);

static IRValue* lower_object_literal(Program* program, ASTNode* node);

static IRValue* lower_array_literal(Program* program, ASTNode* node);

static IRValue* lower_member_access(Program* program, ASTNode* node);

static IRValue* lower_array_access(Program* program, ASTNode* node);

static IRValue* lower_variadic_pack_array(Program* program, ASTNode** arg_nodes,
                                          IRValue** arg_values, size_t start, size_t end);

static bool block_is_terminated(IRBlock* block)
{
	return block && block->last &&
	       (block->last->kind == IR_RET || block->last->kind == IR_BR ||
	        block->last->kind == IR_CBR);
}

static bool starts_with(const char* text, const char* prefix)
{
	return text && prefix && strncmp(text, prefix, strlen(prefix)) == 0;
}

static const char* member_method_name(const char* callee)
{
	const char* prefix = "__member_";
	return starts_with(callee, prefix) ? callee + strlen(prefix) : NULL;
}

static int is_array_member_method_name(const char* method_name)
{
	return method_name &&
	       (strcmp(method_name, "length") == 0 || strcmp(method_name, "len") == 0 ||
	        strcmp(method_name, "slice") == 0 || strcmp(method_name, "remove") == 0 ||
	        strcmp(method_name, "insert") == 0);
}

static const char* array_member_runtime_name(const char* method_name)
{
	if (!method_name)
	{
		return NULL;
	}
	if (strcmp(method_name, "length") == 0 || strcmp(method_name, "len") == 0)
	{
		return "__array_length";
	}
	if (strcmp(method_name, "slice") == 0)
	{
		return "__array_slice";
	}
	if (strcmp(method_name, "remove") == 0)
	{
		return "__array_remove";
	}
	if (strcmp(method_name, "insert") == 0)
	{
		return "__array_insert";
	}
	return NULL;
}

static int reachable_function_contains(ReachableFunctionName* reachable_functions, const char* name)
{
	for (ReachableFunctionName* it = reachable_functions; it; it = it->next)
	{
		if (it->name && name && strcmp(it->name, name) == 0)
		{
			return 1;
		}
	}
	return 0;
}

static int is_function_reachable(ReachableFunctionName* reachable_functions, const char* name)
{
	return reachable_function_contains(reachable_functions, name);
}

static void free_reachable_functions(ReachableFunctionName* reachable_functions)
{
	while (reachable_functions)
	{
		ReachableFunctionName* next = reachable_functions->next;
		free(reachable_functions->name);
		free(reachable_functions);
		reachable_functions = next;
	}
}

static bool is_array_type_name(const char* type_name)
{
	return starts_with(type_name, "array<") && type_name[strlen(type_name) - 1] == '>';
}

static bool is_object_type_name(const char* type_name)
{
	return starts_with(type_name, "object{") && type_name[strlen(type_name) - 1] == '}';
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

static IRValue* lower_variadic_pack_array(Program* program, ASTNode** arg_nodes,
                                          IRValue** arg_values, size_t start, size_t end)
{
	IRFunction* create_fn = ensure_runtime_function(program, "adn_array_create");
	IRValue* array = ir_emit_call(current_block, create_fn, NULL, 0);
	for (size_t i = start; i < end; i++)
	{
		IRValue* push_args[2] = {array, arg_values ? arg_values[i] : NULL};
		ASTNode call_wrapper = {.type = AST_CALL};
		call_wrapper.call.callee = "__array_push";
		ASTNode* wrapper_args[2] = {NULL, arg_nodes ? arg_nodes[i] : NULL};
		call_wrapper.call.args = wrapper_args;
		call_wrapper.call.arg_count = 2;
		lower_array_method_call(program, &call_wrapper, push_args, 2);
	}
	return array;
}

static const char* extract_array_element_type(const char* array_type)
{
	if (!is_array_type_name(array_type))
	{
		return NULL;
	}

	char buffer[256];
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
		if (out + 1 < sizeof(buffer))
		{
			buffer[out++] = current;
		}
	}
	buffer[out] = '\0';
	return cache_type_string(buffer);
}

static const char* build_array_type_name(const char* element_type)
{
	char buffer[256];
	snprintf(buffer, sizeof(buffer), "array<%s>", element_type ? element_type : "any");
	return cache_type_string(buffer);
}

static const char* find_object_property_type(const char* object_type, const char* property_name)
{
	if (!is_object_type_name(object_type) || !property_name)
	{
		return NULL;
	}

	const char* cursor = object_type + 7;
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

static void sym_put(const char* name, const char* type_name, IRValue* v, int is_addr)
{
	if (!name || !v)
	{
		return;
	}
	SymEntry* existing = sym_get(name);
	if (existing)
	{
		if (type_name)
		{
			char* duplicated_type = strdup(type_name);
			if (!duplicated_type)
			{
				return;
			}
			free(existing->type_name);
			existing->type_name = duplicated_type;
		}
		existing->value = v;
		existing->is_address = is_addr ? 1 : 0;
		return;
	}
	SymEntry* e = (SymEntry*)malloc(sizeof(SymEntry));
	if (!e)
	{
		return;
	}
	e->name = strdup(name);
	e->type_name = type_name ? strdup(type_name) : NULL;
	e->value = v;
	e->is_address = is_addr ? 1 : 0;
	e->next = sym_table;
	sym_table = e;
}

static SymEntry* sym_get(const char* name)
{
	if (!name)
	{
		return NULL;
	}
	SymEntry* it = sym_table;
	while (it)
	{
		if (it->name && strcmp(it->name, name) == 0)
		{
			return it;
		}
		it = it->next;
	}
	return NULL;
}

static void sym_clear(void)
{
	SymEntry* it = sym_table;
	while (it)
	{
		SymEntry* nxt = it->next;
		free(it->name);
		free(it->type_name);
		free(it);
		it = nxt;
	}
	sym_table = NULL;
}

static IRFunction* find_ir_function(IRModule* module, const char* name)
{
	if (!module || !name)
	{
		return NULL;
	}
	for (IRFunction* it = module->functions; it; it = it->next)
	{
		if (it->name && strcmp(it->name, name) == 0)
		{
			return it;
		}
	}
	return NULL;
}

static ASTNode* find_function_declaration(ASTNode* root, const char* name)
{
	if (!root || root->type != AST_PROGRAM || !name)
	{
		return NULL;
	}
	for (size_t i = 0; i < root->program.count; i++)
	{
		ASTNode* decl = root->program.decls[i];
		if (decl && decl->type == AST_FUNCTION_DECLARATION && decl->func_decl.name &&
		    strcmp(decl->func_decl.name, name) == 0)
		{
			return decl;
		}
	}
	return NULL;
}

static void mark_function_reachable(Program* program, ReachableFunctionName** reachable_functions,
                                    const char* name)
{
	if (!program || !reachable_functions || !name)
	{
		return;
	}

	ASTNode* decl = find_function_declaration(program->ast_root, name);
	if (!decl || reachable_function_contains(*reachable_functions, name))
	{
		return;
	}

	ReachableFunctionName* entry =
	    (ReachableFunctionName*)calloc(1, sizeof(ReachableFunctionName));
	if (!entry)
	{
		return;
	}
	entry->name = strdup(name);
	if (!entry->name)
	{
		free(entry);
		return;
	}
	entry->next = *reachable_functions;
	*reachable_functions = entry;

	collect_called_functions(program, decl->func_decl.body, reachable_functions);
}

static void collect_called_functions(Program* program, ASTNode* node,
                                     ReachableFunctionName** reachable_functions)
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
				collect_called_functions(program, node->program.decls[i],
				                         reachable_functions);
			}
			break;
		case AST_FUNCTION_DECLARATION:
			collect_called_functions(program, node->func_decl.body,
			                         reachable_functions);
			break;
		case AST_VARIABLE_DECLARATION:
			collect_called_functions(program, node->var_decl.initializer,
			                         reachable_functions);
			break;
		case AST_IF_STATEMENT:
			collect_called_functions(program, node->if_stmt.condition,
			                         reachable_functions);
			collect_called_functions(program, node->if_stmt.then_branch,
			                         reachable_functions);
			collect_called_functions(program, node->if_stmt.else_branch,
			                         reachable_functions);
			break;
		case AST_BLOCK:
			for (size_t i = 0; i < node->block.count; i++)
			{
				collect_called_functions(program, node->block.statements[i],
				                         reachable_functions);
			}
			break;
		case AST_CALL:
			if (node->call.callee)
			{
				const char* member_method = member_method_name(node->call.callee);
				mark_function_reachable(program, reachable_functions,
				                        member_method ? member_method : node->call.callee);
			}
			for (size_t i = 0; i < node->call.arg_count; i++)
			{
				collect_called_functions(program, node->call.args[i],
				                         reachable_functions);
			}
			break;
		case AST_RETURN_STATEMENT:
			collect_called_functions(program, node->ret.expr, reachable_functions);
			break;
		case AST_EXPRESSION_STATEMENT:
			collect_called_functions(program, node->expr_stmt.expr,
			                         reachable_functions);
			break;
		case AST_WHILE_STMT:
			collect_called_functions(program, node->while_stmt.condition,
			                         reachable_functions);
			collect_called_functions(program, node->while_stmt.body,
			                         reachable_functions);
			break;
		case AST_FOR_STMT:
			collect_called_functions(program, node->for_stmt.var_decl,
			                         reachable_functions);
			collect_called_functions(program, node->for_stmt.condition,
			                         reachable_functions);
			collect_called_functions(program, node->for_stmt.increment,
			                         reachable_functions);
			collect_called_functions(program, node->for_stmt.body, reachable_functions);
			break;
		case AST_BINARY_OP:
			collect_called_functions(program, node->binary_op.left,
			                         reachable_functions);
			collect_called_functions(program, node->binary_op.right,
			                         reachable_functions);
			break;
		case AST_ASSIGNMENT:
			collect_called_functions(program, node->assignment.value,
			                         reachable_functions);
			break;
		case AST_CAST:
			collect_called_functions(program, node->cast.expr, reachable_functions);
			break;
		case AST_OBJECT_LITERAL:
			for (size_t i = 0; i < node->object_literal.count; i++)
			{
				collect_called_functions(program,
				                         node->object_literal.properties[i].value,
				                         reachable_functions);
			}
			break;
		case AST_ARRAY_LITERAL:
			for (size_t i = 0; i < node->array_literal.count; i++)
			{
				collect_called_functions(program, node->array_literal.elements[i],
				                         reachable_functions);
			}
			break;
		case AST_MEMBER_ACCESS:
			collect_called_functions(program, node->member_access.object,
			                         reachable_functions);
			collect_called_functions(program, node->member_access.property,
			                         reachable_functions);
			break;
		case AST_ARRAY_ACCESS:
			collect_called_functions(program, node->array_access.array,
			                         reachable_functions);
			collect_called_functions(program, node->array_access.index,
			                         reachable_functions);
			break;
		case AST_TYPE_DECLARATION:
		case AST_IMPORT_STATEMENT:
		case AST_PARAMETER:
		case AST_IDENTIFIER:
		case AST_STRING_LITERAL:
		case AST_NUMBER_LITERAL:
		case AST_TYPE:
		case AST_BOOLEAN_LITERAL:
		case AST_BREAK_STATEMENT:
		case AST_CONTINUE_STATEMENT:
			break;
	}
}

static ReachableFunctionName* collect_reachable_functions(Program* program)
{
	if (!program || !program->ast_root || program->ast_root->type != AST_PROGRAM)
	{
		return NULL;
	}

	ReachableFunctionName* reachable_functions = NULL;
	mark_function_reachable(program, &reachable_functions, "main");

	ASTNode* root = program->ast_root;
	for (size_t i = 0; i < root->program.count; i++)
	{
		ASTNode* decl = root->program.decls[i];
		if (!decl || decl->type == AST_FUNCTION_DECLARATION ||
		    decl->type == AST_IMPORT_STATEMENT || decl->type == AST_TYPE_DECLARATION)
		{
			continue;
		}
		collect_called_functions(program, decl, &reachable_functions);
	}

	return reachable_functions;
}

static IRType* lower_type_name(const char* type_name)
{
	if (!type_name)
	{
		return NULL;
	}
	if (strcmp(type_name, "i64") == 0 || strcmp(type_name, "i32") == 0 ||
	    strcmp(type_name, "u32") == 0 || strcmp(type_name, "u64") == 0 ||
	    strcmp(type_name, "bool") == 0 || strcmp(type_name, "i8") == 0 ||
	    strcmp(type_name, "u8") == 0)
	{
		return ir_type_i64();
	}
	if (strcmp(type_name, "f64") == 0)
	{
		return ir_type_f64();
	}
	if (strcmp(type_name, "f32") == 0)
	{
		return ir_type_f32();
	}
	if (strcmp(type_name, "void") == 0)
	{
		return ir_type_void();
	}
	return ir_type_ptr(ir_type_i64());
}

static IRFunction* ensure_program_function(Program* program, const char* name)
{
	IRFunction* existing = find_ir_function(program->ir, name);
	if (existing)
	{
		return existing;
	}

	ASTNode* decl = find_function_declaration(program->ast_root, name);
	if (!decl)
	{
		return NULL;
	}

	IRFunction* fn = ir_function_create_in_module(
	    program->ir, name, lower_type(program, decl->func_decl.return_type));
	if (!fn)
	{
		return NULL;
	}
	for (size_t i = 0; i < decl->func_decl.param_count; i++)
	{
		ASTNode* param = decl->func_decl.params[i];
		ir_param_create(fn, param->param.name, lower_type(program, param->param.type));
	}
	if (decl->func_decl.is_variadic && decl->func_decl.variadic_name)
	{
		const char* variadic_array_type = build_array_type_name(
		    decl->func_decl.variadic_type ? decl->func_decl.variadic_type->type_node.name
		                                  : "any");
		ir_param_create(fn, decl->func_decl.variadic_name,
		                lower_type_name(variadic_array_type));
	}
	return fn;
}

static IRFunction* ensure_runtime_function(Program* program, const char* name)
{
	IRFunction* fn = find_ir_function(program->ir, name);
	if (fn)
	{
		return fn;
	}

	if (strcmp(name, "adn_print") == 0 || strcmp(name, "adn_error") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_input") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_flush") == 0)
	{
		return ir_function_create_in_module(program->ir, name, ir_type_void());
	}
	if (strcmp(name, "adn_write_file") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_read_file") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_strconcat") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_string_format") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_i32_to_string") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_f64_to_string") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_f64());
		return fn;
	}
	if (strcmp(name, "adn_string_to_i32") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_string_to_f64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_f64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_string_length") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_string_char_at") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_string_code_at") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_string_from_code") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_process_id") == 0 || strcmp(name, "adn_process_parent_id") == 0 ||
	    strcmp(name, "adn_process_arg_count") == 0)
	{
		return ir_function_create_in_module(program->ir, name, ir_type_i64());
	}
	if (strcmp(name, "adn_process_arg") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_process_name") == 0 || strcmp(name, "adn_process_env") == 0 ||
	    strcmp(name, "adn_process_cwd") == 0 || strcmp(name, "adn_process_os") == 0 ||
	    strcmp(name, "adn_process_arch") == 0 ||
	    strcmp(name, "adn_process_executable_path") == 0 ||
	    strcmp(name, "adn_process_home_dir") == 0 ||
	    strcmp(name, "adn_process_temp_dir") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		if (strcmp(name, "adn_process_env") == 0)
		{
			ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		}
		return fn;
	}
	if (strcmp(name, "adn_process_args") == 0 || strcmp(name, "adn_process_env_keys") == 0)
	{
		return ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
	}
	if (strcmp(name, "adn_process_set_env") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_process_has_env") == 0 || strcmp(name, "adn_process_chdir") == 0 ||
	    strcmp(name, "adn_process_run") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_process_exit") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_process_abort") == 0)
	{
		return ir_function_create_in_module(program->ir, name, ir_type_void());
	}
	if (strcmp(name, "adn_process_is_windows") == 0 ||
	    strcmp(name, "adn_process_is_linux") == 0 ||
	    strcmp(name, "adn_process_is_macos") == 0)
	{
		return ir_function_create_in_module(program->ir, name, ir_type_i64());
	}
	if (strcmp(name, "adn_process_run_args") == 0 || strcmp(name, "adn_process_spawn") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_process_run_capture") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_process_run_capture_args") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_process_kill") == 0 || strcmp(name, "adn_process_is_running") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_object_create") == 0 || strcmp(name, "adn_array_create") == 0)
	{
		return ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
	}
	if (strcmp(name, "adn_object_set_i64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_object_set_f64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_f64());
		return fn;
	}
	if (strcmp(name, "adn_object_set_string") == 0 || strcmp(name, "adn_object_set_ptr") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_object_get_i64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_object_get_f64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_f64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_object_get_string") == 0 || strcmp(name, "adn_object_get_ptr") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_array_push_i64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_array_pop_i64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_array_push_f64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_f64());
		return fn;
	}
	if (strcmp(name, "adn_array_pop_f64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_f64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_array_push_string") == 0 || strcmp(name, "adn_array_push_ptr") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_array_pop_string") == 0 || strcmp(name, "adn_array_pop_ptr") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_array_clear") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_array_length") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_array_get_i64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_array_get_f64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_f64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_array_get_string") == 0 || strcmp(name, "adn_array_get_ptr") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_array_insert_i64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_array_insert_f64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_f64());
		return fn;
	}
	if (strcmp(name, "adn_array_insert_string") == 0 ||
	    strcmp(name, "adn_array_insert_ptr") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_void());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		return fn;
	}
	if (strcmp(name, "adn_array_remove_i64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_array_remove_f64") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_f64());
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_array_remove_string") == 0 ||
	    strcmp(name, "adn_array_remove_ptr") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}
	if (strcmp(name, "adn_array_slice") == 0)
	{
		fn = ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_ptr(ir_type_i64()));
		ir_param_create(fn, NULL, ir_type_i64());
		ir_param_create(fn, NULL, ir_type_i64());
		return fn;
	}

	return ir_function_create_in_module(program->ir, name, ir_type_ptr(ir_type_i64()));
}

static const char* infer_expression_type(Program* program, ASTNode* node)
{
	if (!node)
	{
		return NULL;
	}
	switch (node->type)
	{
		case AST_IDENTIFIER:
		{
			SymEntry* entry = sym_get(node->identifier.name);
			return entry ? entry->type_name : NULL;
		}
		case AST_STRING_LITERAL:
			return "string";
		case AST_NUMBER_LITERAL:
			return strchr(node->number_literal.value, '.') ? "f64" : "i32";
		case AST_BOOLEAN_LITERAL:
			return "bool";
		case AST_TYPE:
			return node->type_node.name;
		case AST_CAST:
			return node->cast.target_type ? node->cast.target_type->type_node.name
			                              : NULL;
		case AST_OBJECT_LITERAL:
			return "object";
		case AST_ARRAY_LITERAL:
		{
			const char* element_type =
			    node->array_literal.count > 0
			        ? infer_expression_type(program, node->array_literal.elements[0])
			        : "any";
			char buffer[256];
			snprintf(buffer, sizeof(buffer), "array<%s>",
			         element_type ? element_type : "any");
			return cache_type_string(buffer);
		}
		case AST_MEMBER_ACCESS:
		{
			const char* object_type =
			    infer_expression_type(program, node->member_access.object);
			if (node->member_access.property &&
			    node->member_access.property->type == AST_IDENTIFIER)
			{
				const char* property_type = find_object_property_type(
				    object_type, node->member_access.property->identifier.name);
				return property_type ? property_type : "any";
			}
			return "any";
		}
		case AST_ARRAY_ACCESS:
		{
			const char* array_type =
			    infer_expression_type(program, node->array_access.array);
			const char* element_type = extract_array_element_type(array_type);
			return element_type ? element_type : "any";
		}
		case AST_CALL:
		{
			const char* member_method = member_method_name(node->call.callee);
			if (member_method)
			{
				const char* receiver_type =
				    node->call.arg_count > 0
				        ? infer_expression_type(program, node->call.args[0])
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

				ASTNode* decl = find_function_declaration(program->ast_root, member_method);
				return decl ? decl->func_decl.return_type->type_node.name : NULL;
			}

			if (strcmp(node->call.callee, "__string_format") == 0)
			{
				return "string";
			}
			if (starts_with(node->call.callee, "__array_"))
			{
				const char* array_type =
				    node->call.arg_count > 0
				        ? infer_expression_type(program, node->call.args[0])
				        : NULL;
				const char* element_type = extract_array_element_type(array_type);
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
				if (strcmp(node->call.callee, "adn_string_format") == 0 ||
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
				    strstr(node->call.callee, "_to_string"))
				{
					return "string";
				}
				if (strstr(node->call.callee, "_to_f64"))
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
				if (strstr(node->call.callee, "_get_f64"))
				{
					return "f64";
				}
				if (strstr(node->call.callee, "_get_string") ||
				    strcmp(node->call.callee, "adn_input") == 0)
				{
					return "string";
				}
				if (strstr(node->call.callee, "create") ||
				    strstr(node->call.callee, "_get_ptr"))
				{
					return "any";
				}
			}
			{
				ASTNode* decl =
				    find_function_declaration(program->ast_root, node->call.callee);
				return decl ? decl->func_decl.return_type->type_node.name : NULL;
			}
		}
		case AST_BINARY_OP:
		{
			if (node->binary_op.op && (strcmp(node->binary_op.op, "==") == 0 ||
			                           strcmp(node->binary_op.op, "!==") == 0 ||
			                           strcmp(node->binary_op.op, "<") == 0 ||
			                           strcmp(node->binary_op.op, "<=") == 0 ||
			                           strcmp(node->binary_op.op, ">") == 0 ||
			                           strcmp(node->binary_op.op, ">=") == 0 ||
			                           strcmp(node->binary_op.op, "and") == 0 ||
			                           strcmp(node->binary_op.op, "or") == 0 ||
			                           strcmp(node->binary_op.op, "not") == 0))
			{
				return "bool";
			}
			const char* left = infer_expression_type(program, node->binary_op.left);
			const char* right = infer_expression_type(program, node->binary_op.right);
			if ((left && strcmp(left, "string") == 0) ||
			    (right && strcmp(right, "string") == 0))
			{
				return "string";
			}
			if ((left && strcmp(left, "f64") == 0) ||
			    (right && strcmp(right, "f64") == 0))
			{
				return "f64";
			}
			return "i32";
		}
		default:
			return NULL;
	}
}

static IRValue* lower_string_conversion(Program* program, ASTNode* node, IRValue* value,
                                        const char* type_name)
{
	if (!value)
	{
		return NULL;
	}
	if (!type_name || strcmp(type_name, "string") == 0)
	{
		return value;
	}
	if (strcmp(type_name, "f32") == 0 || strcmp(type_name, "f64") == 0)
	{
		if (value->type && value->type->kind == IR_T_F32)
		{
			value = ir_emit_fpcvt(current_block, value, ir_type_f64());
		}
		IRFunction* fn = ensure_runtime_function(program, "adn_f64_to_string");
		IRValue* args[1] = {value};
		return ir_emit_call(current_block, fn, args, 1);
	}
	IRFunction* fn = ensure_runtime_function(program, "adn_i32_to_string");
	IRValue* args[1] = {value};
	return ir_emit_call(current_block, fn, args, 1);
}

static IRValue* lower_array_method_call(Program* program, ASTNode* node, IRValue** args,
                                        size_t nargs)
{
	if (strcmp(node->call.callee, "__array_clear") == 0)
	{
		IRFunction* fn = ensure_runtime_function(program, "adn_array_clear");
		return ir_emit_call(current_block, fn, args, nargs);
	}
	if (strcmp(node->call.callee, "__array_length") == 0)
	{
		IRFunction* fn = ensure_runtime_function(program, "adn_array_length");
		return ir_emit_call(current_block, fn, args, nargs);
	}
	if (strcmp(node->call.callee, "__array_push") == 0 && nargs == 2)
	{
		const char* value_type = infer_expression_type(program, node->call.args[1]);
		const char* runtime_name = "adn_array_push_ptr";
		if (!value_type || strcmp(value_type, "string") == 0)
		{
			runtime_name = "adn_array_push_string";
		}
		else if (strcmp(value_type, "f32") == 0 || strcmp(value_type, "f64") == 0)
		{
			runtime_name = "adn_array_push_f64";
			if (args[1] && args[1]->type && args[1]->type->kind == IR_T_F32)
			{
				args[1] = ir_emit_fpcvt(current_block, args[1], ir_type_f64());
			}
		}
		else if (strcmp(value_type, "object") == 0 || strcmp(value_type, "any") == 0 ||
		         is_array_type_name(value_type) || is_object_type_name(value_type))
		{
			runtime_name = "adn_array_push_ptr";
		}
		else
		{
			runtime_name = "adn_array_push_i64";
		}
		IRFunction* fn = ensure_runtime_function(program, runtime_name);
		return ir_emit_call(current_block, fn, args, nargs);
	}
	if (strcmp(node->call.callee, "__array_pop") == 0 && nargs == 1)
	{
		const char* array_type = infer_expression_type(program, node->call.args[0]);
		const char* element_type = extract_array_element_type(array_type);
		const char* runtime_name = "adn_array_pop_ptr";
		if (!element_type || strcmp(element_type, "string") == 0)
		{
			runtime_name = "adn_array_pop_string";
		}
		else if (strcmp(element_type, "f32") == 0 || strcmp(element_type, "f64") == 0)
		{
			runtime_name = "adn_array_pop_f64";
		}
		else if (strcmp(element_type, "object") == 0 || strcmp(element_type, "any") == 0 ||
		         is_array_type_name(element_type) || is_object_type_name(element_type))
		{
			runtime_name = "adn_array_pop_ptr";
		}
		else
		{
			runtime_name = "adn_array_pop_i64";
		}
		IRFunction* fn = ensure_runtime_function(program, runtime_name);
		IRValue* value = ir_emit_call(current_block, fn, args, nargs);
		if (element_type && strcmp(element_type, "f32") == 0)
		{
			return ir_emit_fpcvt(current_block, value, ir_type_f32());
		}
		return value;
	}
	if (strcmp(node->call.callee, "__array_insert") == 0 && nargs == 3)
	{
		const char* array_type = infer_expression_type(program, node->call.args[0]);
		const char* element_type = extract_array_element_type(array_type);
		const char* runtime_name = "adn_array_insert_ptr";
		if (!element_type || strcmp(element_type, "string") == 0)
		{
			runtime_name = "adn_array_insert_string";
		}
		else if (strcmp(element_type, "f32") == 0 || strcmp(element_type, "f64") == 0)
		{
			runtime_name = "adn_array_insert_f64";
			if (args[2] && args[2]->type && args[2]->type->kind == IR_T_F32)
			{
				args[2] = ir_emit_fpcvt(current_block, args[2], ir_type_f64());
			}
		}
		else if (strcmp(element_type, "object") == 0 || strcmp(element_type, "any") == 0 ||
		         is_array_type_name(element_type) || is_object_type_name(element_type))
		{
			runtime_name = "adn_array_insert_ptr";
		}
		else
		{
			runtime_name = "adn_array_insert_i64";
		}
		IRFunction* fn = ensure_runtime_function(program, runtime_name);
		return ir_emit_call(current_block, fn, args, nargs);
	}
	if (strcmp(node->call.callee, "__array_slice") == 0 && (nargs == 2 || nargs == 3))
	{
		IRFunction* fn = ensure_runtime_function(program, "adn_array_slice");
		if (nargs == 3)
		{
			return ir_emit_call(current_block, fn, args, nargs);
		}
		IRFunction* length_fn = ensure_runtime_function(program, "adn_array_length");
		IRValue* length_args[1] = {args[0]};
		IRValue* end = ir_emit_call(current_block, length_fn, length_args, 1);
		IRValue* slice_args[3] = {args[0], args[1], end};
		return ir_emit_call(current_block, fn, slice_args, 3);
	}
	if (strcmp(node->call.callee, "__array_remove") == 0 && nargs == 2)
	{
		const char* array_type = infer_expression_type(program, node->call.args[0]);
		const char* element_type = extract_array_element_type(array_type);
		const char* runtime_name = "adn_array_remove_ptr";
		if (!element_type || strcmp(element_type, "string") == 0)
		{
			runtime_name = "adn_array_remove_string";
		}
		else if (strcmp(element_type, "f32") == 0 || strcmp(element_type, "f64") == 0)
		{
			runtime_name = "adn_array_remove_f64";
		}
		else if (strcmp(element_type, "object") == 0 || strcmp(element_type, "any") == 0 ||
		         is_array_type_name(element_type) || is_object_type_name(element_type))
		{
			runtime_name = "adn_array_remove_ptr";
		}
		else
		{
			runtime_name = "adn_array_remove_i64";
		}
		IRFunction* fn = ensure_runtime_function(program, runtime_name);
		IRValue* value = ir_emit_call(current_block, fn, args, nargs);
		if (element_type && strcmp(element_type, "f32") == 0)
		{
			return ir_emit_fpcvt(current_block, value, ir_type_f32());
		}
		return value;
	}
	return NULL;
}

static IRValue* lower_string_method_call(Program* program, ASTNode* node, IRValue** args,
                                         size_t nargs)
{
	if (strcmp(node->call.callee, "__string_format") == 0 && nargs >= 1)
	{
		const char* format_type = infer_expression_type(program, node->call.args[0]);
		if (!format_type || strcmp(format_type, "string") != 0)
		{
			args[0] = lower_string_conversion(program, node->call.args[0], args[0],
			                                  format_type);
		}
		IRValue* packed_args =
		    lower_variadic_pack_array(program, node->call.args, args, 1, nargs);
		IRFunction* fn = ensure_runtime_function(program, "adn_string_format");
		IRValue* call_args[2] = {args[0], packed_args};
		return ir_emit_call(current_block, fn, call_args, 2);
	}
	return NULL;
}

static IRValue* lower_object_literal(Program* program, ASTNode* node)
{
	IRFunction* create_fn = ensure_runtime_function(program, "adn_object_create");
	IRValue* object = ir_emit_call(current_block, create_fn, NULL, 0);
	for (size_t i = 0; i < node->object_literal.count; i++)
	{
		ASTObjectProperty property = node->object_literal.properties[i];
		IRValue* key = ir_const_string(program->ir, property.key);
		IRValue* value = lower_expression(program, property.value);
		const char* value_type = infer_expression_type(program, property.value);
		const char* runtime_name = "adn_object_set_ptr";
		if (!value_type || strcmp(value_type, "string") == 0)
		{
			runtime_name = "adn_object_set_string";
		}
		else if (strcmp(value_type, "f32") == 0 || strcmp(value_type, "f64") == 0)
		{
			runtime_name = "adn_object_set_f64";
			if (value && value->type && value->type->kind == IR_T_F32)
			{
				value = ir_emit_fpcvt(current_block, value, ir_type_f64());
			}
		}
		else if (strcmp(value_type, "object") == 0 || strcmp(value_type, "any") == 0 ||
		         is_array_type_name(value_type) || is_object_type_name(value_type))
		{
			runtime_name = "adn_object_set_ptr";
		}
		else
		{
			runtime_name = "adn_object_set_i64";
		}
		IRFunction* set_fn = ensure_runtime_function(program, runtime_name);
		IRValue* args[3] = {object, key, value};
		ir_emit_call(current_block, set_fn, args, 3);
	}
	return object;
}

static IRValue* lower_array_literal(Program* program, ASTNode* node)
{
	IRFunction* create_fn = ensure_runtime_function(program, "adn_array_create");
	IRValue* array = ir_emit_call(current_block, create_fn, NULL, 0);
	for (size_t i = 0; i < node->array_literal.count; i++)
	{
		IRValue* pushed_value = lower_expression(program, node->array_literal.elements[i]);
		IRValue* args[2] = {array, pushed_value};
		ASTNode call_wrapper = {.type = AST_CALL};
		call_wrapper.call.callee = "__array_push";
		ASTNode* wrapper_args[2] = {NULL, node->array_literal.elements[i]};
		call_wrapper.call.args = wrapper_args;
		call_wrapper.call.arg_count = 2;
		lower_array_method_call(program, &call_wrapper, args, 2);
	}
	return array;
}

static IRValue* lower_member_access(Program* program, ASTNode* node)
{
	const char* property_type = infer_expression_type(program, node);
	IRValue* object = lower_expression(program, node->member_access.object);
	IRValue* key = ir_const_string(program->ir, node->member_access.property->identifier.name);
	const char* runtime_name = "adn_object_get_ptr";
	if (!property_type || strcmp(property_type, "string") == 0)
	{
		runtime_name = "adn_object_get_string";
	}
	else if (strcmp(property_type, "f32") == 0 || strcmp(property_type, "f64") == 0)
	{
		runtime_name = "adn_object_get_f64";
	}
	else if (strcmp(property_type, "object") == 0 || strcmp(property_type, "any") == 0 ||
	         is_array_type_name(property_type) || is_object_type_name(property_type))
	{
		runtime_name = "adn_object_get_ptr";
	}
	else
	{
		runtime_name = "adn_object_get_i64";
	}
	IRFunction* fn = ensure_runtime_function(program, runtime_name);
	IRValue* args[2] = {object, key};
	IRValue* value = ir_emit_call(current_block, fn, args, 2);
	if (property_type && strcmp(property_type, "f32") == 0)
	{
		return ir_emit_fpcvt(current_block, value, ir_type_f32());
	}
	return value;
}

static IRValue* lower_array_access(Program* program, ASTNode* node)
{
	const char* element_type = infer_expression_type(program, node);
	IRValue* array = lower_expression(program, node->array_access.array);
	IRValue* index = lower_expression(program, node->array_access.index);
	const char* runtime_name = "adn_array_get_ptr";
	if (!element_type || strcmp(element_type, "string") == 0)
	{
		runtime_name = "adn_array_get_string";
	}
	else if (strcmp(element_type, "f32") == 0 || strcmp(element_type, "f64") == 0)
	{
		runtime_name = "adn_array_get_f64";
	}
	else if (strcmp(element_type, "object") == 0 || strcmp(element_type, "any") == 0 ||
	         is_array_type_name(element_type) || is_object_type_name(element_type))
	{
		runtime_name = "adn_array_get_ptr";
	}
	else
	{
		runtime_name = "adn_array_get_i64";
	}
	IRFunction* fn = ensure_runtime_function(program, runtime_name);
	IRValue* args[2] = {array, index};
	IRValue* value = ir_emit_call(current_block, fn, args, 2);
	if (element_type && strcmp(element_type, "f32") == 0)
	{
		return ir_emit_fpcvt(current_block, value, ir_type_f32());
	}
	return value;
}

IRValue* lower_expression(Program* program, ASTNode* node)
{
	switch (node->type)
	{
		case AST_IDENTIFIER:
		{
			const char* name = node->identifier.name;
			fprintf(stderr, "lower_expression: identifier '%s'\n", name);
			if (!name)
			{
				fprintf(stderr, "Encountered identifier with no name. (Error)\n");
				return NULL;
			}
			SymEntry* e = sym_get(name);
			if (e)
			{
				if (e->is_address)
				{
					if (!current_block)
					{
						fprintf(stderr,
						        "No current block for loading identifier "
						        "'%s'. (Error)\n",
						        name);
						return NULL;
					}
					return ir_emit_load(current_block, e->value);
				}
				else
				{
					return e->value;
				}
			}
			fprintf(stderr, "Unknown identifier '%s'. (Error)\n", name);
			return NULL;
		}

		case AST_CALL:
		{
			fprintf(stderr, "lower_expression: call to '%s' with %zu args\n",
			        node->call.callee ? node->call.callee : "<null>",
			        node->call.arg_count);
			if (!current_block)
			{
				fprintf(stderr,
				        "Attempt to lower call but no current block is set. "
				        "(Error)\n");
				return NULL;
			}

			const char* callee_name = node->call.callee;
			if (!callee_name)
			{
				fprintf(stderr, "Call with empty callee name. (Error)\n");
				return NULL;
			}

			size_t nargs = node->call.arg_count;
			ASTNode* callee_decl =
			    find_function_declaration(program->ast_root, callee_name);
			bool is_variadic_call = callee_decl && callee_decl->func_decl.is_variadic;
			size_t fixed_arg_count =
			    is_variadic_call ? callee_decl->func_decl.param_count : nargs;
			IRValue** args = NULL;
			if (nargs > 0)
			{
				args = (IRValue**)calloc(nargs, sizeof(IRValue*));
				if (!args)
				{
					fprintf(stderr,
					        "Out of memory allocating args array. "
					        "(Error)\n");
					return NULL;
				}
			}

			for (size_t i = 0; i < nargs; ++i)
			{
				ASTNode* a = node->call.args[i];
				args[i] = lower_expression(program, a);
			}

			IRValue** call_args = args;
			size_t call_arg_count = nargs;
			if (is_variadic_call)
			{
				call_arg_count = fixed_arg_count + 1;
				call_args = (IRValue**)calloc(call_arg_count, sizeof(IRValue*));
				if (!call_args)
				{
					free(args);
					fprintf(stderr,
					        "Out of memory allocating variadic call args. "
					        "(Error)\n");
					return NULL;
				}
				for (size_t i = 0; i < fixed_arg_count; i++)
				{
					call_args[i] = args ? args[i] : NULL;
				}
				call_args[fixed_arg_count] = lower_variadic_pack_array(
				    program, node->call.args, args, fixed_arg_count, nargs);
			}

			const char* member_method = member_method_name(callee_name);
			if (member_method)
			{
				const char* receiver_type =
				    nargs > 0 ? infer_expression_type(program, node->call.args[0]) : NULL;
				if (is_array_type_name(receiver_type) &&
				    is_array_member_method_name(member_method))
				{
					ASTNode wrapper = {.type = AST_CALL};
					wrapper.call.callee = (char*)array_member_runtime_name(member_method);
					wrapper.call.args = node->call.args;
					wrapper.call.arg_count = node->call.arg_count;
					IRValue* result = lower_array_method_call(program, &wrapper, args, nargs);
					if (call_args != args)
					{
						free(call_args);
					}
					free(args);
					return result;
				}
				callee_name = member_method;
			}

			if (starts_with(callee_name, "__array_"))
			{
				IRValue* result =
				    lower_array_method_call(program, node, args, nargs);
				if (call_args != args)
				{
					free(call_args);
				}
				free(args);
				return result;
			}
			if (strcmp(callee_name, "__string_format") == 0)
			{
				IRValue* result =
				    lower_string_method_call(program, node, args, nargs);
				if (call_args != args)
				{
					free(call_args);
				}
				free(args);
				return result;
			}

			IRFunction* callee = ensure_program_function(program, callee_name);
			if (!callee && starts_with(callee_name, "adn_"))
			{
				callee = ensure_runtime_function(program, callee_name);
			}

			if (!callee)
			{
				fprintf(stderr,
				        "Call to unknown function '%s'. Creating stub. "
				        "(Warning)\n",
				        callee_name);
				int already_adn = (strlen(callee_name) >= 4 &&
				                   strncmp(callee_name, "adn_", 4) == 0);
				size_t nlen = strlen(callee_name) + (already_adn ? 1 : 5);
				char* stub_name = (char*)malloc(nlen);
				if (!stub_name)
				{
					fprintf(stderr,
					        "Out of memory creating stub name. "
					        "(Error)\n");
					free(args);
					return NULL;
				}
				if (already_adn)
				{
					snprintf(stub_name, nlen, "%s", callee_name);
				}
				else
				{
					snprintf(stub_name, nlen, "%s", callee_name);
				}

				IRFunction* existing_stub =
				    find_ir_function(program->ir, stub_name);

				IRFunction* fn = existing_stub;
				if (!fn)
				{
					IRType* ret_t = NULL;
					const char* return_type =
					    infer_expression_type(program, node);
					if (strcmp(callee_name, "input") == 0 ||
					    strcmp(callee_name, "adn_input") == 0)
					{
						ret_t = ir_type_ptr(ir_type_i64());
					}
					else if (strcmp(callee_name, "println") == 0 ||
					         strcmp(callee_name, "adn_print") == 0 ||
					         strcmp(callee_name, "errorln") == 0 ||
					         strcmp(callee_name, "adn_error") == 0)
					{
						ret_t = ir_type_void();
					}
					else
					{
						ret_t = lower_type_name(return_type ? return_type
						                                    : "any");
					}

					fn = ir_function_create_in_module(program->ir, stub_name,
					                                  ret_t);
					if (fn)
					{
						for (size_t i = 0; i < call_arg_count; ++i)
						{
							IRValue* a = call_args[i];
							IRType* at =
							    a && a->type ? a->type : ir_type_i64();
							ir_param_create(fn, NULL, at);
						}
						callee = fn;
					}
					else
					{
						fprintf(stderr,
						        "Failed to create stub for '%s'. "
						        "(Error)\n",
						        callee_name);
						free(args);
						free(stub_name);
						return NULL;
					}
				}
				else
				{
					callee = fn;
				}
				free(stub_name);
			}

			IRValue* res =
			    ir_emit_call(current_block, callee, call_args, call_arg_count);
			if (call_args != args)
			{
				free(call_args);
			}
			free(args);
			return res;
		}

		case AST_STRING_LITERAL:
		{
			if (!program || !program->ir || !node->string_literal.value)
			{
				fprintf(stderr, "Invalid string literal node. (Error)\n");
				return NULL;
			}
			return ir_const_string(program->ir, node->string_literal.value);
		}

		case AST_NUMBER_LITERAL:
		{
			if (!node->number_literal.value)
			{
				fprintf(stderr, "Invalid number literal node. (Error)\n");
				return NULL;
			}
			if (strchr(node->number_literal.value, '.'))
			{
				double v = strtod(node->number_literal.value, NULL);
				return ir_const_f64(v);
			}
			long long v = strtoll(node->number_literal.value, NULL, 10);
			return ir_const_i64(v);
		}

		case AST_BOOLEAN_LITERAL:
		{
			return ir_const_i64(node->boolean_literal.value ? 1 : 0);
		}

		case AST_BINARY_OP:
		{
			if (!current_block)
			{
				fprintf(stderr, "No current block for binary op. (Error)\n");
				return NULL;
			}

			if (node->binary_op.op && strcmp(node->binary_op.op, "+") == 0 &&
			    node->binary_op.left &&
			    node->binary_op.left->type == AST_STRING_LITERAL &&
			    node->binary_op.right &&
			    node->binary_op.right->type == AST_STRING_LITERAL)
			{
				const char* ls = node->binary_op.left->string_literal.value;
				const char* rs = node->binary_op.right->string_literal.value;
				size_t ll = strlen(ls);
				size_t rl = strlen(rs);
				const char* raw_l =
				    (ll >= 2 && ((ls[0] == '"' && ls[ll - 1] == '"') ||
				                 (ls[0] == '`' && ls[ll - 1] == '`')))
				        ? ls + 1
				        : ls;
				size_t raw_ll = (ll >= 2 && ((ls[0] == '"' && ls[ll - 1] == '"') ||
				                             (ls[0] == '`' && ls[ll - 1] == '`')))
				                    ? ll - 2
				                    : ll;
				const char* raw_r =
				    (rl >= 2 && ((rs[0] == '"' && rs[rl - 1] == '"') ||
				                 (rs[0] == '`' && rs[rl - 1] == '`')))
				        ? rs + 1
				        : rs;
				size_t raw_rl = (rl >= 2 && ((rs[0] == '"' && rs[rl - 1] == '"') ||
				                             (rs[0] == '`' && rs[rl - 1] == '`')))
				                    ? rl - 2
				                    : rl;
				char* folded = (char*)malloc(raw_ll + raw_rl + 1);
				if (folded)
				{
					memcpy(folded, raw_l, raw_ll);
					memcpy(folded + raw_ll, raw_r, raw_rl);
					folded[raw_ll + raw_rl] = '\0';
					IRValue* result = ir_const_string(program->ir, folded);
					free(folded);
					if (result)
					{
						return result;
					}
				}
			}

			IRValue* lhs = lower_expression(program, node->binary_op.left);
			IRValue* rhs = lower_expression(program, node->binary_op.right);
			const char* left_type =
			    infer_expression_type(program, node->binary_op.left);
			const char* right_type =
			    infer_expression_type(program, node->binary_op.right);
			if (!lhs || !rhs)
			{
				fprintf(stderr,
				        "Failed to lower binary op operands. "
				        "(Error)\n");
				if (lhs && lhs->type && lhs->type->kind == IR_T_PTR)
					return ir_const_i64(0);
				if (rhs && rhs->type && rhs->type->kind == IR_T_PTR)
					return ir_const_i64(0);
				return NULL;
			}
			if (strcmp(node->binary_op.op, "+") == 0 &&
			    ((left_type && strcmp(left_type, "string") == 0) ||
			     (right_type && strcmp(right_type, "string") == 0)))
			{
				IRFunction* concat_fn =
				    ensure_runtime_function(program, "adn_strconcat");
				lhs = lower_string_conversion(program, node->binary_op.left, lhs,
				                              left_type);
				rhs = lower_string_conversion(program, node->binary_op.right, rhs,
				                              right_type);
				IRValue* cargs[2] = {lhs, rhs};
				if (lhs && rhs)
					return ir_emit_call(current_block, concat_fn, cargs, 2);
				else
					return ir_const_i64(0);
			}
			return ir_emit_binop(current_block, node->binary_op.op, lhs, rhs);
		}

		case AST_OBJECT_LITERAL:
			return lower_object_literal(program, node);

		case AST_ARRAY_LITERAL:
			return lower_array_literal(program, node);

		case AST_MEMBER_ACCESS:
			return lower_member_access(program, node);

		case AST_ARRAY_ACCESS:
			return lower_array_access(program, node);

		case AST_CAST:
		{
			if (!node->cast.target_type || !node->cast.expr)
			{
				fprintf(stderr, "Invalid cast node. (Error)\n");
				return NULL;
			}
			const char* target = node->cast.target_type->type_node.name;
			IRValue* inner = lower_expression(program, node->cast.expr);
			if (!inner || !target)
			{
				fprintf(stderr, "Failed to lower cast expression. (Error)\n");
				return NULL;
			}

			int src_is_ptr = (inner->type && inner->type->kind == IR_T_PTR);
			int src_is_float = (inner->type && (inner->type->kind == IR_T_F32 ||
			                                    inner->type->kind == IR_T_F64));
			int dst_is_string = (strcmp(target, "string") == 0);
			int dst_is_int =
			    (strcmp(target, "i32") == 0 || strcmp(target, "i64") == 0 ||
			     strcmp(target, "u32") == 0 || strcmp(target, "u64") == 0 ||
			     strcmp(target, "i8") == 0 || strcmp(target, "u8") == 0);
			int dst_is_float =
			    (strcmp(target, "f32") == 0 || strcmp(target, "f64") == 0);

			if (dst_is_string && !src_is_ptr)
			{
				if (src_is_float)
				{
					IRFunction* conv_fn = NULL;
					IRFunction* it = program->ir->functions;
					while (it)
					{
						if (it->name &&
						    strcmp(it->name, "adn_f64_to_string") == 0)
						{
							conv_fn = it;
							break;
						}
						it = it->next;
					}
					if (!conv_fn)
					{
						conv_fn = ir_function_create_in_module(
						    program->ir, "adn_f64_to_string",
						    ir_type_ptr(ir_type_i64()));
						ir_param_create(conv_fn, NULL, ir_type_f64());
					}
					IRValue* cargs[1] = {inner};
					if (inner->type->kind == IR_T_F32)
						inner = ir_emit_fpcvt(current_block, inner,
						                      ir_type_f64());
					return ir_emit_call(current_block, conv_fn, cargs, 1);
				}
				else
				{
					IRFunction* conv_fn = NULL;
					IRFunction* it = program->ir->functions;
					while (it)
					{
						if (it->name &&
						    strcmp(it->name, "adn_i32_to_string") == 0)
						{
							conv_fn = it;
							break;
						}
						it = it->next;
					}
					if (!conv_fn)
					{
						conv_fn = ir_function_create_in_module(
						    program->ir, "adn_i32_to_string",
						    ir_type_ptr(ir_type_i64()));
						ir_param_create(conv_fn, NULL, ir_type_i64());
					}
					IRValue* cargs[1] = {inner};
					return ir_emit_call(current_block, conv_fn, cargs, 1);
				}
			}
			else if (dst_is_int && src_is_ptr)
			{
				IRFunction* conv_fn = NULL;
				IRFunction* it = program->ir->functions;
				while (it)
				{
					if (it->name && strcmp(it->name, "adn_string_to_i32") == 0)
					{
						conv_fn = it;
						break;
					}
					it = it->next;
				}
				if (!conv_fn)
				{
					conv_fn = ir_function_create_in_module(
					    program->ir, "adn_string_to_i32", ir_type_i64());
					ir_param_create(conv_fn, NULL, ir_type_ptr(ir_type_i64()));
				}
				IRValue* cargs[1] = {inner};
				return ir_emit_call(current_block, conv_fn, cargs, 1);
			}
			else if (dst_is_float && src_is_ptr)
			{
				IRFunction* conv_fn = NULL;
				IRFunction* it = program->ir->functions;
				while (it)
				{
					if (it->name && strcmp(it->name, "adn_string_to_f64") == 0)
					{
						conv_fn = it;
						break;
					}
					it = it->next;
				}
				if (!conv_fn)
				{
					conv_fn = ir_function_create_in_module(
					    program->ir, "adn_string_to_f64", ir_type_f64());
					ir_param_create(conv_fn, NULL, ir_type_ptr(ir_type_i64()));
				}
				IRValue* cargs[1] = {inner};
				IRValue* res = ir_emit_call(current_block, conv_fn, cargs, 1);
				if (strcmp(target, "f32") == 0)
					return ir_emit_fpcvt(current_block, res, ir_type_f32());
				return res;
			}
			else if (dst_is_float && !src_is_float)
			{
				IRType* dt = NULL;
				if (strcmp(target, "f32") == 0)
					dt = ir_type_f32();
				else
					dt = ir_type_f64();
				return ir_emit_itofp(current_block, inner, dt);
			}
			else if (dst_is_float && src_is_float)
			{
				IRType* dt = NULL;
				if (strcmp(target, "f32") == 0)
					dt = ir_type_f32();
				else
					dt = ir_type_f64();
				if (inner->type->kind != dt->kind)
					return ir_emit_fpcvt(current_block, inner, dt);
				return inner;
			}
			else
			{
				return inner;
			}
		}

		default:
			fprintf(stderr,
			        "Unsupported AST node type in "
			        "lower_expression: %d. (Error)\n",
			        (int)node->type);
			return NULL;
	}
}

void lower_statement(Program* program, ASTNode* node)
{
	if (!program || !node)
	{
		fprintf(stderr, "Invalid arguments to lower_statement. (Error)\n");
		return;
	}

	fprintf(stderr, "lower_statement: node type=%d\n", (int)node->type);
	switch (node->type)
	{
		case AST_EXPRESSION_STATEMENT:
			lower_expression(program, node->expr_stmt.expr);
			break;
		case AST_RETURN_STATEMENT:
		{
			fprintf(stderr, "lower_statement: return\n");
			IRValue* ret_val = NULL;
			if (node->ret.expr)
			{
				ret_val = lower_expression(program, node->ret.expr);
			}
			if (current_block)
			{
				ir_emit_ret(current_block, ret_val);
			}
			else
			{
				fprintf(stderr,
				        "No current block to emit return statement "
				        "into. (Error)\n");
			}
			break;
		}
		case AST_BLOCK:
		{
			fprintf(stderr, "lower_statement: block with %zu statements\n",
			        node->block.count);
			for (size_t i = 0; i < node->block.count; i++)
			{
				if (block_is_terminated(current_block))
				{
					break;
				}
				lower_statement(program, node->block.statements[i]);
			}
			break;
		}
		case AST_TYPE_DECLARATION:
			break;
		case AST_IF_STATEMENT:
		{
			if (!current_block)
			{
				fprintf(stderr,
				        "No current block to emit if statement into. (Error)\n");
				return;
			}
			IRValue* cond = lower_expression(program, node->if_stmt.condition);
			IRBlock* then_b = ir_block_create_in_function(current_function, "then_b");
			IRBlock* else_b =
			    node->if_stmt.else_branch
			        ? ir_block_create_in_function(current_function, "else_b")
			        : NULL;
			IRBlock* merge_b = ir_block_create_in_function(current_function, "merge_b");

			ir_emit_cbr(current_block, cond, then_b, else_b ? else_b : merge_b);

			current_block = then_b;
			lower_statement(program, node->if_stmt.then_branch);
			if (current_block &&
			    (!current_block->last || (current_block->last->kind != IR_RET &&
			                              current_block->last->kind != IR_BR &&
			                              current_block->last->kind != IR_CBR)))
			{
				ir_emit_br(current_block, merge_b);
			}

			if (else_b)
			{
				current_block = else_b;
				lower_statement(program, node->if_stmt.else_branch);
				if (current_block &&
				    (!current_block->last || (current_block->last->kind != IR_RET &&
				                              current_block->last->kind != IR_BR &&
				                              current_block->last->kind != IR_CBR)))
				{
					ir_emit_br(current_block, merge_b);
				}
			}

			current_block = merge_b;
			break;
		}
		case AST_WHILE_STMT:
		{
			if (!current_block)
			{
				fprintf(stderr,
				        "No current block to emit while statement into. (Error)\n");
				return;
			}

			IRBlock* cond_b =
			    ir_block_create_in_function(current_function, "while_cond");
			IRBlock* body_b =
			    ir_block_create_in_function(current_function, "while_body");
			IRBlock* merge_b =
			    ir_block_create_in_function(current_function, "while_merge");

			ir_emit_br(current_block, cond_b);

			current_block = cond_b;
			IRValue* cond = lower_expression(program, node->while_stmt.condition);
			ir_emit_cbr(current_block, cond, body_b, merge_b);

			current_block = body_b;
			loop_break_targets[loop_target_depth] = merge_b;
			loop_continue_targets[loop_target_depth] = cond_b;
			loop_target_depth++;
			lower_statement(program, node->while_stmt.body);
			if (loop_target_depth > 0)
			{
				loop_target_depth--;
			}
			if (current_block &&
			    (!current_block->last || (current_block->last->kind != IR_RET &&
			                              current_block->last->kind != IR_BR &&
			                              current_block->last->kind != IR_CBR)))
			{
				ir_emit_br(current_block, cond_b);
			}

			current_block = merge_b;
			break;
		}
		case AST_FOR_STMT:
		{
			if (!current_block)
			{
				fprintf(stderr,
				        "No current block to emit for statement into. (Error)\n");
				return;
			}

			if (node->for_stmt.var_decl)
			{
				lower_statement(program, node->for_stmt.var_decl);
			}

			IRBlock* cond_b = ir_block_create_in_function(current_function, "for_cond");
			IRBlock* body_b = ir_block_create_in_function(current_function, "for_body");
			IRBlock* inc_b = ir_block_create_in_function(current_function, "for_inc");
			IRBlock* merge_b =
			    ir_block_create_in_function(current_function, "for_merge");

			ir_emit_br(current_block, cond_b);

			current_block = cond_b;
			IRValue* cond = lower_expression(program, node->for_stmt.condition);
			ir_emit_cbr(current_block, cond, body_b, merge_b);

			current_block = body_b;
			loop_break_targets[loop_target_depth] = merge_b;
			loop_continue_targets[loop_target_depth] = inc_b;
			loop_target_depth++;
			if (node->for_stmt.body)
			{
				lower_statement(program, node->for_stmt.body);
			}
			if (loop_target_depth > 0)
			{
				loop_target_depth--;
			}
			if (current_block &&
			    (!current_block->last || (current_block->last->kind != IR_RET &&
			                              current_block->last->kind != IR_BR &&
			                              current_block->last->kind != IR_CBR)))
			{
				ir_emit_br(current_block, inc_b);
			}

			current_block = inc_b;
			if (node->for_stmt.increment)
			{
				if (node->for_stmt.increment->type == AST_ASSIGNMENT ||
				    node->for_stmt.increment->type == AST_EXPRESSION_STATEMENT ||
				    node->for_stmt.increment->type == AST_CALL)
				{
					lower_statement(program, node->for_stmt.increment);
				}
				else
				{
					lower_expression(program, node->for_stmt.increment);
				}
			}
			ir_emit_br(current_block, cond_b);

			current_block = merge_b;
			break;
		}
		case AST_BREAK_STATEMENT:
		{
			if (!current_block || loop_target_depth == 0)
			{
				fprintf(stderr, "Break statement outside of loop. (Error)\n");
				return;
			}
			ir_emit_br(current_block, loop_break_targets[loop_target_depth - 1]);
			break;
		}
		case AST_CONTINUE_STATEMENT:
		{
			if (!current_block || loop_target_depth == 0)
			{
				fprintf(stderr, "Continue statement outside of loop. (Error)\n");
				return;
			}
			ir_emit_br(current_block, loop_continue_targets[loop_target_depth - 1]);
			break;
		}
		case AST_VARIABLE_DECLARATION:
		{
			fprintf(stderr, "lower_statement: var decl '%s'\n",
			        node->var_decl.name ? node->var_decl.name : "<anon>");
			if (!current_block)
			{
				fprintf(stderr,
				        "No current block to emit variable "
				        "declaration into. (Error)\n");
				return;
			}
			const char* var_name = node->var_decl.name;
			if (!var_name)
			{
				fprintf(stderr,
				        "Variable declaration with no name. "
				        "(Error)\n");
				return;
			}
			IRType* var_type = lower_type(program, node->var_decl.type);
			if (!var_type)
			{
				fprintf(stderr,
				        "Failed to resolve variable type for "
				        "'%s'. (Error)\n",
				        var_name);
				return;
			}
			IRBlock* entry_block_for_alloc = current_function->blocks;
			if (!entry_block_for_alloc)
			{
				entry_block_for_alloc =
				    ir_block_create_in_function(current_function, "entry");
			}
			IRValue* alloca = ir_emit_alloca(entry_block_for_alloc, var_type);
			sym_put(var_name,
			        node->var_decl.type ? node->var_decl.type->type_node.name : NULL,
			        alloca, 1);

			if (node->var_decl.initializer)
			{
				IRValue* init_val =
				    lower_expression(program, node->var_decl.initializer);
				if (init_val)
				{
					ir_emit_store(current_block, alloca, init_val);
				}
				else
				{
					fprintf(stderr,
					        "Failed to lower initializer for "
					        "variable '%s'. (Error)\n",
					        var_name);
					IRValue* def = ir_const_i64(0);
					ir_emit_store(current_block, alloca, def);
				}
			}
			else
			{
				IRValue* def = ir_const_i64(0);
				ir_emit_store(current_block, alloca, def);
			}
			IRValue* loaded = ir_emit_load(current_block, alloca);
			(void)loaded;
			break;
		}
		case AST_CALL:
		{
			lower_expression(program, node);
			break;
		}
		case AST_ASSIGNMENT:
		{
			fprintf(stderr, "lower_statement: assignment to '%s'\n",
			        node->assignment.name ? node->assignment.name : "<anon>");
			if (!current_block)
			{
				fprintf(stderr,
				        "No current block to emit assignment "
				        "into. (Error)\n");
				return;
			}
			const char* aname = node->assignment.name;
			if (!aname)
			{
				fprintf(stderr,
				        "Assignment with no variable name. "
				        "(Error)\n");
				return;
			}
			SymEntry* se = sym_get(aname);
			if (!se || !se->is_address)
			{
				fprintf(stderr,
				        "Assignment to unknown or "
				        "non-addressable variable '%s'. "
				        "(Error)\n",
				        aname);
				return;
			}
			IRValue* val = lower_expression(program, node->assignment.value);
			if (val)
			{
				ir_emit_store(current_block, se->value, val);
				sym_put(aname, se->type_name, se->is_address ? se->value : val,
				        se->is_address ? 1 : 0);
			}
			else
			{
				fprintf(stderr,
				        "Failed to lower assignment value for "
				        "'%s'. (Error)\n",
				        aname);
				IRValue* def = ir_const_i64(0);
				ir_emit_store(current_block, se->value, def);
				sym_put(aname, se->type_name, se->is_address ? se->value : def,
				        se->is_address ? 1 : 0);
			}
			break;
		}
		default:
			fprintf(stderr,
			        "Unsupported AST node type in "
			        "lower_statement: %d. (Error)\n",
			        (int)node->type);
			return;
	}
}

static IRType* lower_type(Program* program, ASTNode* node)
{
	if (!program || !node)
	{
		return NULL;
	}

	if (node->type != AST_TYPE)
	{
		fprintf(stderr, "lower_type called with non-type node. (Error)\n");
		return NULL;
	}

	const char* type_name = node->type_node.name;
	if (!type_name)
	{
		fprintf(stderr, "Type node with no name. (Error)\n");
		return NULL;
	}
	return lower_type_name(type_name);
}

static void emit_global_inits(IRBlock* block, ASTNode* root, Program* program,
                              IRFunction* init_func)
{
	for (size_t i = 0; i < root->program.count; ++i)
	{
		ASTNode* decl = root->program.decls[i];
		if (!decl)
			continue;
		if (decl->type == AST_VARIABLE_DECLARATION)
		{
			const char* var_name = decl->var_decl.name;
			if (!var_name)
				continue;
			SymEntry* se = sym_get(var_name);
			if (se && se->is_address)
			{
				IRValue* init_val = NULL;
				if (decl->var_decl.initializer)
				{
					current_function = init_func;
					current_block = block;
					init_val =
					    lower_expression(program, decl->var_decl.initializer);
					current_function = NULL;
					current_block = NULL;
				}
				if (!init_val)
				{
					init_val = ir_const_i64(0);
				}
				ir_emit_store(block, se->value, init_val);
			}
		}
	}
}

void lower_program(Program* program)
{
	if (!program)
	{
		fprintf(stderr, "lower_program called with NULL program. (Error)\n");
		return;
	}

	if (!program->ast_root)
	{
		fprintf(stderr, "Program has no AST root. (Error)\n");
		return;
	}

	if (program->ast_root->type != AST_PROGRAM)
	{
		fprintf(stderr, "AST root is not a program node. (Error)\n");
		return;
	}

	if (!program->ir)
	{
		fprintf(stderr, "Program has no IR module. (Error)\n");
		return;
	}

	sym_clear();
	loop_target_depth = 0;
	ASTNode* root = program->ast_root;
	ReachableFunctionName* reachable_functions = collect_reachable_functions(program);
	for (size_t i = 0; i < root->program.count; ++i)
	{
		ASTNode* decl = root->program.decls[i];
		if (!decl)
			continue;
		if (decl->type == AST_VARIABLE_DECLARATION)
		{
			const char* var_name = decl->var_decl.name;
			if (!var_name)
			{
				fprintf(stderr,
				        "Global variable with no name. "
				        "(Error)\n");
				continue;
			}
			IRType* var_type = lower_type(program, decl->var_decl.type);
			if (!var_type)
			{
				fprintf(stderr,
				        "Failed to resolve type for global "
				        "variable '%s'. (Error)\n",
				        var_name);
				continue;
			}
			IRValue* initial = NULL;
			if (decl->var_decl.initializer)
			{
				if (decl->var_decl.initializer->type == AST_STRING_LITERAL ||
				    decl->var_decl.initializer->type == AST_NUMBER_LITERAL ||
				    decl->var_decl.initializer->type == AST_BOOLEAN_LITERAL)
				{
					initial =
					    lower_expression(program, decl->var_decl.initializer);
				}
			}
			IRValue* global =
			    ir_global_create(program->ir, var_name, var_type, initial);
			if (global)
			{
				sym_put(var_name,
				        decl->var_decl.type ? decl->var_decl.type->type_node.name
				                            : NULL,
				        global, 1);
			}
			else
			{
				fprintf(stderr,
				        "Failed to create global variable "
				        "'%s'. (Error)\n",
				        var_name);
			}
		}
		else if (decl->type == AST_IMPORT_STATEMENT)
		{
			const char* import_path = decl->import.path;
			if (!import_path)
			{
				fprintf(stderr,
				        "Import statement with no path. "
				        "(Error)\n");
				continue;
			}
			fprintf(stderr, "Import: %s (Info)\n", import_path);
			if (strcmp(import_path, "adan/io") == 0)
			{
				int exists = 0;
				if (program->ir)
				{
					IRFunction* it = program->ir->functions;
					while (it)
					{
						if (it->name && strcmp(it->name, "println") == 0)
						{
							exists = 1;
							break;
						}
						it = it->next;
					}
				}
				if (!exists)
				{
					IRType* ret_t = ir_type_void();
					IRFunction* fn = ir_function_create_in_module(
					    program->ir, "println", ret_t);
					if (fn)
					{
						IRType* str_t = ir_type_ptr(ir_type_i64());
						ir_param_create(fn, "s", str_t);
					}
				}
			}
		}
	}

	IRFunction* init_func =
	    ir_function_create_in_module(program->ir, "__adan_init", ir_type_void());
	IRBlock* init_block = ir_block_create_in_function(init_func, "entry");
	emit_global_inits(init_block, root, program, init_func);
	for (size_t i = 0; i < root->program.count; ++i)
	{
		ASTNode* decl = root->program.decls[i];
		if (!decl)
			continue;
		if (decl->type == AST_ASSIGNMENT || decl->type == AST_EXPRESSION_STATEMENT ||
		    decl->type == AST_CALL)
		{
			current_function = init_func;
			current_block = init_block;
			lower_statement(program, decl);
			current_function = NULL;
			current_block = NULL;
		}
	}
	for (size_t i = 0; i < root->program.count; ++i)
	{
		ASTNode* decl = root->program.decls[i];
		if (!decl)
			continue;
		if (decl->type == AST_FUNCTION_DECLARATION)
		{
			const char* func_name = decl->func_decl.name;
			if (!func_name || !is_function_reachable(reachable_functions, func_name))
				continue;
			IRFunction* ir_func = NULL;
			IRBlock* entry_block = NULL;
			for (IRFunction* f = program->ir->functions; f; f = f->next)
			{
				if (f->name && strcmp(f->name, func_name) == 0)
				{
					ir_func = f;
					break;
				}
			}
			if (ir_func)
				entry_block = ir_func->blocks;
			if (entry_block)
				emit_global_inits(entry_block, root, program, init_func);
		}
	}

	for (size_t i = 0; i < root->program.count; ++i)
	{
		ASTNode* decl = root->program.decls[i];
		if (!decl)
			continue;
		if (decl->type == AST_FUNCTION_DECLARATION)
		{
			const char* func_name = decl->func_decl.name;
			if (!func_name)
			{
				fprintf(stderr,
				        "Function declaration with no name. "
				        "(Error)\n");
				continue;
			}
			if (!is_function_reachable(reachable_functions, func_name))
			{
				continue;
			}
			IRType* ret_type = lower_type(program, decl->func_decl.return_type);
			if (!ret_type)
			{
				fprintf(stderr,
				        "Failed to resolve return type for "
				        "function '%s'. (Error)\n",
				        func_name);
				continue;
			}
			IRFunction* ir_func = find_ir_function(program->ir, func_name);
			if (!ir_func)
			{
				ir_func =
				    ir_function_create_in_module(program->ir, func_name, ret_type);
			}
			if (!ir_func)
			{
				fprintf(stderr,
				        "Failed to create IR function '%s'. "
				        "(Error)\n",
				        func_name);
				continue;
			}
			current_function = ir_func;
			IRBlock* entry_block = ir_block_create_in_function(ir_func, "entry");
			if (!entry_block)
			{
				fprintf(stderr,
				        "Failed to create entry block for "
				        "function '%s'. (Error)\n",
				        func_name);
				current_function = NULL;
				continue;
			}
			if (strcmp(func_name, "main") == 0)
			{
				ir_emit_call(entry_block, init_func, NULL, 0);
			}
			current_block = entry_block;
			IRValue* existing_param = ir_func->params;
			for (size_t j = 0; j < decl->func_decl.param_count; j++)
			{
				ASTNode* param_node = decl->func_decl.params[j];
				if (!param_node)
					continue;
				const char* param_name = param_node->param.name;
				IRType* param_type = lower_type(program, param_node->param.type);
				if (!param_name || !param_type)
				{
					fprintf(stderr,
					        "Failed to lower parameter "
					        "%zu for function '%s'. "
					        "(Error)\n",
					        j, func_name);
					continue;
				}
				IRValue* param_val = existing_param;
				if (!param_val)
				{
					param_val =
					    ir_param_create(ir_func, param_name, param_type);
				}
				if (existing_param)
				{
					existing_param = existing_param->next;
				}
				if (param_val)
				{
					IRBlock* entry_alloc_block = ir_func->blocks;
					if (!entry_alloc_block)
					{
						entry_alloc_block =
						    ir_block_create_in_function(ir_func, "entry");
					}
					IRValue* alloca =
					    ir_emit_alloca(entry_alloc_block, param_type);
					if (alloca)
					{
						ir_emit_store(entry_alloc_block, alloca, param_val);
						sym_put(param_name,
						        param_node->param.type
						            ? param_node->param.type->type_node.name
						            : NULL,
						        alloca, 1);
					}
					else
					{
						sym_put(param_name,
						        param_node->param.type
						            ? param_node->param.type->type_node.name
						            : NULL,
						        param_val, 0);
					}
				}
			}
			if (decl->func_decl.is_variadic && decl->func_decl.variadic_name)
			{
				const char* variadic_array_type = build_array_type_name(
				    decl->func_decl.variadic_type
				        ? decl->func_decl.variadic_type->type_node.name
				        : "any");
				IRType* variadic_type = lower_type_name(variadic_array_type);
				IRValue* variadic_param = existing_param;
				if (!variadic_param)
				{
					variadic_param = ir_param_create(
					    ir_func, decl->func_decl.variadic_name, variadic_type);
				}
				if (existing_param)
				{
					existing_param = existing_param->next;
				}
				if (variadic_param)
				{
					IRBlock* entry_alloc_block = ir_func->blocks;
					if (!entry_alloc_block)
					{
						entry_alloc_block =
						    ir_block_create_in_function(ir_func, "entry");
					}
					IRValue* alloca =
					    ir_emit_alloca(entry_alloc_block, variadic_type);
					if (alloca)
					{
						ir_emit_store(entry_alloc_block, alloca,
						              variadic_param);
						sym_put(decl->func_decl.variadic_name,
						        variadic_array_type, alloca, 1);
					}
					else
					{
						sym_put(decl->func_decl.variadic_name,
						        variadic_array_type, variadic_param, 0);
					}
				}
			}
			if (decl->func_decl.body)
			{
				lower_statement(program, decl->func_decl.body);
			}
			if (current_block)
			{
				if (!current_block->last || current_block->last->kind != IR_RET)
				{
					ir_emit_ret(current_block, NULL);
				}
			}
			current_function = NULL;
			current_block = NULL;
		}
	}

	free_reachable_functions(reachable_functions);
	sym_clear();
}

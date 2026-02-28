#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lower.h"
#include "ir/ir.h"
#include "../stm.h"

static IRFunction* current_function = NULL;

static IRType* lower_type(Program* program, ASTNode* node);

static IRBlock* current_block = NULL;

// Helper functions

static SymEntry* sym_table = NULL;

static void sym_put(const char* name, IRValue* v, int is_addr)
{
	if (!name || !v)
	{
		return;
	}
	SymEntry* e = (SymEntry*)malloc(sizeof(SymEntry));
	if (!e)
	{
		return;
	}
	e->name = strdup(name);
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
		free(it);
		it = nxt;
	}
	sym_table = NULL;
}

IRValue* lower_expression(Program* program, ASTNode* node)
{
	if (!program || !node)
	{
		fprintf(stderr, "Invalid arguments to lower_expression. (Error)\n");
		return NULL;
	}

	fprintf(stderr, "lower_expression: node type=%d\n", (int)node->type);
	switch (node->type)
	{
		case AST_NUMBER_LITERAL:
		{
			const char* s = node->number_literal.value;
			if (!s)
			{
				fprintf(stderr, "Empty numeric literal. (Error)\n");
				return NULL;
			}
			long long v = strtoll(s, NULL, 0);
			return ir_const_i64((int64_t)v);
		}
		case AST_STRING_LITERAL:
		{
			const char* s = node->string_literal.value;
			fprintf(stderr, "lower_expression: string literal '%s'\n", s);
			if (!s)
			{
				fprintf(stderr, "Empty string literal. (Error)\n");
				return NULL;
			}
			IRValue* str_val = ir_const_string(program->ir, s);
			return str_val;
		}
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
				return e->value;
			}

			SymbolEntry* se = NULL;
			if (program)
			{
				if (program->ir)
				{
					IRFunction* it = program->ir->functions;
					while (it)
					{
						if (it->name && strcmp(it->name, name) == 0)
						{
							return NULL;
						}
						it = it->next;
					}
				}
			}

			if (!current_function)
			{
				fprintf(stderr,
				        "No current function while resolving identifier '%s'. "
				        "(Error)\n",
				        name);
				return NULL;
			}

			IRBlock* entry_block = current_function->blocks;
			if (!entry_block)
			{
				entry_block =
				    ir_block_create_in_function(current_function, "entry");
			}
			IRType* ty = ir_type_i64();
			IRValue* alloca = ir_emit_alloca(entry_block, ty);
			sym_put(name, alloca, 1);

			if (!current_block)
			{
				fprintf(stderr,
				        "No current block for loading freshly created alloca '%s'. "
				        "(Error)\n",
				        name);
				return NULL;
			}
			return ir_emit_load(current_block, alloca);
		}

		case AST_CALL:
		{
			fprintf(stderr, "lower_expression: call to '%s' with %zu args\n", node->call.callee ? node->call.callee : "<null>", node->call.arg_count);
			if (!current_block)
			{
				fprintf(
				    stderr,
				    "Attempt to lower call but no current block is set. (Error)\n");
				return NULL;
			}

			const char* callee_name = node->call.callee;
			if (!callee_name)
			{
				fprintf(stderr, "Call with empty callee name. (Error)\n");
				return NULL;
			}

			IRFunction* callee = NULL;
			if (program->ir)
			{
				IRFunction* it = program->ir->functions;
				while (it)
				{
					if (it->name && strcmp(it->name, callee_name) == 0)
					{
						callee = it;
						break;
					}
					it = it->next;
				}
			}

			if (!callee)
			{
				fprintf(stderr, "Call to unknown function '%s'. (Warning)\n",
				        callee_name);
				return NULL;
			}

			size_t nargs = node->call.arg_count;
			IRValue** args = NULL;
			if (nargs > 0)
			{
				args = (IRValue**)calloc(nargs, sizeof(IRValue*));
				if (!args)
				{
					fprintf(
					    stderr,
					    "Out of memory while lowering call args. (Error)\n");
					return NULL;
				}
			}

			for (size_t i = 0; i < nargs; ++i)
			{
				ASTNode* a = node->call.args[i];
				args[i] = lower_expression(program, a);
			}

			IRValue* res = ir_emit_call(current_block, callee, args, nargs);

			free(args);
			return res;
		}

		default:
			fprintf(stderr,
			        "Unsupported AST node type in lower_expression: %d. (Error)\n",
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
				fprintf(
				    stderr,
				    "No current block to emit return statement into. (Error)\n");
			}
			break;
		}
		case AST_BLOCK:
		{
			fprintf(stderr, "lower_statement: block with %zu statements\n", node->block.count);
			for (size_t i = 0; i < node->block.count; i++)
			{
				lower_statement(program, node->block.statements[i]);
			}
			break;
		}
		case AST_VARIABLE_DECLARATION:
		{
			fprintf(stderr, "lower_statement: var decl '%s'\n", node->var_decl.name ? node->var_decl.name : "<anon>");
			if (!current_block)
			{
				fprintf(stderr,
				        "No current block to emit variable declaration into. "
				        "(Error)\n");
				return;
			}
			const char* var_name = node->var_decl.name;
			if (!var_name)
			{
				fprintf(stderr, "Variable declaration with no name. (Error)\n");
				return;
			}
			IRType* var_type = lower_type(program, node->var_decl.type);
			if (!var_type)
			{
				fprintf(stderr,
				        "Failed to resolve variable type for '%s'. (Error)\n",
				        var_name);
				return;
			}
			IRBlock* entry_block_for_alloc = current_function->blocks;
			if (!entry_block_for_alloc)
				entry_block_for_alloc = ir_block_create_in_function(current_function, "entry");
			IRValue* alloca = ir_emit_alloca(entry_block_for_alloc, var_type);
			sym_put(var_name, alloca, 1);

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
					        "Failed to lower initializer for variable '%s'. "
					        "(Error)\n",
					        var_name);
				}
			}
			break;
		}
		case AST_CALL:
		{
			lower_expression(program, node);
			break;
		}
		default:
			fprintf(stderr,
			        "Unsupported AST node type in lower_statement: %d. (Error)\n",
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

	if (strcmp(type_name, "i64") == 0 || strcmp(type_name, "i32") == 0 ||
	    strcmp(type_name, "u32") == 0 || strcmp(type_name, "u64") == 0)
	{
		return ir_type_i64();
	}
	if (strcmp(type_name, "f64") == 0)
	{
		return ir_type_f64();
	}
	if (strcmp(type_name, "void") == 0)
	{
		return ir_type_void();
	}
	if (strcmp(type_name, "string") == 0)
	{
		return ir_type_ptr(ir_type_i64());
	}

	fprintf(stderr, "Unknown type: %s. (Error)\n", type_name);
	return NULL;
}

// Main lowering function

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
	current_function = NULL;
	current_block = NULL;

	ASTNode* root = program->ast_root;
	for (size_t i = 0; i < root->program.count; i++)
	{
		ASTNode* decl = root->program.decls[i];
		fprintf(stderr, "lower_program: processing decl %zu type=%d\n", i, decl ? (int)decl->type : -1);
		if (!decl)
		{
			continue;
		}

		switch (decl->type)
		{
			case AST_FUNCTION_DECLARATION:
			{
				const char* func_name = decl->func_decl.name;
				if (!func_name)
				{
					fprintf(stderr,
					        "Function declaration with no name. (Error)\n");
					break;
				}

				IRType* ret_type = lower_type(program, decl->func_decl.return_type);
				if (!ret_type)
				{
					fprintf(stderr,
					        "Failed to resolve return type for function '%s'. "
					        "(Error)\n",
					        func_name);
					break;
				}

				IRFunction* ir_func =
				    ir_function_create_in_module(program->ir, func_name, ret_type);
				if (!ir_func)
				{
					fprintf(stderr,
					        "Failed to create IR function '%s'. (Error)\n",
					        func_name);
					break;
				}

				current_function = ir_func;

				IRBlock* entry_block =
				    ir_block_create_in_function(ir_func, "entry");
				if (!entry_block)
				{
					fprintf(stderr,
					        "Failed to create entry block for function '%s'. "
					        "(Error)\n",
					        func_name);
					current_function = NULL;
					break;
				}

				current_block = entry_block;

				for (size_t j = 0; j < decl->func_decl.param_count; j++)
				{
					ASTNode* param_node = decl->func_decl.params[j];
					if (!param_node)
					{
						continue;
					}

					const char* param_name = param_node->param.name;
					IRType* param_type =
					    lower_type(program, param_node->param.type);
					if (!param_name || !param_type)
					{
						fprintf(stderr,
						        "Failed to lower parameter %zu for "
						        "function '%s'. "
						        "(Error)\n",
						        j, func_name);
						continue;
					}

					IRValue* param_val = ir_param_create(ir_func, param_name, param_type);
					if (param_val)
					{
						IRBlock* entry_alloc_block = ir_func->blocks;
						if (!entry_alloc_block)
							entry_alloc_block = ir_block_create_in_function(ir_func, "entry");
						IRValue* alloca = ir_emit_alloca(entry_alloc_block, param_type);
						if (alloca)
						{
							ir_emit_store(entry_alloc_block, alloca, param_val);
							sym_put(param_name, alloca, 1);
						}
						else
						{
							sym_put(param_name, param_val, 0);
						}
					}
				}

				if (decl->func_decl.body)
				{
					lower_statement(program, decl->func_decl.body);
				}

				if (current_block)
				{
					if (!current_block->last ||
					    current_block->last->kind != IR_RET)
					{
						ir_emit_ret(current_block, NULL);
					}
				}

				current_function = NULL;
				current_block = NULL;

				break;
			}

			case AST_VARIABLE_DECLARATION:
			{
				const char* var_name = decl->var_decl.name;
				if (!var_name)
				{
					fprintf(stderr, "Global variable with no name. (Error)\n");
					break;
				}

				IRType* var_type = lower_type(program, decl->var_decl.type);
				if (!var_type)
				{
					fprintf(stderr,
					        "Failed to resolve type for global variable '%s'. "
					        "(Error)\n",
					        var_name);
					break;
				}

				IRValue* initial = NULL;
				if (decl->var_decl.initializer)
				{
					initial =
					    lower_expression(program, decl->var_decl.initializer);
				}

				IRValue* global =
				    ir_global_create(program->ir, var_name, var_type, initial);
				if (global)
				{
					sym_put(var_name, global, 1);
				}
				else
				{
					fprintf(stderr,
					        "Failed to create global variable '%s'. (Error)\n",
					        var_name);
				}

				break;
			}

			case AST_IMPORT_STATEMENT:
			{
				const char* import_path = decl->import.path;
				if (!import_path)
				{
					fprintf(stderr, "Import statement with no path. (Error)\n");
					break;
				}

				fprintf(stderr, "Import: %s (Info)\n", import_path);

				/* Create known stubs for standard library imports so calls lower
				 * correctly. */
				if (strcmp(import_path, "adan/io") == 0)
				{
					/* Only create stub if it does not already exist in the
					 * module. */
					int exists = 0;
					if (program->ir)
					{
						IRFunction* it = program->ir->functions;
						while (it)
						{
							if (it->name &&
							    strcmp(it->name, "println") == 0)
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
				break;
			}

			default:
				fprintf(stderr,
				        "Unexpected top-level declaration type: %d. (Error)\n",
				        (int)decl->type);
				break;
		}
	}

	sym_clear();
}
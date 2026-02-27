#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lower.h"
#include "ir/ir.h"
#include "../stm.h"

static IRFunction* current_function = NULL;

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

		case AST_IDENTIFIER:
		{
			const char* name = node->identifier.name;
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
	return;
}

IRType* lower_type(Program* program, ASTNode* node)
{
	return NULL;
}

// Main lowering function

void lower_program(Program* program)
{
	return;
}
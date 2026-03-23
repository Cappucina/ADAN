#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lower.h"
#include "ir/ir.h"

static IRFunction* current_function = NULL;

static IRType* lower_type(Program* program, ASTNode* node);

static IRBlock* current_block = NULL;

static SymEntry* sym_table = NULL;

static void emit_global_inits(IRBlock* block, ASTNode* root, Program* program,
                              IRFunction* init_func);

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
					snprintf(stub_name, nlen, "adn_%s", callee_name);
				}

				IRFunction* existing_stub = NULL;
				if (program->ir)
				{
					IRFunction* it = program->ir->functions;
					while (it)
					{
						if (it->name && strcmp(it->name, stub_name) == 0)
						{
							existing_stub = it;
							break;
						}
						it = it->next;
					}
				}

				IRFunction* fn = existing_stub;
				if (!fn)
				{
					IRType* ret_t = NULL;
					if (strcmp(callee_name, "input") == 0 ||
					    strcmp(callee_name, "adn_input") == 0)
					{
						ret_t = ir_type_ptr(ir_type_i64());
					}
					else if (strcmp(callee_name, "println") == 0 ||
					         strcmp(callee_name, "adn_println") == 0 ||
					         strcmp(callee_name, "errorln") == 0 ||
					         strcmp(callee_name, "adn_errorln") == 0)
					{
						ret_t = ir_type_void();
					}
					else
					{
						ret_t = ir_type_ptr(ir_type_i64());
					}

					fn = ir_function_create_in_module(program->ir, stub_name,
					                                  ret_t);
					if (fn)
					{
						for (size_t i = 0; i < nargs; ++i)
						{
							IRValue* a = args[i];
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

			IRValue* res = ir_emit_call(current_block, callee, args, nargs);
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
				    (ll >= 2 && ((ls[0] == '"' && ls[ll - 1] == '"') || (ls[0] == '`' && ls[ll - 1] == '`'))) ? ls + 1 : ls;
				size_t raw_ll =
				    (ll >= 2 && ((ls[0] == '"' && ls[ll - 1] == '"') || (ls[0] == '`' && ls[ll - 1] == '`'))) ? ll - 2 : ll;
				const char* raw_r =
				    (rl >= 2 && ((rs[0] == '"' && rs[rl - 1] == '"') || (rs[0] == '`' && rs[rl - 1] == '`'))) ? rs + 1 : rs;
				size_t raw_rl =
				    (rl >= 2 && ((rs[0] == '"' && rs[rl - 1] == '"') || (rs[0] == '`' && rs[rl - 1] == '`'))) ? rl - 2 : rl;
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
			if (strcmp(node->binary_op.op, "+") == 0 && lhs->type &&
			    lhs->type->kind == IR_T_PTR && rhs->type && rhs->type->kind == IR_T_PTR)
			{
				IRFunction* concat_fn = NULL;
				IRFunction* it = program->ir->functions;
				while (it)
				{
					if (it->name && strcmp(it->name, "adn_strconcat") == 0)
					{
						concat_fn = it;
						break;
					}
					it = it->next;
				}
				if (!concat_fn)
				{
					concat_fn = ir_function_create_in_module(
					    program->ir, "adn_strconcat",
					    ir_type_ptr(ir_type_i64()));
					ir_param_create(concat_fn, NULL,
					                ir_type_ptr(ir_type_i64()));
					ir_param_create(concat_fn, NULL,
					                ir_type_ptr(ir_type_i64()));
				}
				IRValue* cargs[2] = {lhs, rhs};
				if (lhs && rhs)
					return ir_emit_call(current_block, concat_fn, cargs, 2);
				else
					return ir_const_i64(0);
			}
			return ir_emit_binop(current_block, node->binary_op.op, lhs, rhs);
		}

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
			int dst_is_string = (strcmp(target, "string") == 0);
			int dst_is_int =
			    (strcmp(target, "i32") == 0 || strcmp(target, "i64") == 0 ||
			     strcmp(target, "u32") == 0 || strcmp(target, "u64") == 0);

			if (dst_is_string && !src_is_ptr)
			{
				// int -> string: call adn_i32_to_string
				IRFunction* conv_fn = NULL;
				IRFunction* it = program->ir->functions;
				while (it)
				{
					if (it->name && strcmp(it->name, "adn_i32_to_string") == 0)
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
				lower_statement(program, node->block.statements[i]);
			}
			break;
		}
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
			if (current_block && (!current_block->last || (current_block->last->kind != IR_RET && current_block->last->kind != IR_BR && current_block->last->kind != IR_CBR)))
			{
				ir_emit_br(current_block, merge_b);
			}

			if (else_b)
			{
				current_block = else_b;
				lower_statement(program, node->if_stmt.else_branch);
				if (current_block && (!current_block->last || (current_block->last->kind != IR_RET && current_block->last->kind != IR_BR && current_block->last->kind != IR_CBR)))
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

			IRBlock* cond_b = ir_block_create_in_function(current_function, "while_cond");
			IRBlock* body_b = ir_block_create_in_function(current_function, "while_body");
			IRBlock* merge_b = ir_block_create_in_function(current_function, "while_merge");

			ir_emit_br(current_block, cond_b);

			current_block = cond_b;
			IRValue* cond = lower_expression(program, node->while_stmt.condition);
			ir_emit_cbr(current_block, cond, body_b, merge_b);

			current_block = body_b;
			lower_statement(program, node->while_stmt.body);
			if (current_block && (!current_block->last || (current_block->last->kind != IR_RET && current_block->last->kind != IR_BR && current_block->last->kind != IR_CBR)))
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
			IRBlock* inc_b  = ir_block_create_in_function(current_function, "for_inc");
			IRBlock* merge_b = ir_block_create_in_function(current_function, "for_merge");

			ir_emit_br(current_block, cond_b);

			current_block = cond_b;
			IRValue* cond = lower_expression(program, node->for_stmt.condition);
			ir_emit_cbr(current_block, cond, body_b, merge_b);

			current_block = body_b;
			if (node->for_stmt.body)
			{
				lower_statement(program, node->for_stmt.body);
			}
			if (current_block && (!current_block->last || (current_block->last->kind != IR_RET && current_block->last->kind != IR_BR && current_block->last->kind != IR_CBR)))
			{
				ir_emit_br(current_block, inc_b);
			}

			current_block = inc_b;
			if (node->for_stmt.increment)
			{
				if (node->for_stmt.increment->type == AST_ASSIGNMENT || node->for_stmt.increment->type == AST_EXPRESSION_STATEMENT || node->for_stmt.increment->type == AST_CALL)
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
				if (se->is_address)
				{
					sym_put(aname, se->value, 1);
				}
				else
				{
					sym_put(aname, val, 0);
				}
			}
			else
			{
				fprintf(stderr,
				        "Failed to lower assignment value for "
				        "'%s'. (Error)\n",
				        aname);
				IRValue* def = ir_const_i64(0);
				ir_emit_store(current_block, se->value, def);
				if (se->is_address)
				{
					sym_put(aname, se->value, 1);
				}
				else
				{
					sym_put(aname, def, 0);
				}
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
	if (strcmp(type_name, "string") == 0)
	{
		return ir_type_ptr(ir_type_i64());
	}

	fprintf(stderr, "Unknown type: %s. (Error)\n", type_name);
	return NULL;
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
	ASTNode* root = program->ast_root;
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
				initial = lower_expression(program, decl->var_decl.initializer);
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
			if (!func_name)
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
			IRType* ret_type = lower_type(program, decl->func_decl.return_type);
			if (!ret_type)
			{
				fprintf(stderr,
				        "Failed to resolve return type for "
				        "function '%s'. (Error)\n",
				        func_name);
				continue;
			}
			IRFunction* ir_func =
			    ir_function_create_in_module(program->ir, func_name, ret_type);
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
				IRValue* param_val =
				    ir_param_create(ir_func, param_name, param_type);
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
				if (!current_block->last || current_block->last->kind != IR_RET)
				{
					ir_emit_ret(current_block, NULL);
				}
			}
			current_function = NULL;
			current_block = NULL;
		}
	}

	sym_clear();
}

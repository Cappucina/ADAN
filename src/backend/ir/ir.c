#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir.h"

IRModule* ir_module_create()
{
	IRModule* mod = malloc(sizeof(IRModule));
	if (!mod)
	{
		fprintf(stderr, "Failed to allocate IRModule. (Error)\n");
		return NULL;
	}
	mod->functions = NULL;
	mod->globals = NULL;
	fprintf(stderr, "IRModule created successfully. (Info)\n");
	return mod;
}

void ir_module_destroy(IRModule* mod)
{
	if (!mod)
	{
		fprintf(stderr, "Attempted to destroy a NULL IRModule. (Warning)\n");
		return;
	}
	fprintf(stderr, "ir_module_destroy: start (mod=%p)\n", (void*)mod);
	/* Conservative destructor: many IR objects (IRValue, IRInstruction,
	 * IRBlock, IRFunction) are referenced in multiple places and ownership
	 * is not fully tracked. Freeing them aggressively previously caused
	 * double-free / corruption. To keep teardown safe, only free owned
	 * heap allocations we can guarantee: call-arg arrays attached to
	 * instructions and module-level global nodes and their string data.
	 * This intentionally leaves IRFunction/IRBlock/IRInstruction/IRValue
	 * structs alive (minor leak) but avoids crashes while we stabilize
	 * ownership semantics.
	 */

	IRFunction* f = mod->functions;
	while (f)
	{
		fprintf(stderr, "ir_module_destroy: function %p name=%p\n", (void*)f, (void*)f->name);
		IRBlock* b = f->blocks;
		while (b)
		{
			fprintf(stderr, "ir_module_destroy: block %p name=%p\n", (void*)b, (void*)b->name);
			IRInstruction* i = b->first;
			while (i)
			{
				if (i->call_args)
				{
					fprintf(stderr, "ir_module_destroy: free call_args for instr %p\n", (void*)i);
					free(i->call_args);
					i->call_args = NULL;
					i->call_nargs = 0;
				}
				i = i->next;
			}
			b = b->next;
		}
		f = f->next;
	}

	/* free module-level globals and their associated string data */
	fprintf(stderr, "ir_module_destroy: free globals start (mod->globals=%p)\n", (void*)mod->globals);
	IRGlobal* gg = mod->globals;
	while (gg)
	{
		fprintf(stderr, "ir_module_destroy: freeing global %p name=%p\n", (void*)gg, (void*)gg->name);
		IRGlobal* gnext = gg->next;
		if (gg->value)
		{
			/* If the global value holds a strdup'd string in u.i64, free it. */
			if (gg->value->kind == IRV_GLOBAL && gg->value->u.i64)
			{
				const char* s = (const char*)(intptr_t)gg->value->u.i64;
				fprintf(stderr, "ir_module_destroy: global value string ptr=%p\n", (void*)s);
				if (s)
					free((void*)s);
			}
		}
		if (gg->name)
			free(gg->name);
		free(gg);
		gg = gnext;
	}

	/* Finally free the module container only. */
	fprintf(stderr, "ir_module_destroy: free(mod=%p)\n", (void*)mod);
	free(mod);
}

void ir_module_add_function(IRModule* mod, IRFunction* func)
{
	if (!mod || !func)
	{
		fprintf(stderr, "Invalid arguments to ir_module_add_function. (Error)\n");
		return;
	}
	func->next = NULL;
	if (!mod->functions)
	{
		mod->functions = func;
		return;
	}
	IRFunction* it = mod->functions;
	while (it->next)
		it = it->next;
	it->next = func;
	fprintf(stderr, "Function '%s' added to module. (Info)\n",
	        func->name ? func->name : "<anon>");
}

void ir_function_add_block(IRFunction* func, IRBlock* block)
{
	if (!func || !block)
	{
		fprintf(stderr, "Invalid arguments to ir_function_add_block. (Error)\n");
		return;
	}
	block->next = NULL;
	if (!func->blocks)
	{
		func->blocks = block;
		return;
	}
	IRBlock* it = func->blocks;
	while (it->next)
		it = it->next;
	it->next = block;
	fprintf(stderr, "Block '%s' added to function '%s'. (Info)\n",
	        block->name ? block->name : "<anon>", func->name ? func->name : "<anon>");
}

void ir_print_module(IRModule* mod, FILE* out)
{
	if (!mod || !out)
	{
		fprintf(stderr, "Invalid arguments to ir_print_module. (Error)\n");
		return;
	}
	IRFunction* f = mod->functions;
	while (f)
	{
		fprintf(out, "function %s\n", f->name);
		IRBlock* b = f->blocks;
		while (b)
		{
			fprintf(out, "  block %s:\n", b->name);
			IRInstruction* ins = b->first;
			while (ins)
			{
				switch (ins->kind)
				{
					case IR_BINOP:
						fprintf(out, "    binop\n");
						break;
					case IR_RET:
						fprintf(out, "    ret\n");
						break;
					default:
						fprintf(out, "    instr(kind=%d)\n", ins->kind);
						break;
				}
				ins = ins->next;
			}
			b = b->next;
		}
		f = f->next;
	}
	fprintf(stderr, "Module printed to output. (Info)\n");
}

IRType* ir_type_i64(void)
{
	IRType* t = malloc(sizeof(IRType));
	if (!t)
	{
		fprintf(stderr, "Failed to allocate IRType (i64). (Error)\n");
		return NULL;
	}
	t->kind = IR_T_I64;
	t->pointee = NULL;
	fprintf(stderr, "IRType i64 created. (Info)\n");
	return t;
}

IRType* ir_type_f64(void)
{
	IRType* t = malloc(sizeof(IRType));
	if (!t)
	{
		fprintf(stderr, "Failed to allocate IRType (f64). (Error)\n");
		return NULL;
	}
	t->kind = IR_T_F64;
	t->pointee = NULL;
	fprintf(stderr, "IRType f64 created. (Info)\n");
	return t;
}

IRType* ir_type_void(void)
{
	IRType* t = malloc(sizeof(IRType));
	if (!t)
	{
		fprintf(stderr, "Failed to allocate IRType (void). (Error)\n");
		return NULL;
	}
	t->kind = IR_T_VOID;
	t->pointee = NULL;
	fprintf(stderr, "IRType void created. (Info)\n");
	return t;
}

IRType* ir_type_ptr(IRType* pointee)
{
	IRType* t = malloc(sizeof(IRType));
	if (!t)
	{
		fprintf(stderr, "Failed to allocate IRType (ptr). (Error)\n");
		return NULL;
	}
	t->kind = IR_T_PTR;
	t->pointee = pointee;
	fprintf(stderr, "IRType ptr created. (Info)\n");
	return t;
}

IRFunction* ir_function_create(const char* name, IRType* return_type)
{
	if (!name)
	{
		fprintf(stderr, "ir_function_create called without a name. (Error)\n");
		return NULL;
	}
	IRFunction* f = malloc(sizeof(IRFunction));
	if (!f)
	{
		fprintf(stderr, "Failed to allocate IRFunction. (Error)\n");
		return NULL;
	}
	f->name = strdup(name);
	f->return_type = return_type;
	f->blocks = NULL;
	f->params = NULL;
	f->next = NULL;
	fprintf(stderr, "IRFunction '%s' created. (Info)\n", name);
	return f;
}

IRBlock* ir_block_create(const char* name)
{
	if (!name)
	{
		fprintf(stderr, "ir_block_create called without a name. (Error)\n");
		return NULL;
	}
	IRBlock* b = malloc(sizeof(IRBlock));
	if (!b)
	{
		fprintf(stderr, "Failed to allocate IRBlock. (Error)\n");
		return NULL;
	}
	b->name = strdup(name);
	b->first = NULL;
	b->last = NULL;
	b->next = NULL;
	fprintf(stderr, "IRBlock '%s' created. (Info)\n", name);
	return b;
}

IRValue* ir_emit_binop(IRBlock* block, const char* op, IRValue* lhs, IRValue* rhs)
{
	if (!block || !lhs || !rhs)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_binop. (Error)\n");
		return NULL;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	if (!ins)
		return NULL;
	ins->kind = IR_BINOP;
	IRValue* dst = ir_temp(block, lhs->type);
	ins->dest = dst;
	ins->operands[0] = lhs;
	ins->operands[1] = rhs;
	ins->operands[2] = NULL;
	if (!op)
		ins->opcode = 0;
	else if (strcmp(op, "+") == 0)
		ins->opcode = 1;
	else if (strcmp(op, "-") == 0)
		ins->opcode = 2;
	else if (strcmp(op, "*") == 0)
		ins->opcode = 3;
	else if (strcmp(op, "/") == 0)
		ins->opcode = 4;
	else if (strcmp(op, "%") == 0)
		ins->opcode = 5;
	else if (strcmp(op, "==") == 0)
		ins->opcode = 6;
	else if (strcmp(op, "!=") == 0)
		ins->opcode = 7;
	else if (strcmp(op, "<") == 0)
		ins->opcode = 8;
	else if (strcmp(op, ">") == 0)
		ins->opcode = 9;
	else if (strcmp(op, "<=") == 0)
		ins->opcode = 10;
	else if (strcmp(op, ">=") == 0)
		ins->opcode = 11;
	else
		ins->opcode = 0;
	ins->next = NULL;
	ins->call_args = NULL;
	ins->call_nargs = 0;
	if (block->last)
	{
		block->last->next = ins;
		block->last = ins;
	}
	else
	{
		block->first = block->last = ins;
	}
	return dst;
}

void ir_emit_ret(IRBlock* block, IRValue* value)
{
	if (!block)
	{
		fprintf(stderr, "Attempted to emit return into a NULL block. (Warning)\n");
		return;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	if (!ins)
		return;
	ins->kind = IR_RET;
	ins->dest = NULL;
	ins->operands[0] = value;
	ins->operands[1] = NULL;
	ins->operands[2] = NULL;
	ins->next = NULL;
	ins->call_args = NULL;
	ins->call_nargs = 0;
	if (block->last)
	{
		block->last->next = ins;
		block->last = ins;
	}
	else
	{
		block->first = block->last = ins;
	}
}

IRValue* ir_const_i64(int64_t value)
{
	IRValue* v = malloc(sizeof(IRValue));
	if (!v)
	{
		fprintf(stderr, "Failed to allocate IRValue (const i64). (Error)\n");
		return NULL;
	}
	v->kind = IRV_CONST;
	v->u.i64 = value;
	v->type = ir_type_i64();
	fprintf(stderr, "IR constant (i64) created: %lld. (Info)\n", (long long)value);
	return v;
}
IRValue* ir_const_string(IRModule* m, const char* str)
{
	if (!m || !str)
	{
		fprintf(stderr, "ir_const_string called with NULL module or string. (Error)\n");
		return NULL;
	}

	char* copy = strdup(str);
	if (!copy)
	{
		fprintf(stderr, "Failed to allocate memory for string constant. (Error)\n");
		return NULL;
	}

	static int next_str_idx = 1;
	char gname[64];
	snprintf(gname, sizeof(gname), ".str%d", next_str_idx++);
	fprintf(stderr, "ir_const_string: creating global %s for string '%s'\n", gname, str);

	IRType* t = ir_type_ptr(ir_type_i64());
	IRValue* g = ir_global_create(m, gname, t, NULL);
	if (!g)
	{
		fprintf(stderr, "Failed to create global for string constant. (Error)\n");
		free(copy);
		return NULL;
	}

	g->u.i64 = (int64_t)(intptr_t)copy;
	fprintf(stderr, "IR constant (string) created: \"%s\" as %s at %p. (Info)\n", str, gname,
	        (void*)copy);
	return g;
}

IRValue* ir_temp(IRBlock* block, IRType* type)
{
	if (!block || !type)
	{
		fprintf(stderr, "Invalid arguments to ir_temp. (Error)\n");
		return NULL;
	}
	static int next_temp = 1;
	IRValue* v = malloc(sizeof(IRValue));
	if (!v)
		return NULL;
	v->kind = IRV_TEMP;
	v->u.temp_id = next_temp++;
	v->type = type;
	fprintf(stderr, "Temporary IRValue t%d created. (Info)\n", v->u.temp_id);
	return v;
}

IRFunction* ir_function_create_in_module(IRModule* m, const char* name, IRType* return_type)
{
	if (!m || !name)
	{
		fprintf(stderr, "Invalid arguments to ir_function_create_in_module. (Error)\n");
		return NULL;
	}
	IRFunction* f = ir_function_create(name, return_type);
	if (!f)
		return NULL;
	ir_module_add_function(m, f);
	fprintf(stderr, "IRFunction '%s' created in module. (Info)\n", name);
	return f;
}

IRBlock* ir_block_create_in_function(IRFunction* f, const char* name)
{
	if (!f || !name)
	{
		fprintf(stderr, "Invalid arguments to ir_block_create_in_function. (Error)\n");
		return NULL;
	}
	IRBlock* b = ir_block_create(name);
	if (!b)
		return NULL;
	ir_function_add_block(f, b);
	fprintf(stderr, "IRBlock '%s' created in function '%s'. (Info)\n", name,
	        f->name ? f->name : "<anon>");
	return b;
}

IRValue* ir_param_create(IRFunction* f, const char* name, IRType* type)
{
	if (!f || !type)
	{
		fprintf(stderr, "Invalid arguments to ir_param_create. (Error)\n");
		return NULL;
	}
	IRValue* v = malloc(sizeof(IRValue));
	if (!v)
	{
		fprintf(stderr, "Failed to allocate IRValue for param. (Error)\n");
		return NULL;
	}
	static int next_param = 1;
	v->kind = IRV_PARAM;
	v->u.temp_id = next_param++;
	v->type = type;
	v->name = name ? strdup(name) : NULL;
	v->next = NULL;
	if (!f->params)
		f->params = v;
	else
	{
		IRValue* it = f->params;
		while (it->next)
			it = it->next;
		it->next = v;
	}
	fprintf(stderr, "Parameter created t%d for function '%s'. (Info)\n", v->u.temp_id,
	        f->name ? f->name : "<anon>");
	return v;
}

IRValue* ir_global_create(IRModule* m, const char* name, IRType* type, IRValue* initial)
{
	if (!m || !name || !type)
	{
		fprintf(stderr, "Invalid arguments to ir_global_create. (Error)\n");
		return NULL;
	}
	fprintf(stderr, "ir_global_create: creating global '%s'\n", name);
	IRValue* v = malloc(sizeof(IRValue));
	fprintf(stderr, "ir_global_create: malloc IRValue -> %p\n", (void*)v);
	if (!v)
	{
		fprintf(stderr, "Failed to allocate IRValue for global. (Error)\n");
		return NULL;
	}
	v->kind = IRV_GLOBAL;
	v->u.temp_id = 0;
	v->type = type;
	v->name = strdup(name);
	fprintf(stderr, "ir_global_create: strdup for v->name -> %p\n", (void*)v->name);
	v->next = NULL;
	(void)initial;
	IRGlobal* g = (IRGlobal*)malloc(sizeof(IRGlobal));
	fprintf(stderr, "ir_global_create: malloc IRGlobal -> %p\n", (void*)g);
	if (!g)
	{
		free(v->name);
		free(v);
		fprintf(stderr, "Failed to allocate IRGlobal. (Error)\n");
		return NULL;
	}
	g->name = strdup(name);
	fprintf(stderr, "ir_global_create: strdup for g->name -> %p\n", (void*)g->name);
	g->value = v;
	g->next = NULL;
	fprintf(stderr, "ir_global_create: current m->globals = %p\n", (void*)m->globals);
	if (!m->globals)
		m->globals = g;
	else
	{
		IRGlobal* it = m->globals;
		while (it->next)
			it = it->next;
		it->next = g;
	}
	fprintf(stderr, "ir_global_create: Global '%s' created and linked. (Info)\n", name);
	return v;
}

IRValue* ir_emit_alloca(IRBlock* b, IRType* type)
{
	if (!b || !type)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_alloca. (Error)\n");
		return NULL;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	fprintf(stderr, "ir_emit_alloca: malloc IRInstruction -> %p\n", (void*)ins);
	if (!ins)
		return NULL;
	ins->kind = IR_ALLOCA;
	IRValue* dst = ir_temp(b, type);
	ins->dest = dst;
	ins->operands[0] = NULL;
	ins->operands[1] = NULL;
	ins->operands[2] = NULL;
	ins->call_args = NULL;
	ins->call_nargs = 0;
	ins->next = NULL;
	if (b->last)
	{
		b->last->next = ins;
		b->last = ins;
	}
	else
	{
		b->first = b->last = ins;
	}
	return dst;
}

IRValue* ir_emit_load(IRBlock* b, IRValue* ptr)
{
	if (!b || !ptr)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_load. (Error)\n");
		return NULL;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	fprintf(stderr, "ir_emit_load: malloc IRInstruction -> %p\n", (void*)ins);
	if (!ins)
		return NULL;
	ins->kind = IR_LOAD;
	IRValue* dst = ir_temp(b, ptr->type ? ptr->type->pointee : NULL);
	ins->dest = dst;
	ins->operands[0] = ptr;
	ins->operands[1] = NULL;
	ins->operands[2] = NULL;
	ins->call_args = NULL;
	ins->call_nargs = 0;
	ins->next = NULL;
	if (b->last)
	{
		b->last->next = ins;
		b->last = ins;
	}
	else
	{
		b->first = b->last = ins;
	}
	return dst;
}

void ir_emit_store(IRBlock* b, IRValue* ptr, IRValue* val)
{
	if (!b || !ptr || !val)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_store. (Error)\n");
		return;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	fprintf(stderr, "ir_emit_store: malloc IRInstruction -> %p\n", (void*)ins);
	if (!ins)
		return;
	ins->kind = IR_STORE;
	ins->dest = NULL;
	ins->operands[0] = ptr;
	ins->operands[1] = val;
	ins->operands[2] = NULL;
	ins->call_args = NULL;
	ins->call_nargs = 0;
	ins->next = NULL;
	if (b->last)
	{
		b->last->next = ins;
		b->last = ins;
	}
	else
	{
		b->first = b->last = ins;
	}
}

IRValue* ir_emit_call(IRBlock* b, IRFunction* callee, IRValue** args, size_t nargs)
{
	if (!b || !callee)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_call. (Error)\n");
		return NULL;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	fprintf(stderr, "ir_emit_call: malloc IRInstruction -> %p\n", (void*)ins);
	if (!ins)
		return NULL;
	ins->kind = IR_CALL;
	IRValue* dst = NULL;
	if (callee->return_type && callee->return_type->kind != IR_T_VOID)
	{
		dst = ir_temp(b, callee->return_type);
		ins->dest = dst;
	}
	ins->operands[0] = NULL;
	ins->operands[1] = NULL;
	ins->operands[2] = (IRValue*)(void*)callee;
	/* copy args into a dynamic array for arbitrary arity */
	ins->call_args = NULL;
	ins->call_nargs = 0;
	if (nargs > 0 && args)
	{
		ins->call_args = (IRValue**)malloc(sizeof(IRValue*) * nargs);
		fprintf(stderr, "ir_emit_call: allocated call_args -> %p for instr %p\n", (void*)ins->call_args, (void*)ins);
		if (ins->call_args)
		{
			for (size_t i = 0; i < nargs; ++i)
				ins->call_args[i] = args[i];
			ins->call_nargs = nargs;
		}
		else
		{
			ins->call_nargs = 0;
		}
	}
	ins->next = NULL;
	if (b->last)
	{
		b->last->next = ins;
		b->last = ins;
	}
	else
	{
		b->first = b->last = ins;
	}
	return dst;
}

void ir_emit_br(IRBlock* b, IRBlock* target)
{
	if (!b || !target)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_br. (Error)\n");
		return;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	if (!ins)
		return;
	ins->kind = IR_BR;
	ins->dest = NULL;
	ins->operands[0] = (IRValue*)(void*)target;
	ins->operands[1] = NULL;
	ins->operands[2] = NULL;
	ins->call_args = NULL;
	ins->call_nargs = 0;
	ins->next = NULL;
	if (b->last)
	{
		b->last->next = ins;
		b->last = ins;
	}
	else
	{
		b->first = b->last = ins;
	}
}

void ir_emit_cbr(IRBlock* b, IRValue* cond, IRBlock* true_b, IRBlock* false_b)
{
	if (!b || !cond || !true_b || !false_b)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_cbr. (Error)\n");
		return;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	if (!ins)
		return;
	ins->kind = IR_CBR;
	ins->dest = NULL;
	ins->operands[0] = cond;
	ins->operands[1] = (IRValue*)(void*)true_b;
	ins->operands[2] = (IRValue*)(void*)false_b;
	ins->call_args = NULL;
	ins->call_nargs = 0;
	ins->next = NULL;
	if (b->last)
	{
		b->last->next = ins;
		b->last = ins;
	}
	else
	{
		b->first = b->last = ins;
	}
}

IRValue* ir_emit_phi(IRBlock* b, IRType* t, IRValue** values, IRBlock** blocks, size_t n)
{
	if (!b || !t || (n && (!values || !blocks)))
	{
		fprintf(stderr, "Invalid arguments to ir_emit_phi. (Error)\n");
		return NULL;
	}
	IRInstruction* ins = malloc(sizeof(IRInstruction));
	if (!ins)
		return NULL;
	ins->kind = IR_PHI;
	IRValue* dst = ir_temp(b, t);
	ins->dest = dst;
	ins->operands[0] = (n > 0) ? values[0] : NULL;
	ins->operands[1] = (n > 1) ? values[1] : NULL;
	ins->operands[2] = NULL;
	ins->call_args = NULL;
	ins->call_nargs = 0;
	ins->next = NULL;
	if (b->last)
	{
		b->last->next = ins;
		b->last = ins;
	}
	else
	{
		b->first = b->last = ins;
	}
	return dst;
}

int ir_validate_module(IRModule* m)
{
	if (!m)
	{
		fprintf(stderr, "ir_validate_module called with NULL module. (Error)\n");
		return 0;
	}
	IRFunction* f = m->functions;
	while (f)
	{
		IRBlock* b = f->blocks;
		while (b)
		{
			if (!b->last)
				return 0;
			IRInstruction* t = b->last;
			if (!(t->kind == IR_RET || t->kind == IR_BR || t->kind == IR_CBR))
				return 0;
			b = b->next;
		}
		f = f->next;
	}
	return 1;
}

void ir_replace_value(IRModule* m, IRValue* oldv, IRValue* newv)
{
	if (!m || !oldv || !newv)
	{
		fprintf(stderr, "Invalid arguments to ir_replace_value. (Error)\n");
		return;
	}
	IRFunction* f = m->functions;
	while (f)
	{
		IRBlock* b = f->blocks;
		while (b)
		{
			IRInstruction* ins = b->first;
			while (ins)
			{
				for (int i = 0; i < 3; ++i)
					if (ins->operands[i] == oldv)
						ins->operands[i] = newv;
				/* replace in call args array if present */
				if (ins->call_args && ins->call_nargs)
				{
					for (size_t ci = 0; ci < ins->call_nargs; ++ci)
						if (ins->call_args[ci] == oldv)
							ins->call_args[ci] = newv;
				}
				if (ins->dest == oldv)
					ins->dest = newv;
				ins = ins->next;
			}
			b = b->next;
		}
		f = f->next;
	}
}

void ir_remove_instruction(IRBlock* b, IRInstruction* i)
{
	if (!b || !i)
	{
		fprintf(stderr, "Invalid arguments to ir_remove_instruction. (Error)\n");
		return;
	}
	IRInstruction* prev = NULL;
	IRInstruction* cur = b->first;
	while (cur)
	{
		if (cur == i)
		{
			if (prev)
				prev->next = cur->next;
			else
				b->first = cur->next;
			if (b->last == cur)
				b->last = prev;
			if (cur->call_args)
				free(cur->call_args);
			free(cur);
			return;
		}
		prev = cur;
		cur = cur->next;
	}
}

void ir_instr_append(IRBlock* b, IRInstruction* i)
{
	if (!b || !i)
	{
		fprintf(stderr, "Invalid arguments to ir_instr_append. (Error)\n");
		return;
	}
	i->next = NULL;
	if (b->last)
	{
		b->last->next = i;
		b->last = i;
	}
	else
	{
		b->first = b->last = i;
	}
}

char* ir_strdup(IRModule* m, const char* s)
{
	(void)m;
	if (!s)
	{
		fprintf(stderr, "ir_strdup called with NULL string. (Warning)\n");
		return NULL;
	}
	fprintf(stderr, "String duplicated. (Info)\n");
	return strdup(s);
}

int ir_dump_module_to_file(IRModule* m, const char* path)
{
	if (!m || !path)
	{
		fprintf(stderr, "Invalid arguments to ir_dump_module_to_file. (Error)\n");
		return 0;
	}
	FILE* f = fopen(path, "w");
	if (!f)
	{
		fprintf(stderr, "Failed to open file '%s' for writing. (Error)\n", path);
		return 0;
	}
	ir_print_module(m, f);
	fclose(f);
	fprintf(stderr, "Module dumped to '%s'. (Info)\n", path);
	return 1;
}

void ir_walk_module(IRModule* m, void (*fn)(IRFunction*, void*), void* user)
{
	if (!m || !fn)
	{
		fprintf(stderr, "Invalid arguments to ir_walk_module. (Error)\n");
		return;
	}
	IRFunction* f = m->functions;
	while (f)
	{
		fn(f, user);
		f = f->next;
	}
}

void ir_walk_function(IRFunction* f, void (*fn)(IRBlock*, void*), void* user)
{
	if (!f || !fn)
	{
		fprintf(stderr, "Invalid arguments to ir_walk_function. (Error)\n");
		return;
	}
	IRBlock* b = f->blocks;
	while (b)
	{
		fn(b, user);
		b = b->next;
	}
}
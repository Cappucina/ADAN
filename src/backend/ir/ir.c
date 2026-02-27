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
	IRFunction* f = mod->functions;
	while (f)
	{
		IRFunction* fnext = f->next;
		free(f->name);
		IRBlock* b = f->blocks;
		while (b)
		{
			IRBlock* bnext = b->next;
			free(b->name);
			IRInstruction* i = b->first;
			while (i)
			{
				IRInstruction* inext = i->next;
				free(i);
				i = inext;
			}
			free(b);
			b = bnext;
		}
		free(f);
		f = fnext;
	}
	fprintf(stderr, "IRModule destroyed. (Info)\n");
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
	ins->next = NULL;
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
	IRValue* v = malloc(sizeof(IRValue));
	if (!v)
	{
		fprintf(stderr, "Failed to allocate IRValue for global. (Error)\n");
		return NULL;
	}
	v->kind = IRV_GLOBAL;
	v->u.temp_id = 0;
	v->type = type;
	(void)initial;
	fprintf(stderr, "Global '%s' created. (Info)\n", name);
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
	if (!ins)
		return NULL;
	ins->kind = IR_ALLOCA;
	IRValue* dst = ir_temp(b, type);
	ins->dest = dst;
	ins->operands[0] = NULL;
	ins->operands[1] = NULL;
	ins->operands[2] = NULL;
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
	if (!ins)
		return NULL;
	ins->kind = IR_LOAD;
	IRValue* dst = ir_temp(b, ptr->type ? ptr->type->pointee : NULL);
	ins->dest = dst;
	ins->operands[0] = ptr;
	ins->operands[1] = NULL;
	ins->operands[2] = NULL;
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
	if (!ins)
		return;
	ins->kind = IR_STORE;
	ins->dest = NULL;
	ins->operands[0] = ptr;
	ins->operands[1] = val;
	ins->operands[2] = NULL;
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
	if (!ins)
		return NULL;
	ins->kind = IR_CALL;
	IRValue* dst = NULL;
	if (callee->return_type && callee->return_type->kind != IR_T_VOID)
	{
		dst = ir_temp(b, callee->return_type);
		ins->dest = dst;
	}
	ins->operands[0] = (nargs > 0) ? args[0] : NULL;
	ins->operands[1] = (nargs > 1) ? args[1] : NULL;
	ins->operands[2] = (IRValue*)(void*)callee;
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
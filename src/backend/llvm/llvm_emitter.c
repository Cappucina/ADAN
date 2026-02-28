#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llvm_emitter.h"

static char* es_get_val_name(EmitterState* s, IRValue* v)
{
	if (!s || !v)
		return NULL;
	if (v->kind == IRV_CONST)
		return NULL;
	ValMap* it = s->vmap;
	while (it)
	{
		if (it->v == v)
			return it->name;
		it = it->next;
	}
	char buf[64];
	snprintf(buf, sizeof(buf), "%%v%lu", s->tmp_counter++);
	char* n = strdup(buf);
	ValMap* e2 = (ValMap*)malloc(sizeof(ValMap));
	e2->v = v;
	e2->name = n;
	e2->next = s->vmap;
	s->vmap = e2;
	return n;
}

static void es_emit_value_rep(EmitterState* s, FILE* outf, IRValue* v)
{
	if (!outf)
		return;
	if (!v)
	{
		fprintf(outf, "null");
		return;
	}
	if (v->kind == IRV_CONST)
	{
		if (v->type && v->type->kind == IR_T_I64)
			fprintf(outf, "%lld", (long long)v->u.i64);
		else
			fprintf(outf, "%lld", (long long)v->u.i64);
		return;
	}
	if (v->kind == IRV_GLOBAL)
	{
		if (v->name)
		{
			fprintf(outf, "@%s", v->name);
			return;
		}
	}
	char* n = es_get_val_name(s, v);
	if (n)
		fprintf(outf, "%s", n);
	else
		fprintf(outf, "<val>");
}

LLVMEEmitter* llvm_emitter_create(void)
{
	LLVMEEmitter* e = (LLVMEEmitter*)malloc(sizeof(LLVMEEmitter));
	if (!e)
	{
		fprintf(stderr, "Failed to allocate LLVMEEmitter. (Error)\n");
		return NULL;
	}
	e->ctx = llvm_utils_create_context();
	if (!e->ctx)
	{
		free(e);
		return NULL;
	}
	fprintf(stderr, "LLVMEEmitter created successfully. (Info)\n");
	return e;
}

void llvm_emitter_destroy(LLVMEEmitter* e)
{
	if (!e)
	{
		fprintf(stderr, "Attempted to destroy a NULL LLVMEEmitter. (Warning)\n");
		return;
	}
	llvm_utils_destroy_context(e->ctx);
	fprintf(stderr, "LLVMEEmitter destroyed successfully. (Info)\n");
	free(e);
}

int llvm_emitter_emit_module(LLVMEEmitter* e, IRModule* m, FILE* out)
{
	if (!e || !m || !out)
	{
		fprintf(stderr, "Invalid arguments to llvm_emitter_emit_module. (Error)\n");
		return 0;
	}

	EmitterState st = {.vmap = NULL, .tmp_counter = 1};

	for (IRGlobal* g = m->globals; g; g = g->next)
	{
		if (!g->name || !g->value)
			continue;
		IRValue* gv = g->value;
		if (gv->u.i64)
		{
			const char* s = (const char*)(intptr_t)gv->u.i64;
			if (s)
			{
				size_t len = strlen(s);
				fprintf(out, "@%s = private constant [%zu x i8] c\"", g->name,
				        len + 1);
				for (size_t i = 0; i < len; ++i)
				{
					unsigned char c = (unsigned char)s[i];
					if (c == '\\')
						fprintf(out, "\\5C");
					else if (c == '"')
						fprintf(out, "\\22");
					else if (c >= 32 && c < 127)
						fprintf(out, "%c", c);
					else
						fprintf(out, "\\%02X", c);
				}
				fprintf(out, "\\00\"\n");
			}
		}
	}

	for (IRFunction* f = m->functions; f; f = f->next)
	{
		char* rett = llvm_type_to_string(f->return_type ? f->return_type : ir_type_void());
		char* fname = llvm_utils_mangle_name(f->name ? f->name : "<anon>");
		for (IRValue* pv = f->params; pv; pv = pv->next)
		{
			(void)es_get_val_name(&st, pv);
		}

		if (!f->blocks)
		{
			fprintf(out, "declare %s @%s(", rett ? rett : "void",
					fname ? fname : "<anon>");
			int firstp = 1;
			for (IRValue* pv = f->params; pv; pv = pv->next)
			{
				char* ptype = llvm_type_to_string(pv->type ? pv->type : ir_type_i64());
				char* pname = es_get_val_name(&st, pv);
				if (!firstp)
					fprintf(out, ", ");
				fprintf(out, "%s %s", ptype ? ptype : "i64", pname ? pname : "<p>");
				free(ptype);
				firstp = 0;
			}
			fprintf(out, ")\n\n");
			free(rett);
			free(fname);
			continue;
		}

		fprintf(out, "define %s @%s(", rett ? rett : "void", fname ? fname : "<anon>");
		int firstp = 1;
		for (IRValue* pv = f->params; pv; pv = pv->next)
		{
			char* ptype = llvm_type_to_string(pv->type ? pv->type : ir_type_i64());
			char* pname = es_get_val_name(&st, pv);
			if (!firstp)
				fprintf(out, ", ");
			fprintf(out, "%s %s", ptype ? ptype : "i64", pname ? pname : "<p>");
			free(ptype);
			firstp = 0;
		}
		fprintf(out, ") {\n");
		free(rett);
		free(fname);

		for (IRBlock* b = f->blocks; b; b = b->next)
		{
			const char* label = b->name ? b->name : "entry";
			fprintf(out, "%s:\n", label);
			IRInstruction* ins = b->first;
			while (ins)
			{
				switch (ins->kind)
				{
					case IR_ALLOCA:
					{
						char* dname = es_get_val_name(&st, ins->dest);
						char* t = llvm_type_to_string(ins->dest->type);
						fprintf(out, "  %s = alloca %s\n",
						        dname ? dname : "<dst>", t ? t : "i64");
						free(t);
						break;
					}
					case IR_LOAD:
					{
						char* dname = es_get_val_name(&st, ins->dest);
						IRValue* ptr = ins->operands[0];
						IRType* pt =
						    ptr && ptr->type ? ptr->type->pointee : NULL;
						char* tstr =
						    llvm_type_to_string(pt ? pt : ir_type_i64());
						char* ptrname = es_get_val_name(&st, ptr);
						fprintf(out, "  %s = load %s, %s* %s\n",
						        dname ? dname : "<dst>",
						        tstr ? tstr : "i64", tstr ? tstr : "i64",
						        ptrname ? ptrname : "<ptr>");
						free(tstr);
						break;
					}
					case IR_STORE:
					{
						IRValue* ptr = ins->operands[0];
						IRValue* val = ins->operands[1];
						IRType* vt =
						    val && val->type ? val->type : ir_type_i64();
						char* vtstr = llvm_type_to_string(vt);
						char* ptrname = es_get_val_name(&st, ptr);
						fprintf(out, "  store %s ", vtstr ? vtstr : "i64");
						es_emit_value_rep(&st, out, val);
						fprintf(out, ", %s* %s\n", vtstr ? vtstr : "i64",
						        ptrname ? ptrname : "<ptr>");
						free(vtstr);
						break;
					}
					case IR_CALL:
					{
						IRFunction* callee =
						    (IRFunction*)(void*)ins->operands[2];
						char* callee_name = llvm_utils_mangle_name(
						    callee->name ? callee->name : "<anon>");
						char* rettype = llvm_type_to_string(
						    callee->return_type ? callee->return_type
						                        : ir_type_void());
						if (ins->dest)
						{
							char* dname =
							    es_get_val_name(&st, ins->dest);
							fprintf(
							    out, "  %s = call %s @%s(",
							    dname ? dname : "<dst>",
							    rettype ? rettype : "void",
							    callee_name ? callee_name : "<anon>");
						}
						else
						{
							fprintf(
							    out, "  call %s @%s(",
							    rettype ? rettype : "void",
							    callee_name ? callee_name : "<anon>");
						}
						int first = 1;
						if (ins->call_args && ins->call_nargs)
						{
							for (size_t ai = 0; ai < ins->call_nargs;
							     ++ai)
							{
								IRValue* a = ins->call_args[ai];
								char* at = llvm_type_to_string(
								    a && a->type ? a->type
								                 : ir_type_i64());
								if (!first)
									fprintf(out, ", ");
								fprintf(out, "%s ",
								        at ? at : "i64");
								es_emit_value_rep(&st, out, a);
								free(at);
								first = 0;
							}
						}
						fprintf(out, ")\n");
						free(callee_name);
						free(rettype);
						break;
					}
					case IR_RET:
					{
						IRValue* v = ins->operands[0];
						if (!v)
						{
							IRType* frt = f->return_type ? f->return_type : ir_type_void();
							if (frt->kind == IR_T_VOID)
							{
								fprintf(out, "  ret void\n");
							}
							else if (frt->kind == IR_T_PTR)
							{
								char* rt = llvm_type_to_string(frt);
								fprintf(out, "  ret %s null\n", rt ? rt : "ptr");
								free(rt);
							}
							else if (frt->kind == IR_T_F64)
							{
								char* rt = llvm_type_to_string(frt);
								fprintf(out, "  ret %s 0.0\n", rt ? rt : "f64");
								free(rt);
							}
							else
							{
								char* rt = llvm_type_to_string(frt);
								fprintf(out, "  ret %s 0\n", rt ? rt : "i64");
								free(rt);
							}
						}
						else
						{
							char* t = llvm_type_to_string(
							    v->type ? v->type : ir_type_i64());
							fprintf(out, "  ret %s ", t ? t : "i64");
							es_emit_value_rep(&st, out, v);
							fprintf(out, "\n");
							free(t);
						}
						break;
					}
					case IR_BR:
					{
						IRBlock* tgt = (IRBlock*)(void*)ins->operands[0];
						fprintf(out, "  br label %s\n",
						        tgt && tgt->name ? tgt->name : "<label>");
						break;
					}
					case IR_CBR:
					{
						IRValue* cond = ins->operands[0];
						IRBlock* tb = (IRBlock*)(void*)ins->operands[1];
						IRBlock* fb = (IRBlock*)(void*)ins->operands[2];
						fprintf(out, "  br i1 ");
						es_emit_value_rep(&st, out, cond);
						fprintf(out, ", label %s, label %s\n",
						        tb && tb->name ? tb->name : "<t>",
						        fb && fb->name ? fb->name : "<f>");
						break;
					}
					case IR_BINOP:
					{
						char* dst = es_get_val_name(&st, ins->dest);
						IRValue* lhs = ins->operands[0];
						IRValue* rhs = ins->operands[1];
						char* lstr = es_get_val_name(&st, lhs);
						char* rstr = es_get_val_name(&st, rhs);
						char* tstr = llvm_type_to_string(
						    ins->dest->type ? ins->dest->type
						                    : ir_type_i64());
						switch (ins->opcode)
						{
							case 1:  // +
								fprintf(out, "  %s = add %s ",
								        dst ? dst : "<dst>",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 2:  // -
								fprintf(out, "  %s = sub %s ",
								        dst ? dst : "<dst>",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 3:  // *
								fprintf(out, "  %s = mul %s ",
								        dst ? dst : "<dst>",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 4:  // /
								fprintf(out, "  %s = sdiv %s ",
								        dst ? dst : "<dst>",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 6:  // ==
								fprintf(out, "  %s = icmp eq %s ",
								        dst ? dst : "<dst>",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 7:  // !=
								fprintf(out, "  %s = icmp ne %s ",
								        dst ? dst : "<dst>",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							default:
								fprintf(out,
								        "  ; binop opcode %d "
								        "unsupported\n",
								        ins->opcode);
								break;
						}
						free(tstr);
						break;
					}
					break;
					case IR_PHI:
					{
						char* d = es_get_val_name(&st, ins->dest);
						char* t = llvm_type_to_string(ins->dest->type
						                                  ? ins->dest->type
						                                  : ir_type_i64());
						fprintf(out, "  %s = phi %s [", d ? d : "<dst>",
						        t ? t : "i64");
						es_emit_value_rep(&st, out, ins->operands[0]);
						fprintf(out, "]");
						if (ins->operands[1])
						{
							fprintf(out, ", [");
							es_emit_value_rep(&st, out,
							                  ins->operands[1]);
							fprintf(out, "]");
						}
						fprintf(out, "\n");
						free(t);
						break;
					}
					break;
					default:
						fprintf(out,
						        "  ; unsupported instruction kind %d\n",
						        ins->kind);
						break;
				}
				ins = ins->next;
			}
		}

		fprintf(out, "}\n\n");
	}

	ValMap* it = st.vmap;
	while (it)
	{
		ValMap* nx = it->next;
		free(it->name);
		free(it);
		it = nx;
	}

	return 1;
}

int llvm_emitter_emit_module_to_file(LLVMEEmitter* e, IRModule* m, const char* path)
{
	if (!e || !m || !path)
	{
		fprintf(stderr, "Invalid arguments to llvm_emitter_emit_module_to_file. (Error)\n");
		return 0;
	}
	FILE* f = fopen(path, "w");
	if (!f)
	{
		fprintf(stderr, "Failed to open '%s' for writing. (Error)\n", path);
		return 0;
	}
	int ok = llvm_emitter_emit_module(e, m, f);
	fclose(f);
	return ok;
}
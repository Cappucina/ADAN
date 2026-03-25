#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "llvm_emitter.h"
#include "../../macros.h"

static char* es_get_val_name(EmitterState* s, IRValue* v)
{
	if (!s || !v)
	{
		return NULL;
	}
	if (v->kind == IRV_CONST)
	{
		return NULL;
	}
	ValMap* it = s->vmap;
	while (it)
	{
		if (it->v == v)
		{
			return it->name;
		}
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
	{
		return;
	}
	if (!v)
	{
		fprintf(outf, "null");
		return;
	}
	if (v->kind == IRV_CONST)
	{
		if (v->type && (v->type->kind == IR_T_F64 || v->type->kind == IR_T_F32))
		{
			fprintf(outf, "%f", v->u.f64);
		}
		else if (v->type && v->type->kind == IR_T_I64)
		{
			fprintf(outf, "%lld", (long long)v->u.i64);
		}
		else
		{
			fprintf(outf, "%lld", (long long)v->u.i64);
		}
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
	{
		fprintf(outf, "%s", n);
	}
	else
	{
		fprintf(outf, "<val>");
	}
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
		{
			continue;
		}
		IRValue* gv = g->value;
		if (gv->type && gv->type->kind == IR_T_PTR && gv->u.i64)
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
					{
						fprintf(out, "\\5C");
					}
					else if (c == '"')
					{
						fprintf(out, "\\22");
					}
					else if (c >= 32 && c < 127)
					{
						fprintf(out, "%c", c);
					}
					else
					{
						fprintf(out, "\\%02X", c);
					}
				}
				fprintf(out, "\\00\"\n");
			}
		}
		else
		{
			char* tstr =
			    llvm_type_to_string(gv->type ? gv->type->pointee : ir_type_i64());
			if (g->initial)
			{
				if (gv->type && gv->type->pointee &&
				    gv->type->pointee->kind == IR_T_PTR)
				{
					fprintf(out, "@%s = global %s null\n", g->name,
					        tstr ? tstr : "i64*");
				}
				else
				{
					fprintf(out, "@%s = global %s ", g->name,
					        tstr ? tstr : "i64");
					es_emit_value_rep(&st, out, g->initial);
					fprintf(out, "\n");
				}
			}
			else
			{
				fprintf(out, "@%s = common global %s ", g->name,
				        tstr ? tstr : "i64");
				if (gv->type && gv->type->pointee &&
				    gv->type->pointee->kind == IR_T_PTR)
				{
					fprintf(out, "null\n");
				}
				else
				{
					fprintf(out, "0\n");
				}
			}
			free(tstr);
		}
	}

	fprintf(out, "declare i64 @adn_powi(i64, i64)\n");
	fprintf(out, "declare double @llvm.pow.f64(double, double)\n\n");

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
				char* ptype =
				    llvm_type_to_string(pv->type ? pv->type : ir_type_i64());
				if (!firstp)
				{
					fprintf(out, ", ");
				}
				fprintf(out, "%s", ptype ? ptype : "i64");
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
			{
				fprintf(out, ", ");
			}
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
			IRInstruction* last_emitted = NULL;
			if (!ins)
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
			IRValue* defined[256];
			size_t defined_count = 0;
			for (IRValue* pv = f->params; pv; pv = pv->next)
			{
				defined[defined_count++] = pv;
			}
			while (ins)
			{
				int skip = 0;

				switch (ins->kind)
				{
					case IR_ALLOCA:
						if (!ins->dest || !ins->dest->type)
							skip = 1;
						break;
					case IR_LOAD:
						if (!ins->dest || !ins->dest->type ||
						    !IS_DEFINED(ins->operands[0]))
							skip = 1;
						break;
					case IR_STORE:
						if (!IS_DEFINED(ins->operands[0]) ||
						    !IS_DEFINED(ins->operands[1]))
							skip = 1;
						break;
					case IR_CALL:
						if (!ins->operands[2])
							skip = 1;
						for (size_t ai = 0;
						     ins->call_args && ai < ins->call_nargs; ++ai)
						{
							if (!IS_DEFINED(ins->call_args[ai]))
							{
								skip = 1;
								break;
							}
						}
						break;
					case IR_RET:
						if (ins->operands[0] &&
						    !IS_DEFINED(ins->operands[0]))
							skip = 1;
						break;
					case IR_BR:
						// block operand doesn't need IS_DEFINED
						break;
					case IR_CBR:
						if (!IS_DEFINED(ins->operands[0]))
							skip = 1;
						break;
					case IR_BINOP:
						if (!ins->dest || !ins->dest->type ||
						    !IS_DEFINED(ins->operands[0]) ||
						    !IS_DEFINED(ins->operands[1]))
							skip = 1;
						break;
					case IR_PHI:
						if (!ins->dest || !ins->dest->type ||
						    !IS_DEFINED(ins->operands[0]))
							skip = 1;
						break;
					case IR_FPCVT:
					case IR_ITOFP:
						if (!ins->dest || !ins->dest->type ||
						    !IS_DEFINED(ins->operands[0]))
							skip = 1;
						break;
					default:
						skip = 1;
						break;
				}

				if (skip)
				{
					ins = ins->next;
					continue;
				}
				if (ins->dest && defined_count < 256)
				{
					defined[defined_count++] = ins->dest;
				}
				switch (ins->kind)
				{
					case IR_ALLOCA:
					{
						char* dname = es_get_val_name(&st, ins->dest);
						IRType* t_base = ins->dest->type;
						if (t_base && t_base->kind == IR_T_PTR && t_base->pointee)
						{
							t_base = t_base->pointee;
						}
						char* t = llvm_type_to_string(t_base);
						fprintf(out, "  %s = alloca %s\n", dname ? dname : "<dst>",
						        t ? t : "i64");
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
						fprintf(out, "  %s = load %s, %s* ",
						        dname ? dname : "<dst>",
						        tstr ? tstr : "i64", tstr ? tstr : "i64");
						es_emit_value_rep(&st, out, ptr);
						fprintf(out, "\n");
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
						fprintf(out, "  store %s ", vtstr ? vtstr : "i64");
						es_emit_value_rep(&st, out, val);
						fprintf(out, ", %s* ", vtstr ? vtstr : "i64");
						es_emit_value_rep(&st, out, ptr);
						fprintf(out, "\n");
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
						if (ins->dest && callee->return_type &&
						    callee->return_type->kind != IR_T_VOID)
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
								{
									fprintf(out, ", ");
								}
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
							IRType* frt = f->return_type
							                  ? f->return_type
							                  : ir_type_void();
							if (frt->kind == IR_T_VOID)
							{
								fprintf(out, "  ret void\n");
							}
							else if (frt->kind == IR_T_PTR)
							{
								char* rt = llvm_type_to_string(frt);
								fprintf(out, "  ret %s null\n",
								        rt ? rt : "ptr");
								free(rt);
							}
							else if (frt->kind == IR_T_F64)
							{
								char* rt = llvm_type_to_string(frt);
								fprintf(out, "  ret %s 0.0\n",
								        rt ? rt : "f64");
								free(rt);
							}
							else
							{
								char* rt = llvm_type_to_string(frt);
								fprintf(out, "  ret %s 0\n",
								        rt ? rt : "i64");
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
						fprintf(out, "  br label %%%s\n",
						        tgt && tgt->name ? tgt->name : "<label>");
						break;
					}
					case IR_CBR:
					{
						IRValue* cond = ins->operands[0];
						IRBlock* tb = (IRBlock*)(void*)ins->operands[1];
						IRBlock* fb = (IRBlock*)(void*)ins->operands[2];
						char* cond_name = es_get_val_name(&st, cond);
						char* t = llvm_type_to_string(cond->type);
						if (cond->type && cond->type->kind == IR_T_I64)
						{
							// Convert i64 to i1
							fprintf(out,
							        "  %%cbr_cond_%lu = icmp ne i64 ",
							        st.tmp_counter++);
							if (cond->kind == IRV_CONST)
							{
								fprintf(out, "%lld",
								        (long long)cond->u.i64);
							}
							else
							{
								fprintf(out, "%s",
								        cond_name ? cond_name
								                  : "<cond>");
							}
							fprintf(out, ", 0\n");
							fprintf(out,
							        "  br i1 %%cbr_cond_%lu, label "
							        "%%%s, label %%%s\n",
							        st.tmp_counter - 1,
							        tb && tb->name ? tb->name : "<t>",
							        fb && fb->name ? fb->name : "<f>");
						}
						else
						{
							fprintf(out, "  br i1 ");
							es_emit_value_rep(&st, out, cond);
							fprintf(out, ", label %%%s, label %%%s\n",
							        tb && tb->name ? tb->name : "<t>",
							        fb && fb->name ? fb->name : "<f>");
						}
						free(t);
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
						int is_float =
						    (lhs->type && (lhs->type->kind == IR_T_F32 ||
						                   lhs->type->kind == IR_T_F64));
						switch (ins->opcode)
						{
							case 1:  // +
								fprintf(out, "  %s = %s %s ",
								        dst ? dst : "<dst>",
								        is_float ? "fadd" : "add",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 2:  // -
								fprintf(out, "  %s = %s %s ",
								        dst ? dst : "<dst>",
								        is_float ? "fsub" : "sub",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 3:  // *
								fprintf(out, "  %s = %s %s ",
								        dst ? dst : "<dst>",
								        is_float ? "fmul" : "mul",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 4:  // /
								fprintf(out, "  %s = %s %s ",
								        dst ? dst : "<dst>",
								        is_float ? "fdiv" : "sdiv",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 5:  // %
								fprintf(out, "  %s = %s %s ",
								        dst ? dst : "<dst>",
								        is_float ? "frem" : "srem",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 12:  // ^ (exponentiation)
							{
								if (is_float)
								{
									fprintf(
									    out,
									    "  %s = call %s "
									    "@llvm.pow.f64(",
									    dst ? dst : "<dst>",
									    tstr ? tstr : "f64");
									fprintf(
									    out, "%s ",
									    tstr ? tstr : "f64");
									es_emit_value_rep(&st, out,
									                  lhs);
									fprintf(
									    out, ", %s ",
									    tstr ? tstr : "f64");
									es_emit_value_rep(&st, out,
									                  rhs);
									fprintf(out, ")\n");
								}
								else
								{
									fprintf(
									    out,
									    "  %s = call i64 "
									    "@adn_powi(",
									    dst ? dst : "<dst>");
									fprintf(out, "i64 ");
									es_emit_value_rep(&st, out,
									                  lhs);
									fprintf(out, ", i64 ");
									es_emit_value_rep(&st, out,
									                  rhs);
									fprintf(out, ")\n");
								}
								break;
							}
							case 6:   // ==
							case 7:   // !=
							case 8:   // <
							case 9:   // >
							case 10:  // <=
							case 11:  // >=
							{
								const char* icmp_op = "eq";
								const char* fcmp_op = "oeq";
								if (ins->opcode == 7)
								{
									icmp_op = "ne";
									fcmp_op = "une";
								}
								if (ins->opcode == 8)
								{
									icmp_op = "slt";
									fcmp_op = "olt";
								}
								if (ins->opcode == 9)
								{
									icmp_op = "sgt";
									fcmp_op = "ogt";
								}
								if (ins->opcode == 10)
								{
									icmp_op = "sle";
									fcmp_op = "ole";
								}
								if (ins->opcode == 11)
								{
									icmp_op = "sge";
									fcmp_op = "oge";
								}

								char* op_tstr =
								    llvm_type_to_string(lhs->type);
								fprintf(
								    out, "  %%cmp_%lu = %s %s %s ",
								    st.tmp_counter++,
								    is_float ? "fcmp" : "icmp",
								    is_float ? fcmp_op : icmp_op,
								    op_tstr ? op_tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								free(op_tstr);
								fprintf(out,
								        "  %s = zext i1 %%cmp_%lu "
								        "to %s\n",
								        dst ? dst : "<dst>",
								        st.tmp_counter - 1,
								        tstr ? tstr : "i64");
								break;
							}
							case 13:  // or
								fprintf(out, "  %s = or %s ",
								        dst ? dst : "<dst>",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 14:  // and
								fprintf(out, "  %s = and %s ",
								        dst ? dst : "<dst>",
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, lhs);
								fprintf(out, ", ");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, "\n");
								break;
							case 15:  // not (unary trick)
								fprintf(out,
								        "  %%cmp_%lu = icmp eq %s ",
								        st.tmp_counter++,
								        tstr ? tstr : "i64");
								es_emit_value_rep(&st, out, rhs);
								fprintf(out, ", 0\n");
								fprintf(out,
								        "  %s = zext i1 %%cmp_%lu "
								        "to %s\n",
								        dst ? dst : "<dst>",
								        st.tmp_counter - 1,
								        tstr ? tstr : "i64");
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
					case IR_FPCVT:
					{
						char* dname = es_get_val_name(&st, ins->dest);
						char* stype =
						    llvm_type_to_string(ins->operands[0]->type);
						char* dtype = llvm_type_to_string(ins->dest->type);
						const char* op = "fpext";
						if (ins->operands[0]->type->kind == IR_T_F64 &&
						    ins->dest->type->kind == IR_T_F32)
							op = "fptrunc";
						fprintf(out, "  %s = %s %s ",
						        dname ? dname : "<dst>", op,
						        stype ? stype : "double");
						es_emit_value_rep(&st, out, ins->operands[0]);
						fprintf(out, " to %s\n", dtype ? dtype : "float");
						free(stype);
						free(dtype);
						break;
					}
					case IR_ITOFP:
					{
						char* dname = es_get_val_name(&st, ins->dest);
						char* stype =
						    llvm_type_to_string(ins->operands[0]->type);
						char* dtype = llvm_type_to_string(ins->dest->type);
						fprintf(out, "  %s = sitofp %s ",
						        dname ? dname : "<dst>",
						        stype ? stype : "i64");
						es_emit_value_rep(&st, out, ins->operands[0]);
						fprintf(out, " to %s\n", dtype ? dtype : "double");
						free(stype);
						free(dtype);
						break;
					}
					default:
						break;
				}
				last_emitted = ins;
				ins = ins->next;
			}
			int needs_ret = 1;
			if (last_emitted &&
			    (last_emitted->kind == IR_RET || last_emitted->kind == IR_BR ||
			     last_emitted->kind == IR_CBR))
			{
				needs_ret = 0;
			}
			if (needs_ret)
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
		}

		fprintf(out, "}\n\n");
	}

	ValMap* it = st.vmap;
	while (it)
	{
		ValMap* nxt = it->next;
		free(it->name);
		free(it);
		it = nxt;
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
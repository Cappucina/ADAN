#include <stdio.h>
#include <stdlib.h>

#include "backend.h"
#include "lower.h"
#include "ir/ir.h"
#include "ir/emitter.h"
#include "ir/opt.h"
#include "llvm/llvm_emitter.h"

int backend_compile_ast_to_lltext(struct ASTNode* ast, FILE* out)
{
	if (!ast || !out)
	{
		return -1;
	}

	IRModule* m = ir_module_create();
	if (!m)
	{
		return -1;
	}

	Program prog = {0};
	prog.ast_root = ast;
	prog.ir = m;

	lower_program(&prog);

	ir_opt_run_all((void*)m);

	LLVMEEmitter* e = llvm_emitter_create();
	if (!e)
	{
		ir_module_destroy(m);
		return -1;
	}

	int ok = llvm_emitter_emit_module(e, m, out);

	llvm_emitter_destroy(e);
	ir_module_destroy(m);
	return ok ? 0 : -1;
}

int backend_compile_ast_to_llvm_file(struct ASTNode* ast, const char* path)
{
	if (!ast || !path)
	{
		return -1;
	}
	FILE* f = fopen(path, "w");
	if (!f)
	{
		return -1;
	}
	int res = backend_compile_ast_to_lltext(ast, f);
	fclose(f);
	return res;
}

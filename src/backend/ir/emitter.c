#include <stdlib.h>

#include "emitter.h"

IREmitterContext *ir_emitter_create(void)
{
	IREmitterContext *ctx = (IREmitterContext *)malloc(sizeof(IREmitterContext));
	if (!ctx)
	{
		fprintf(stderr, "Failed to allocate IREmitterContext. (Error)\n");
		return NULL;
	}
	fprintf(stderr, "IREmitterContext created successfully. (Info)\n");
	return ctx;
}

void ir_emitter_destroy(IREmitterContext *ctx)
{
	if (!ctx)
	{
		fprintf(stderr, "Attempted to destroy a NULL IREmitterContext. (Warning)\n");
		return;
	}
	fprintf(stderr, "IREmitterContext destroyed successfully. (Info)\n");
	free(ctx);
}

int ir_emit_module_to_lltext(IRModule *m, FILE *out)
{
	if (!m || !out)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_module_to_lltext. (Error)\n");
		return -1;
	}
	ir_print_module(m, out);
	return 0;
}

int ir_emit_module_to_llvm(IRModule *m, const char *out_path)
{
	if (!m || !out_path)
	{
		fprintf(stderr, "Invalid arguments to ir_emit_module_to_llvm. (Error)\n");
		return -1;
	}
	FILE *f = fopen(out_path, "w");
	if (!f)
	{
		fprintf(stderr, "Failed to open output file '%s' for writing. (Error)\n", out_path);
		return -1;
	}
	ir_print_module(m, f);
	fclose(f);
	return 0;
}
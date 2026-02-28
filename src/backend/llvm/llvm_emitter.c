#include <stdio.h>
#include <stdlib.h>

#include "llvm_emitter.h"

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
	return 1;
}

int llvm_emitter_emit_module_to_file(LLVMEEmitter* e, IRModule* m, const char* path)
{
	return 1;
}
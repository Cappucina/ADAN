#ifndef BACKEND_IR_EMITTER_H
#define BACKEND_IR_EMITTER_H

#include <stdio.h>
#include "ir.h"

typedef struct IREmitterContext
{
	IRModule* module;
} IREmitterContext;

IREmitterContext* ir_emitter_create(void);

void ir_emitter_destroy(IREmitterContext* ctx);

int ir_emit_module_to_lltext(IRModule* m, FILE* out);

int ir_emit_module_to_llvm(IRModule* m, const char* out_path);

#endif

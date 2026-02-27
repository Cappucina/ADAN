#ifndef BACKEND_LLVM_EMITTER_H
#define BACKEND_LLVM_EMITTER_H

#include <stdio.h>
#include "../ir/ir.h"
#include "llvm_utils.h"
#include "llvm_types.h"

typedef struct LLVMEEmitter
{
    LLVMContext* ctx;
} LLVMEEmitter;

LLVMEEmitter* llvm_emitter_create(void);

void llvm_emitter_destroy(LLVMEEmitter* e);

int llvm_emitter_emit_module(LLVMEEmitter* e, IRModule* m, FILE* out);

int llvm_emitter_emit_module_to_file(LLVMEEmitter* e, IRModule* m,
                                     const char* path);

#endif
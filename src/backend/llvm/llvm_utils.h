#ifndef BACKEND_LLVM_UTILS_H
#define BACKEND_LLVM_UTILS_H

#include <stddef.h>

typedef struct LLVMContext
{
	unsigned long label_counter;
	unsigned long tmp_counter;
} LLVMContext;

LLVMContext* llvm_utils_create_context(void);

void llvm_utils_destroy_context(LLVMContext* ctx);

char* llvm_utils_mangle_name(const char* name);

char* llvm_utils_unique_label(LLVMContext* ctx, const char* base);

#endif
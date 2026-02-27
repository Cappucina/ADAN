#ifndef BACKEND_LLVM_TYPES_H
#define BACKEND_LLVM_TYPES_H

#include "../ir/ir.h"

char* llvm_type_to_string(IRType* t);

char* llvm_type_mangle(IRType* t);

#endif
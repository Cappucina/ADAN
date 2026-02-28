#ifndef ADAN_BACKEND_LINKER_H
#define ADAN_BACKEND_LINKER_H

#include <stddef.h>

int linker_link_with_clang(const char* input_ll_path, const char* output_path, const char* libs);

int linker_llvm_link_bitcode(const char** inputs, size_t ninputs, const char* out_bc_path);

int linker_emit_bitcode_from_ll(const char* ll_path, const char* bc_path);

#endif

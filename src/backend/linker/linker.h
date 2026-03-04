#ifndef ADAN_BACKEND_LINKER_H
#define ADAN_BACKEND_LINKER_H

#include <stddef.h>

int linker_link_with_clang(const char* input_ll_path, const char* output_path, const char* libs);

int linker_link_and_bundle(const char* input_ll_path, const char* output_path, const char* libs,
                           const char* bundle_csv);

int linker_link_and_bundle_embedded(const char* input_ll_path, const char* output_path,
                                    const char* libs, const char* modules_csv);

#endif

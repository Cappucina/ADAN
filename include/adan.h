#ifndef ADAN_H
#define ADAN_H

#include <stdint.h>

const char* to_string(const void* input);

intptr_t to_int(const void* input);

int to_bool(const void* input);

int to_char(const void* input);

double to_float(const void* input);

const void* cast_to(int to_type, const void* input);

#endif
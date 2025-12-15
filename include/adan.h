#ifndef ADAN_H
#define ADAN_H

#include <stdint.h>

// Runtime helpers exported by the adan runtime library.
// Generated code can include this header to call the helpers safely.

// Convert any runtime value to a NUL-terminated string (caller must not free).
const char* to_string(const void* input);

// Convert to integer representation (small ints encoded as pointer-sized ints).
intptr_t to_int(const void* input);

// Convert to boolean (0 or 1).
int to_bool(const void* input);

// Convert to character code (returned as int).
int to_char(const void* input);

// Convert to floating point (double).
double to_float(const void* input);

// Generic cast-by-type for compatibility; accepts `Type` enum values.
const void* cast_to(int to_type, const void* input);

#endif
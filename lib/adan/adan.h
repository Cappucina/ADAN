#ifndef ADAN_RUNTIME_H
#define ADAN_RUNTIME_H

#include <stdint.h>

// Public runtime casting helpers. These functions are available in the
// adan runtime library and can be called from generated code without
// requiring any compiler-internal headers.

// Convert to string. Returns a pointer to a NUL-terminated string.
const char* to_string(const void* input);

// Convert to integer representation (small ints encoded as pointers).
intptr_t to_int(const void* input);

// Convert to boolean (0 or 1 integer return).
int to_bool(const void* input);

// Convert to character (returns numeric char value in int range).
int to_char(const void* input);

// Convert to floating point (double).
double to_float(const void* input);

// Generic cast-by-type (keeps backward compatibility). Accepts the
// `Type` integer values (from the compiler) and returns a `const void*`
// encoding the result when applicable.
const void* cast_to(int to_type, const void* input);

#endif

#include "stringUtils.h"

const char* cast(const void* input) {
	// The compiler may pass integer values in registers where the cast
	// expects a string pointer. Heuristically detect small integer values
	// and convert them to their decimal representation; otherwise treat
	// the input as a C string pointer.
	static char buf[64];
	intptr_t ip = (intptr_t)input;

	if (ip > -4096 && ip < 4096) {
		snprintf(buf, sizeof(buf), "%ld", (long)ip);
		return buf;
	}

	return (const char*)input;
}
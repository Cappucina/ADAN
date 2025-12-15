#include "string_utils.h"

const char* cast(const void* input) {
	static char buf[64];
	intptr_t ip = (intptr_t)input;

	if (ip > -4096 && ip < 4096) {
		snprintf(buf, sizeof(buf), "%ld", (long)ip);
		return buf;
	}

	return (const char*)input;
}
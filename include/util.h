#include <stdio.h>
#include <stdint.h>

#ifndef UTIL_H
#define UTIL_H

static inline unsigned long hash_string(const char *string) {
	unsigned long hash = 5381;
	int c;

	while ((c = *string++)) {
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}

	return hash;
}

static inline const char* cast(const void* input) {
	static char buf[64];
	intptr_t ip = (intptr_t)input;

	if (ip > -4096 && ip < 4096) {
		snprintf(buf, sizeof(buf), "%ld", (long)ip);
		return buf;
	}

	return (const char*)input;
}

#endif
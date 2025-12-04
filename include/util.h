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

#endif
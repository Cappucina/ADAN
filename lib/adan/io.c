#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void print(const char* str) {
	if (!str) return;
	printf("%s", str);
	fflush(stdout);
}

// Runtime helper: read a file into a heap-allocated buffer and return it.
// This function is provided to programs built by the compiler and is
// intentionally omitted when building the compiler itself (to avoid
// symbol conflicts during the compiler build).
#ifndef BUILDING_COMPILER_MAIN
const char* read_file_source(const char* file_path) {
	FILE* fp = fopen(file_path, "rb");
	if (!fp) {
		printf("error: failed to open file %s\n", file_path);
		return NULL;
	}

	char* out = NULL;
	size_t len = 0;
	size_t cap = 1024;

	out = malloc(cap);
	if (!out) {
		fclose(fp);
		return NULL;
	}

	char buf[256];
	while (fgets(buf, sizeof(buf), fp)) {
		size_t blen = strlen(buf);
		if (len + blen + 1 > cap) {
			cap *= 2;
			char* tmp = realloc(out, cap);
			if (!tmp) {
				free(out);
				fclose(fp);
				return NULL;
			}
			out = tmp;
		}

		memcpy(out + len, buf, blen);
		len += blen;
	}

	out[len] = '\0';
	fclose(fp);

	return out;
}

bool write_file_source(const char* file_path, const char* content) {
	FILE* fp = fopen(file_path, "wb");
	if (!fp) {
		// Create parent directories if needed
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "mkdir -p $(dirname \"%s\")", file_path);
		system(cmd);
		fp = fopen(file_path, "wb");
		if (!fp) {
			printf("error: failed to open file %s for writing\n", file_path);
			return false;
		}
	}

	size_t written = fwrite(content, 1, strlen(content), fp);
	fclose(fp);

	return written == strlen(content);
}
#endif

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
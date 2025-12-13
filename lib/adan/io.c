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

#ifndef BUILDING_COMPILER_MAIN
void print_int(long long x) {
	printf("%lld", x);
	fflush(stdout);
}
#endif

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

bool append_file_source(const char* file_path, const char* content) {
	FILE* fp = fopen(file_path, "ab");
	if (!fp) {
		// Create parent directories if needed
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "mkdir -p $(dirname \"%s\")", file_path);
		system(cmd);
		fp = fopen(file_path, "ab");
		if (!fp) {
			printf("error: failed to open file %s for appending\n", file_path);
			return false;
		}
	}

	size_t written = fwrite(content, 1, strlen(content), fp);
	fclose(fp);

	return written == strlen(content);
}

bool file_exists(const char* file_path) {
	FILE* fp = fopen(file_path, "rb");
	if (!fp) return false;
	fclose(fp);
	return true;
}

bool delete_file(const char* file_path) {
	if (remove(file_path) == 0) {
		return true;
	} else {
		return false;
	}
}

bool create_directory(const char* dir_path) {
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", dir_path);
	int res = system(cmd);
	return res == 0;
}

bool delete_directory(const char* dir_path) {
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", dir_path);
	int res = system(cmd);
	return res == 0;
}

bool is_directory(const char* path) {
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "[ -d \"%s\" ]", path);
	int res = system(cmd);
	return res == 0;
}

bool is_file(const char* path) {
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "[ -f \"%s\" ]", path);
	int res = system(cmd);
	return res == 0;
}

bool copy_file(const char* src_path, const char* dest_path) {
	char cmd[2048];
	snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s\"", src_path, dest_path);
	int res = system(cmd);
	return res == 0;
}

bool move_file(const char* src_path, const char* dest_path) {
	char cmd[2048];
	snprintf(cmd, sizeof(cmd), "mv \"%s\" \"%s\"", src_path, dest_path);
	int res = system(cmd);
	return res == 0;
}
#endif
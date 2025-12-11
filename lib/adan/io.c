#include <stdio.h>

void print(const char* str) {
	if (!str) return;
	printf("%s", str);
	fflush(stdout);
}
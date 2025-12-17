#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

void print(const char* str) {
	if (!str) return;
	printf("%s", str);
	fflush(stdout);
}

void println(const char* str) {
	print(str);
	print("\n");
}
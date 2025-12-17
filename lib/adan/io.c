#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print(const char* str) {
	if (!str) return;
	printf("%s", str);
	fflush(stdout);
}

void println(const char* str) {
	print(str);
	print("\n");
}

char* readln() {
    char *buff = malloc(1024);
    if (buff != NULL && fgets(buff, 1024, stdin)) {
        size_t len = strlen(buff);
        if (len > 0 && buff[len-1] == '\n') buff[len-1] = '\0';
        return buff;
    }
    free(buff);
    return NULL;
}
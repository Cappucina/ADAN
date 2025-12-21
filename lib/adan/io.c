#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print(const char* str) {
    if (!str || str[0] == '\0') return;
	printf("%s", str);
	fflush(stdout);
}

void println(const char* str) {
    if (!str) str = "";
	print(str);
	print("\n");
}

char* readln() {
    char *buff = malloc(1024);
    if (!buff) return strdup("");

    if (fgets(buff, 1024, stdin)) {
        size_t len = strlen(buff);
        if (len > 0 && buff[len-1] == '\n') buff[len-1] = '\0';

        if (buff[0] == '\0') {
            free(buff);
            return strdup("");
        }
        return buff;
    }

    free(buff);
    return strdup("");
}
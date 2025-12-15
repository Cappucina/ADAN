#include "string.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char* concat(char* str1, char* str2) {
	size_t len1 = strlen(str1);
	size_t len2 = strlen(str2);
	char* result = malloc(len1 + len2 + 1);
	if (!result) return NULL;
	strcpy(result, str1);
	strcat(result, str2);
	return result;
}

char* repeat(char* str, int n) {
    size_t len = strlen(str);
    char* result = malloc(len * n + 1);
    if (!result) return NULL;
    for (int i = 0; i < n; i++) {
        strcpy(result + i * len, str);
    }
    result[len * n] = '\0';
    return result;
}

char* reverse(char* str) {
    size_t len = strlen(str);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    for (size_t i = 0; i < len; i++) {
        result[i] = str[len - 1 - i];
    }
    result[len] = '\0';
    return result;
}

char* upper(char* str) {
    size_t len = strlen(str);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    for (size_t i = 0; i < len; i++) {
        result[i] = toupper(str[i]);
    }
    result[len] = '\0';
    return result;
}

char* lower(char* str) {
    size_t len = strlen(str);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    for (size_t i = 0; i < len; i++) {
        result[i] = tolower(str[i]);
    }
    result[len] = '\0';
    return result;
}

char* trim(char* str) {
    size_t len = strlen(str);
    size_t start = 0;
    size_t end = len - 1;
    while (start < len && isspace(str[start])) {
        start++;
    }
    while (end > start && isspace(str[end])) {
        end--;
    }
    char* result = malloc(end - start + 2);
    if (!result) return NULL;
    strncpy(result, str + start, end - start + 1);
    result[end - start + 1] = '\0';
    return result;
}

char* replace(char* str, char* old, char* new) {    size_t len_len = strlen(str);
    size_t old_len = strlen(old);
    size_t new_len = strlen(new);
    char* result = malloc(len_len + 1);
    if (!result) return NULL;
    size_t i = 0, j = 0;
    while (i < len_len) {
        if (strncmp(str + i, old, old_len) == 0) {
            strcpy(result + j, new);
            j += new_len;
            i += old_len;
        } else {
            result[j++] = str[i++];
        }
    }
    result[j] = '\0';
    return result;
}

static char* match_str(char* str, char* pattern) {
    size_t len = strlen(str);
    size_t pat_len = strlen(pattern);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    size_t i = 0, j = 0;
    while (i < len) {
        if (str[i] == pattern[j]) {
            j++;
        }
        i++;
    }
    if (j == pat_len) {
        strncpy(result, str, len);
        result[len] = '\0';
    } else {
        result[0] = '\0';
    }
    return result;
}

char* split(char* str, char delimiter) {
    size_t len = strlen(str);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    size_t i = 0, j = 0;
    while (i < len) {
        if (str[i] == delimiter) {
            result[j++] = ' ';
        } else {
            result[j++] = str[i];
        }
        i++;
    }
    result[j] = '\0';
    return result;
}

char* join(char* str, char delimiter) {
    size_t len = strlen(str);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    size_t i = 0, j = 0;
    while (i < len) {
        if (isspace(str[i])) {
            result[j++] = delimiter;
        } else {
            result[j++] = str[i];
        }
        i++;
    }
    result[j] = '\0';
    return result;
}

char* find(char* str, char* substring) {
    if (!str || !substring) return NULL;
    char* pos = strstr(str, substring);
    if (!pos) return NULL;
    size_t sub_len = strlen(substring);
    char* result = malloc(sub_len + 1);
    if (!result) return NULL;
    memcpy(result, pos, sub_len);
    result[sub_len] = '\0';
    return result;
}

int starts_with(char* s, char* prefix) {
    if (!s || !prefix) return 0;
    size_t p = strlen(prefix);
    if (strlen(s) < p) return 0;
    return strncmp(s, prefix, p) == 0;
}

char* replace_all(char* str, char* old, char* new) {
    size_t len = strlen(str);
    size_t old_len = strlen(old);
    size_t new_len = strlen(new);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    size_t i = 0, j = 0;
    while (i < len) {
        if (strncmp(str + i, old, old_len) == 0) {
            strcpy(result + j, new);
            j += new_len;
            i += old_len;
        } else {
            result[j++] = str[i++];
        }
    }
    result[j] = '\0';
    return result;
}

#ifndef BUILDING_COMPILER_MAIN
const char* cast(const void* input) {
    static char buf[64];
    intptr_t ip = (intptr_t)input;
    if (ip > -4096 && ip < 4096) {
        snprintf(buf, sizeof(buf), "%ld", (long)ip);
        return buf;
    }
    return (const char*)input;
}
#endif
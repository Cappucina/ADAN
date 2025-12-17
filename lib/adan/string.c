#include "string.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "ast.h"

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

static const char* cast_internal(const void* input) {
    static char buf[64];
    intptr_t ip = (intptr_t)input;
    if (ip > -4096 && ip < 4096) {
        snprintf(buf, sizeof(buf), "%ld", (long)ip);
        return buf;
    }
    return (const char*)input;
}

const char* to_string(const void* input) {
    return cast_internal(input);
}

intptr_t to_int(const void* input) {
    intptr_t ip = (intptr_t)input;
    if (ip > -4096 && ip < 4096) return ip;
    if (!input) return 0;
    return (intptr_t)strtol((const char*)input, NULL, 10);
}

int to_bool(const void* input) {
    intptr_t ip = (intptr_t)input;
    if (ip > -4096 && ip < 4096) return ip != 0;
    if (!input) return 0;
    if (strcmp((const char*)input, "true") == 0) return 1;
    return atoi((const char*)input) != 0;
}

int to_char(const void* input) {
    intptr_t ip = (intptr_t)input;
    if (ip > -4096 && ip < 4096) return (int)((char)ip);
    if (!input) return 0;
    return (int)(*(const char*)input);
}

double to_float(const void* input) {
    intptr_t ip = (intptr_t)input;
    if (ip > -4096 && ip < 4096) return (double)ip;
    if (!input) return 0.0;
    return atof((const char*)input);
}

const void* cast_to(int to_type, const void* input) {
    intptr_t ip = (intptr_t)input;
    int is_small_int = (ip > -4096 && ip < 4096);

    switch (to_type) {
        case TYPE_STRING:
            return (const void*)to_string(input);

        case TYPE_INT:
            return (const void*)(intptr_t)to_int(input);

        case TYPE_BOOLEAN:
            return (const void*)(intptr_t)to_bool(input);

        case TYPE_CHAR:
            return (const void*)(intptr_t)to_char(input);

        case TYPE_FLOAT:
            return (const void*)(intptr_t)((int)to_float(input));

        default:
            return input;
    }
}

#endif
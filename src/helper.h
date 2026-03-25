#ifndef HELPER_H
#define HELPER_H

#ifdef _WIN32
#include <sys/stat.h>
#include <string.h>
#define PATH_MAX 260
#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & _S_IFMT) == _S_IFDIR)
#endif
#define strtok_r strtok_s
#define strdup _strdup
#endif

#include <stddef.h>

char* read_file(char* file_path);

unsigned int hash(const char* name);

char* clone_string(const char* string, size_t length);

#endif
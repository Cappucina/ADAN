#ifndef HELPER_H
#define HELPER_H

#include <stddef.h>

char* read_file(char* file_path);

unsigned int hash(const char* name);

char* clone_string(const char* string, size_t length);

#endif
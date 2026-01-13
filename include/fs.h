#ifndef FS_H
#define FS_H

#include <stdbool.h>
#include <stdio.h>

#include "diagnostic.h"

bool file_exsists(const char* file_location);

const char* file_to_string(FILE* file, ErrorList* error_list);

#endif

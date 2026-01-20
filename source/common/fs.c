#include "fs.h"

bool file_exsists(const char* file_location)
{
    FILE* file;

    if ((file = fopen(file_location, "r")) != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

const char* file_to_string(FILE* file, ErrorList* error_list)
{
    if (!file)
    {
        error(error_list, "input", 0, 0, GENERIC, "File not accessible.");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t length = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);

    if (!buffer)
    {
        error(error_list, "input", 0, 0, GENERIC, "No memory left.");
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    return buffer;
}

void free_file_string(const char* file_string)
{
    free(file_string);
}
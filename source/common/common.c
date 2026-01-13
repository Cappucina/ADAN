#include "common.h"

char* strdup(const char* s)
{
    if (s == NULL)
    {
        return NULL;
    }

    size_t size = strlen(s) + 1;
    char* p = malloc(size);
    if (p)
    {
        memcpy(p, s, size);
    }
    return p;
}
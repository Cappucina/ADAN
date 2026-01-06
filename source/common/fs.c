#include "fs.h"

#include <stdio.h>

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

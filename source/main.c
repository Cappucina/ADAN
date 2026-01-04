#include <stdio.h>

#include "diagnostic.h"

int main()
{
    ErrorList* error_list = create_errors();

    if (error_list->size > 0)
    {
	    printf("%s\n", error_list->errors[0].message);
    }

    free_errors(error_list);

    printf("I am compiling correctly\n");

    return 0;
}

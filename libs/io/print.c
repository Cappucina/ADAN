#include <stdio.h>

#include "print.h"

void adn_println(const char* message)
{
	if (message)
	{
		printf("%s\n", message);
	}
	else
	{
		printf("\n");
	}
}

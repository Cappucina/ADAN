#include <stdio.h>

#include "print.h"
#include <string.h>

void adn_println(const char* message)
{
	if (!message)
	{
		putchar('\n');
		return;
	}
	size_t len = strlen(message);
	if (len > 0 && message[len - 1] == '\n')
	{
		fputs(message, stdout);
	}
	else
	{
		printf("%s\n", message);
	}
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "print.h"

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

char* adn_input(const char* prompt)
{
	if (prompt && prompt[0] != '\0')
	{
		fputs(prompt, stdout);
		fflush(stdout);
	}
#ifdef _POSIX_C_SOURCE
	char* line = NULL;
	size_t len = 0;
	ssize_t r = getline(&line, &len, stdin);
	if (r == -1)
	{
		free(line);
		return NULL;
	}
	if (r > 0 && line[r - 1] == '\n')
	{
		line[r - 1] = '\0';
	}
	return line;
#else
	size_t cap = 256;
	char* buf = malloc(cap);
	if (!buf)
	{
		return NULL;
	}
	if (!fgets(buf, cap, stdin))
	{
		free(buf);
		return NULL;
	}
	size_t l = strlen(buf);
	while (l > 0 && buf[l - 1] != '\n' && !feof(stdin))
	{
		cap *= 2;
		char* nbuf = realloc(buf, cap);
		if (!nbuf)
		{
			free(buf);
			return NULL;
		}
		buf = nbuf;
		if (!fgets(buf + l, cap - l, stdin))
		{
			break;
		}
		l = strlen(buf);
	}
	if (l > 0 && buf[l - 1] == '\n')
	{
		buf[l - 1] = '\0';
	}
	return buf;
#endif
}

void adn_errorln(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	fflush(stderr);
	exit(1);
}
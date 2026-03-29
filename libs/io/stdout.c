#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include "io.h"

static const char* unwrap_string_literal(const char* text, size_t* out_len)
{
	if (!text)
	{
		if (out_len)
		{
			*out_len = 0;
		}
		return NULL;
	}

	size_t len = strlen(text);
	const char* start = text;
	if (len >= 2 && ((text[0] == '"' && text[len - 1] == '"') ||
	                 (text[0] == '\'' && text[len - 1] == '\'') ||
	                 (text[0] == '`' && text[len - 1] == '`')))
	{
		start = text + 1;
		len -= 2;
	}

	if (out_len)
	{
		*out_len = len;
	}
	return start;
}

void adn_print(const char* message)
{
	if (!message)
	{
		putchar('\n');
		return;
	}
	size_t print_len = 0;
	const char* start = unwrap_string_literal(message, &print_len);
	printf("%.*s\n", (int)print_len, start);
}

void adn_flush(void)
{
	fflush(stdout);
	fflush(stderr);
}

void adn_write_file(const char* path, const char* content)
{
	size_t path_len = 0;
	size_t content_len = 0;
	const char* raw_path = unwrap_string_literal(path, &path_len);
	const char* raw_content = unwrap_string_literal(content, &content_len);
	if (!raw_path || path_len == 0)
	{
		fprintf(stderr, "write() requires a valid path. (Error)\n");
		exit(1);
	}

	char* normalized_path = malloc(path_len + 1);
	if (!normalized_path)
	{
		fprintf(stderr, "Failed to allocate memory for file path. (Error)\n");
		exit(1);
	}
	memcpy(normalized_path, raw_path, path_len);
	normalized_path[path_len] = '\0';

	FILE* file = fopen(normalized_path, "wb");
	if (!file)
	{
		fprintf(stderr, "Failed to open '%s' for writing: %s (Error)\n", normalized_path,
		        strerror(errno));
		free(normalized_path);
		exit(1);
	}

	if (raw_content && content_len > 0)
	{
		size_t written = fwrite(raw_content, 1, content_len, file);
		if (written != content_len)
		{
			fprintf(stderr, "Failed to write full content to '%s'. (Error)\n",
			        normalized_path);
			fclose(file);
			free(normalized_path);
			exit(1);
		}
	}

	fclose(file);
	free(normalized_path);
}

char* adn_read_file(const char* path)
{
	size_t path_len = 0;
	const char* raw_path = unwrap_string_literal(path, &path_len);
	if (!raw_path || path_len == 0)
	{
		return NULL;
	}

	char* normalized_path = malloc(path_len + 1);
	if (!normalized_path)
	{
		return NULL;
	}
	memcpy(normalized_path, raw_path, path_len);
	normalized_path[path_len] = '\0';

	FILE* file = fopen(normalized_path, "rb");
	free(normalized_path);
	if (!file)
	{
		return NULL;
	}

	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		return NULL;
	}
	long size = ftell(file);
	if (size < 0)
	{
		fclose(file);
		return NULL;
	}
	rewind(file);

	char* buffer = malloc((size_t)size + 1);
	if (!buffer)
	{
		fclose(file);
		return NULL;
	}

	size_t read = fread(buffer, 1, (size_t)size, file);
	fclose(file);
	buffer[read] = '\0';
	return buffer;
}

char* adn_input(const char* prompt)
{
	if (prompt && prompt[0] != '\0')
	{
		size_t print_len = 0;
		const char* start = unwrap_string_literal(prompt, &print_len);
		printf("%.*s", (int)print_len, start);
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

void adn_error(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	fflush(stderr);
	exit(1);
}
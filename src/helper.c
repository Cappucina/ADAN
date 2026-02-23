#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* read_file(char* file_path)
{
	FILE* fptr = fopen(file_path, "r");
	if (!fptr)
	{
		printf("Failed to open file! (Error)");
		return NULL;
	}

	fseek(fptr, 0, SEEK_END);
	long fsize = ftell(fptr);
	rewind(fptr);

	char* buffer = (char*)malloc(fsize + 1);
	if (!buffer)
	{
		printf("Failed to allocate memory for read file buffer! (Error)");
		return NULL;
	}

	fread(buffer, 1, fsize, fptr);
	buffer[fsize] = '\0';

	fclose(fptr);
	return buffer;
}

unsigned int hash(const char* name)
{
	unsigned int hash_value = 5531;

	for (int i = 0; name[i] != '\0'; i++)
	{
		hash_value = (hash_value * 33) + name[i];
	}

	return hash_value;
}

// `strdup` is a POSIX addition and not a standard C function.
char* clone_string(const char* string)
{
	if (!string)
		return NULL;

	size_t total = strlen(string);
	char* alloc = (char*)malloc(total + 1);
	if (!alloc)
	{
		printf("Failed to clone string during memory allocation segment! (Error)");
		return NULL;
	}

	strcpy(alloc, string);
	return alloc;
}
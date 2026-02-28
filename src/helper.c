#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* read_file(char* file_path)
{
	FILE* fptr = fopen(file_path, "rb");
	if (!fptr)
	{
		fprintf(stderr, "Failed to open file '%s'! (Error)\n", file_path);
		return NULL;
	}

	if (fseek(fptr, 0, SEEK_END) != 0)
	{
		fprintf(stderr, "Failed to seek end of file '%s'! (Error)\n", file_path);
		fclose(fptr);
		return NULL;
	}

	long fsize = ftell(fptr);
	if (fsize < 0)
	{
		fprintf(stderr, "Failed to determine file size for '%s'! (Error)\n", file_path);
		fclose(fptr);
		return NULL;
	}

	rewind(fptr);

	char* buffer = (char*)malloc((size_t)fsize + 1);
	if (!buffer)
	{
		fprintf(stderr, "Failed to allocate memory for read file buffer! (Error)\n");
		fclose(fptr);
		return NULL;
	}

	size_t read = fread(buffer, 1, (size_t)fsize, fptr);
	if (read != (size_t)fsize)
	{
		fprintf(stderr, "Failed to read file '%s' fully: read %zu of %ld bytes. (Error)\n",
		        file_path, read, fsize);
		free(buffer);
		fclose(fptr);
		return NULL;
	}

	buffer[fsize] = '\0';

	fclose(fptr);
	return buffer;
}

unsigned int hash(const char* name)
{
	if (!name)
	{
		return 0;
	}

	unsigned int hash_value = 5531;

	for (size_t i = 0; name[i] != '\0'; i++)
	{
		hash_value = (hash_value * 33) + (unsigned char)name[i];
	}

	return hash_value;
}

char* clone_string(const char* string, size_t length)
{
	if (!string)
	{
		return NULL;
	}

	char* alloc = (char*)malloc(length + 1);
	if (!alloc)
	{
		fprintf(stderr,
		        "Failed to clone string segment during memory allocation! (Error)\n");
		return NULL;
	}

	memcpy(alloc, string, length);
	alloc[length] = '\0';
	return alloc;
}
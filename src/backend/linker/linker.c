#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "linker.h"

static int run_cmd(char* const argv[])
{
	if (!argv || !argv[0])
	{
		return -1;
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		fprintf(stderr, "linker: fork() failed\n");
		return -1;
	}
	if (pid == 0)
	{
		execvp(argv[0], argv);
		perror("linker: execvp failed");
		_exit(127);
	}
	int status;
	if (waitpid(pid, &status, 0) == -1)
	{
		fprintf(stderr, "linker: waitpid() failed\n");
		return -1;
	}
	if (WIFEXITED(status))
	{
		return WEXITSTATUS(status);
	}
	return -1;
}

/* Split a space-separated string into a NULL-terminated argv array.
   *out_storage receives the duplicated string that backs the tokens;
   both *out_storage and the returned array must be freed by the caller. */
static char** split_args(const char* str, size_t* out_count, char** out_storage)
{
	*out_count = 0;
	*out_storage = NULL;
	if (!str || !str[0])
	{
		return NULL;
	}
	char* copy = strdup(str);
	if (!copy)
	{
		return NULL;
	}
	size_t count = 0;
	char* p = copy;
	while (*p)
	{
		while (*p == ' ')
			p++;
		if (*p)
		{
			count++;
			while (*p && *p != ' ')
				p++;
		}
	}
	char** arr = (char**)malloc((count + 1) * sizeof(char*));
	if (!arr)
	{
		free(copy);
		return NULL;
	}
	size_t idx = 0;
	p = copy;
	while (*p && idx < count)
	{
		while (*p == ' ')
			p++;
		if (*p)
		{
			arr[idx++] = p;
			while (*p && *p != ' ')
				p++;
			if (*p)
				*p++ = '\0';
		}
	}
	arr[idx] = NULL;
	*out_count = idx;
	*out_storage = copy;
	return arr;
}

int linker_link_with_clang(const char* input_ll_path, const char* output_path, const char* libs)
{
	if (!input_ll_path || !output_path)
	{
		return -1;
	}
	int r;
	if (libs && libs[0])
	{
		size_t lib_count = 0;
		char* lib_storage = NULL;
		char** lib_args = split_args(libs, &lib_count, &lib_storage);
		if (!lib_args)
		{
			return -1;
		}
		/* argv: clang, input, -o, output, lib_args..., NULL */
		size_t argc = 4 + lib_count + 1;
		char** argv = (char**)malloc(argc * sizeof(char*));
		if (!argv)
		{
			free(lib_storage);
			free(lib_args);
			return -1;
		}
		argv[0] = "clang";
		argv[1] = (char*)input_ll_path;
		argv[2] = "-o";
		argv[3] = (char*)output_path;
		for (size_t i = 0; i < lib_count; ++i)
			argv[4 + i] = lib_args[i];
		argv[argc - 1] = NULL;
		r = run_cmd(argv);
		free(argv);
		free(lib_storage);
		free(lib_args);
	}
	else
	{
		char* argv[] = {"clang", (char*)input_ll_path, "-o", (char*)output_path, NULL};
		r = run_cmd(argv);
	}
	return r;
}

int linker_emit_bitcode_from_ll(const char* ll_path, const char* bc_path)
{
	if (!ll_path || !bc_path)
	{
		return -1;
	}
	char* argv[] = {"llvm-as", "-o", (char*)bc_path, (char*)ll_path, NULL};
	return run_cmd(argv);
}

int linker_llvm_link_bitcode(const char** inputs, size_t ninputs, const char* out_bc_path)
{
	if (!inputs || ninputs == 0 || !out_bc_path)
	{
		return -1;
	}
	/* argv: llvm-link, -o, out_bc_path, inputs..., NULL */
	size_t argc = 3 + ninputs + 1;
	char** argv = (char**)malloc(argc * sizeof(char*));
	if (!argv)
	{
		return -1;
	}
	argv[0] = "llvm-link";
	argv[1] = "-o";
	argv[2] = (char*)out_bc_path;
	for (size_t i = 0; i < ninputs; ++i)
		argv[3 + i] = (char*)inputs[i];
	argv[argc - 1] = NULL;
	int r = run_cmd(argv);
	free(argv);
	return r;
}

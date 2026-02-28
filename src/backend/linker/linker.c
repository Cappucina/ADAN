#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "linker.h"

static int run_cmd(const char* cmd)
{
	if (!cmd)
		return -1;
	fprintf(stderr, "linker: running: %s\n", cmd);
	int rc = system(cmd);
	if (rc == -1)
	{
		fprintf(stderr, "linker: system() failed\n");
		return -1;
	}
	if (WIFEXITED(rc))
		return WEXITSTATUS(rc);
	return rc;
}

int linker_link_with_clang(const char* input_ll_path, const char* output_path, const char* libs)
{
	if (!input_ll_path || !output_path)
		return -1;
	size_t len = strlen(input_ll_path) + strlen(output_path) + 64 + (libs ? strlen(libs) : 0);
	char* cmd = (char*)malloc(len);
	if (!cmd)
		return -1;
	if (libs && libs[0])
		snprintf(cmd, len, "clang %s -o %s %s", input_ll_path, output_path, libs);
	else
		snprintf(cmd, len, "clang %s -o %s", input_ll_path, output_path);
	int r = run_cmd(cmd);
	free(cmd);
	return r;
}

int linker_emit_bitcode_from_ll(const char* ll_path, const char* bc_path)
{
	if (!ll_path || !bc_path)
		return -1;
	size_t len = strlen(ll_path) + strlen(bc_path) + 32;
	char* cmd = (char*)malloc(len);
	if (!cmd)
		return -1;
	snprintf(cmd, len, "llvm-as -o %s %s", bc_path, ll_path);
	int r = run_cmd(cmd);
	free(cmd);
	return r;
}

int linker_llvm_link_bitcode(const char** inputs, size_t ninputs, const char* out_bc_path)
{
	if (!inputs || ninputs == 0 || !out_bc_path)
		return -1;
	size_t len = 64 + strlen(out_bc_path);
	for (size_t i = 0; i < ninputs; ++i)
		len += strlen(inputs[i]) + 3;
	char* cmd = (char*)malloc(len);
	if (!cmd)
		return -1;
	strcpy(cmd, "llvm-link -o ");
	strcat(cmd, out_bc_path);
	for (size_t i = 0; i < ninputs; ++i)
	{
		strcat(cmd, " ");
		strcat(cmd, inputs[i]);
	}
	int r = run_cmd(cmd);
	free(cmd);
	return r;
}

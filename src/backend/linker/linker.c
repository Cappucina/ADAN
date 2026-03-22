#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "linker.h"
#include "../../embedded_libs.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>
#define unlink      _unlink
#define rmdir       _rmdir
#define mkdir(p, m) _mkdir(p)
#define getpid      (int)GetCurrentProcessId
static char* get_tmp_dir(void)
{
	static char buf[MAX_PATH];
	GetTempPathA(MAX_PATH, buf);
	return buf;
}
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
static char* get_tmp_dir(void)
{
	return "/tmp";
}
#endif

static int path_is_dir(const char* path)
{
#ifdef _WIN32
	DWORD a = GetFileAttributesA(path);
	return (a != INVALID_FILE_ATTRIBUTES) && (a & FILE_ATTRIBUTE_DIRECTORY);
#else
	struct stat st;
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

static int path_exists(const char* path)
{
#ifdef _WIN32
	return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
#else
	struct stat st;
	return stat(path, &st) == 0;
#endif
}

static int run_cmd(char* const argv[])
{
	if (!argv || !argv[0])
		return -1;

#ifdef _WIN32
	size_t total = 0;
	for (int i = 0; argv[i]; i++)
		total += strlen(argv[i]) + 3;
	char* cmdline = malloc(total + 1);
	if (!cmdline)
		return -1;
	cmdline[0] = '\0';
	for (int i = 0; argv[i]; i++)
	{
		if (i)
			strcat(cmdline, " ");
		strcat(cmdline, "\"");
		strcat(cmdline, argv[i]);
		strcat(cmdline, "\"");
	}

	STARTUPINFOA si = {.cb = sizeof(si)};
	PROCESS_INFORMATION pi = {0};
	if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		fprintf(stderr, "linker: CreateProcess failed for: %s\n", cmdline);
		free(cmdline);
		return -1;
	}
	free(cmdline);
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD code = 1;
	GetExitCodeProcess(pi.hProcess, &code);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return (int)code;

#else
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
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

typedef void (*file_cb)(const char* srcpath, void* ctx);

static void foreach_c_file_in_dir(const char* dir, file_cb cb, void* ctx)
{
#ifdef _WIN32
	char pattern[4096];
	snprintf(pattern, sizeof(pattern), "%s\\*.c", dir);
	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileA(pattern, &fd);
	if (h == INVALID_HANDLE_VALUE)
		return;
	do
	{
		char srcpath[4096];
		snprintf(srcpath, sizeof(srcpath), "%s/%s", dir, fd.cFileName);
		cb(srcpath, ctx);
	} while (FindNextFileA(h, &fd));
	FindClose(h);
#else
	DIR* d = opendir(dir);
	if (!d)
	{
		fprintf(stderr, "linker: failed to open directory %s: %s\n", dir, strerror(errno));
		return;
	}
	struct dirent* e;
	while ((e = readdir(d)) != NULL)
	{
		size_t l = strlen(e->d_name);
		if (l > 2 && strcmp(e->d_name + l - 2, ".c") == 0)
		{
			char srcpath[4096];
			snprintf(srcpath, sizeof(srcpath), "%s/%s", dir, e->d_name);
			cb(srcpath, ctx);
		}
	}
	closedir(d);
#endif
}

static char** split_args(const char* str, size_t* out_count, char** out_storage)
{
	*out_count = 0;
	*out_storage = NULL;
	if (!str || !str[0])
		return NULL;
	char* copy = strdup(str);
	if (!copy)
		return NULL;
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

static char* trim_whitespace(char* s)
{
	if (!s)
		return s;
	while (*s && isspace((unsigned char)*s))
		s++;
	char* end = s + strlen(s);
	while (end > s && isspace((unsigned char)*(end - 1)))
		end--;
	*end = '\0';
	return s;
}

static int ends_with(const char* s, const char* suffix)
{
	if (!s || !suffix)
		return 0;
	size_t ls = strlen(s), lsu = strlen(suffix);
	return ls >= lsu && strcmp(s + ls - lsu, suffix) == 0;
}

int linker_link_with_clang(const char* input_ll_path, const char* output_path, const char* libs)
{
	if (!input_ll_path || !output_path)
		return -1;
	int r;
	if (libs && libs[0])
	{
		size_t lib_count = 0;
		char* lib_storage = NULL;
		char** lib_args = split_args(libs, &lib_count, &lib_storage);
		if (!lib_args)
			return -1;
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

typedef struct
{
	char*** link_items;
	size_t* link_count;
	char*** temp_objs;
	size_t* temp_count;
	int* tmpid;
	int pid;
	int error;
} bundle_ctx;

static void compile_c_file(const char* srcpath, void* vctx)
{
	bundle_ctx* ctx = (bundle_ctx*)vctx;
	if (ctx->error)
		return;
	char tmpobj[4096];
	snprintf(tmpobj, sizeof(tmpobj), "%s/adan_bundle_%d_%d.o", get_tmp_dir(), ctx->pid,
	         (*ctx->tmpid)++);
	size_t cmdlen = strlen(srcpath) + strlen(tmpobj) + 64;
	char* cmd = malloc(cmdlen);
	if (!cmd)
	{
		ctx->error = 1;
		return;
	}
	snprintf(cmd, cmdlen, "clang -c %s -o %s", srcpath, tmpobj);
	size_t cmd_argc = 0;
	char* cmd_storage = NULL;
	char** cmd_argv = split_args(cmd, &cmd_argc, &cmd_storage);
	int r = -1;
	if (cmd_argv)
	{
		r = run_cmd(cmd_argv);
		free(cmd_argv);
		free(cmd_storage);
	}
	free(cmd);
	if (r != 0)
	{
		ctx->error = 1;
		return;
	}
	*ctx->link_items = realloc(*ctx->link_items, sizeof(char*) * (*ctx->link_count + 1));
	(*ctx->link_items)[(*ctx->link_count)++] = strdup(tmpobj);
	*ctx->temp_objs = realloc(*ctx->temp_objs, sizeof(char*) * (*ctx->temp_count + 1));
	(*ctx->temp_objs)[(*ctx->temp_count)++] = strdup(tmpobj);
}

int linker_link_and_bundle(const char* input_ll_path, const char* output_path, const char* libs,
                           const char* bundle_csv)
{
	if (!input_ll_path || !output_path)
		return -1;
	if (!bundle_csv || bundle_csv[0] == '\0')
		return linker_link_with_clang(input_ll_path, output_path, libs);

	char* dup = strdup(bundle_csv);
	if (!dup)
		return -1;
	char* saveptr = NULL;
	char* tok = strtok_r(dup, ",", &saveptr);
	char** link_items = NULL;
	size_t link_count = 0;
	char** temp_objs = NULL;
	size_t temp_count = 0;
	int pid = (int)getpid(), tmpid = 0;

	bundle_ctx ctx = {&link_items, &link_count, &temp_objs, &temp_count, &tmpid, pid, 0};

	while (tok)
	{
		char* path = trim_whitespace(tok);
		if (!path[0])
		{
			tok = strtok_r(NULL, ",", &saveptr);
			continue;
		}

		if (path_is_dir(path))
		{
			foreach_c_file_in_dir(path, compile_c_file, &ctx);
			if (ctx.error)
				goto cleanup_error;
		}
		else if (ends_with(path, ".c"))
		{
			compile_c_file(path, &ctx);
			if (ctx.error)
				goto cleanup_error;
		}
		else if (ends_with(path, ".o") || ends_with(path, ".a"))
		{
			link_items = realloc(link_items, sizeof(char*) * (link_count + 1));
			link_items[link_count++] = strdup(path);
		}
		else
		{
			if (path_exists(path))
			{
				link_items = realloc(link_items, sizeof(char*) * (link_count + 1));
				link_items[link_count++] = strdup(path);
			}
			else
			{
				fprintf(stderr, "linker: unknown bundle path '%s'\n", path);
				goto cleanup_error;
			}
		}
		tok = strtok_r(NULL, ",", &saveptr);
	}

	{
		size_t len =
		    strlen(input_ll_path) + strlen(output_path) + 64 + (libs ? strlen(libs) : 0);
		for (size_t i = 0; i < link_count; ++i)
			len += strlen(link_items[i]) + 3;
		char* linkcmd = malloc(len + 1);
		if (!linkcmd)
			goto cleanup_error;
		strcpy(linkcmd, "clang ");
		strcat(linkcmd, input_ll_path);
		for (size_t i = 0; i < link_count; ++i)
		{
			strcat(linkcmd, " ");
			strcat(linkcmd, link_items[i]);
		}
		strcat(linkcmd, " -o ");
		strcat(linkcmd, output_path);
		if (libs && libs[0])
		{
			strcat(linkcmd, " ");
			strcat(linkcmd, libs);
		}
		size_t cmd_argc = 0;
		char* cmd_storage = NULL;
		char** cmd_argv = split_args(linkcmd, &cmd_argc, &cmd_storage);
		int r = -1;
		if (cmd_argv)
		{
			r = run_cmd(cmd_argv);
			free(cmd_argv);
			free(cmd_storage);
		}
		free(linkcmd);
		for (size_t i = 0; i < temp_count; ++i)
		{
			unlink(temp_objs[i]);
			free(temp_objs[i]);
		}
		for (size_t i = 0; i < link_count; ++i)
			free(link_items[i]);
		free(temp_objs);
		free(link_items);
		free(dup);
		return r;
	}

cleanup_error:
	if (temp_objs)
	{
		for (size_t i = 0; i < temp_count; ++i)
		{
			unlink(temp_objs[i]);
			free(temp_objs[i]);
		}
	}
	if (link_items)
	{
		for (size_t i = 0; i < link_count; ++i)
			free(link_items[i]);
	}
	free(temp_objs);
	free(link_items);
	free(dup);
	return -1;
}

int linker_link_and_bundle_embedded(const char* input_ll_path, const char* output_path,
                                    const char* libs, const char* modules_csv)
{
	if (!input_ll_path || !output_path)
		return -1;
	if (!modules_csv || modules_csv[0] == '\0')
		return linker_link_with_clang(input_ll_path, output_path, libs);

	char* dup = strdup(modules_csv);
	if (!dup)
		return -1;
	int pid = (int)getpid(), tmpid = 0;
	char** objs = NULL;
	size_t obj_count = 0;
	char* saveptr = NULL;
	char* tok = strtok_r(dup, ",", &saveptr);

	while (tok)
	{
		char* module = trim_whitespace(tok);
		if (!module[0])
		{
			tok = strtok_r(NULL, ",", &saveptr);
			continue;
		}

		const char* c_src = embedded_lib_get_c_source(module);
		if (!c_src)
		{
			fprintf(stderr, "linker: no embedded C source for module '%s'.\n", module);
			goto cleanup_embedded_error;
		}

		char tmp_dir[256], tmp_c[300], tmp_obj[256];
		snprintf(tmp_dir, sizeof(tmp_dir), "%s/adan_emb_%d_%d", get_tmp_dir(), pid, tmpid);
		snprintf(tmp_c, sizeof(tmp_c), "%s/source.c", tmp_dir);
		snprintf(tmp_obj, sizeof(tmp_obj), "%s/adan_emb_%d_%d.o", get_tmp_dir(), pid,
		         tmpid++);

		if (mkdir(tmp_dir, 0700) != 0)
		{
			fprintf(stderr, "linker: failed to create temp dir for '%s'.\n", module);
			goto cleanup_embedded_error;
		}

		FILE* f = fopen(tmp_c, "w");
		if (!f)
		{
			fprintf(stderr, "linker: failed to write embedded source for '%s'.\n",
			        module);
			rmdir(tmp_dir);
			goto cleanup_embedded_error;
		}
		fputs(c_src, f);
		fclose(f);

		const char* h_filename = embedded_lib_get_h_filename(module);
		const char* h_src = embedded_lib_get_h_source(module);
		if (h_filename && h_src)
		{
			char tmp_h[512];
			snprintf(tmp_h, sizeof(tmp_h), "%s/%s", tmp_dir, h_filename);
			FILE* fh = fopen(tmp_h, "w");
			if (fh)
			{
				fputs(h_src, fh);
				fclose(fh);
			}
		}

		size_t cmdlen = strlen(tmp_c) + strlen(tmp_dir) + strlen(tmp_obj) + 64;
		char* cmd = malloc(cmdlen);
		if (!cmd)
		{
			unlink(tmp_c);
			goto cleanup_embedded_error;
		}
		snprintf(cmd, cmdlen, "clang -c %s -I %s -o %s", tmp_c, tmp_dir, tmp_obj);
		size_t cmd_argc = 0;
		char* cmd_storage = NULL;
		char** cmd_argv = split_args(cmd, &cmd_argc, &cmd_storage);
		int r = -1;
		if (cmd_argv)
		{
			r = run_cmd(cmd_argv);
			free(cmd_argv);
			free(cmd_storage);
		}
		free(cmd);
		unlink(tmp_c);
		if (h_filename && h_src)
		{
			char tmp_h[512];
			snprintf(tmp_h, sizeof(tmp_h), "%s/%s", tmp_dir, h_filename);
			unlink(tmp_h);
		}
		rmdir(tmp_dir);
		if (r != 0)
		{
			unlink(tmp_obj);
			goto cleanup_embedded_error;
		}
		objs = realloc(objs, sizeof(char*) * (obj_count + 1));
		objs[obj_count++] = strdup(tmp_obj);
		tok = strtok_r(NULL, ",", &saveptr);
	}

	{
		size_t llen =
		    strlen(input_ll_path) + strlen(output_path) + 64 + (libs ? strlen(libs) : 0);
		for (size_t i = 0; i < obj_count; ++i)
			llen += strlen(objs[i]) + 2;
		char* linkcmd = malloc(llen + 1);
		if (!linkcmd)
			goto cleanup_embedded_error;
		strcpy(linkcmd, "clang ");
		strcat(linkcmd, input_ll_path);
		for (size_t i = 0; i < obj_count; ++i)
		{
			strcat(linkcmd, " ");
			strcat(linkcmd, objs[i]);
		}
		strcat(linkcmd, " -o ");
		strcat(linkcmd, output_path);
		if (libs && libs[0])
		{
			strcat(linkcmd, " ");
			strcat(linkcmd, libs);
		}
		size_t cmd_argc = 0;
		char* cmd_storage = NULL;
		char** cmd_argv = split_args(linkcmd, &cmd_argc, &cmd_storage);
		int r = -1;
		if (cmd_argv)
		{
			r = run_cmd(cmd_argv);
			free(cmd_argv);
			free(cmd_storage);
		}
		free(linkcmd);
		for (size_t i = 0; i < obj_count; ++i)
		{
			unlink(objs[i]);
			free(objs[i]);
		}
		free(objs);
		free(dup);
		return r;
	}

cleanup_embedded_error:
	if (objs)
	{
		for (size_t i = 0; i < obj_count; ++i)
		{
			unlink(objs[i]);
			free(objs[i]);
		}
		free(objs);
	}
	free(dup);
	return -1;
}

int linker_emit_bitcode_from_ll(const char* ll_path, const char* bc_path)
{
	if (!ll_path || !bc_path)
		return -1;
	char* argv[] = {"llvm-as", "-o", (char*)bc_path, (char*)ll_path, NULL};
	return run_cmd(argv);
}

int linker_llvm_link_bitcode(const char** inputs, size_t ninputs, const char* out_bc_path)
{
	if (!inputs || ninputs == 0 || !out_bc_path)
		return -1;
	size_t argc = 3 + ninputs + 1;
	char** argv = (char**)malloc(argc * sizeof(char*));
	if (!argv)
		return -1;
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
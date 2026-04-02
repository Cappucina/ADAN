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

typedef struct
{
	char** args;
	size_t count;
	size_t capacity;
} arg_list;

static int append_string_item(char*** items, size_t* count, const char* value)
{
	char** resized;
	char* copy;

	if (!items || !count || !value)
	{
		return -1;
	}

	resized = realloc(*items, sizeof(char*) * (*count + 1));
	if (!resized)
	{
		return -1;
	}

	copy = strdup(value);
	if (!copy)
	{
		return -1;
	}

	*items = resized;
	(*items)[(*count)++] = copy;
	return 0;
}

static void free_string_items(char** items, size_t count)
{
	if (!items)
	{
		return;
	}

	for (size_t i = 0; i < count; ++i)
	{
		free(items[i]);
	}
	free(items);
}

static void cleanup_temp_objects(char** temp_objs, size_t temp_count)
{
	if (!temp_objs)
	{
		return;
	}

	for (size_t i = 0; i < temp_count; ++i)
	{
		if (temp_objs[i])
		{
			unlink(temp_objs[i]);
			free(temp_objs[i]);
		}
	}
	free(temp_objs);
}

static int arg_list_append(arg_list* list, const char* value)
{
	char** resized;
	char* copy;
	size_t new_capacity;

	if (!list || !value)
	{
		return -1;
	}

	if (list->count + 2 > list->capacity)
	{
		new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
		while (new_capacity < list->count + 2)
		{
			new_capacity *= 2;
		}
		resized = realloc(list->args, sizeof(char*) * new_capacity);
		if (!resized)
		{
			return -1;
		}
		list->args = resized;
		list->capacity = new_capacity;
	}

	copy = strdup(value);
	if (!copy)
	{
		return -1;
	}

	list->args[list->count++] = copy;
	list->args[list->count] = NULL;
	return 0;
}

static void arg_list_free(arg_list* list)
{
	if (!list)
	{
		return;
	}

	for (size_t i = 0; i < list->count; ++i)
	{
		free(list->args[i]);
	}
	free(list->args);
	list->args = NULL;
	list->count = 0;
	list->capacity = 0;
}

static int arg_list_append_raw_args(arg_list* list, const char* raw_args)
{
	size_t arg_count = 0;
	char* storage = NULL;
	char** split = NULL;

	if (!raw_args || raw_args[0] == '\0')
	{
		return 0;
	}

	split = split_args(raw_args, &arg_count, &storage);
	if (!split)
	{
		return -1;
	}

	for (size_t i = 0; i < arg_count; ++i)
	{
		if (arg_list_append(list, split[i]) != 0)
		{
			free(split);
			free(storage);
			return -1;
		}
	}

	free(split);
	free(storage);
	return 0;
}

static int arg_list_append_search_paths(arg_list* list, const char* csv)
{
	char* dup;
	char* saveptr = NULL;
	char* tok;

	if (!csv || csv[0] == '\0')
	{
		return 0;
	}

	dup = strdup(csv);
	if (!dup)
	{
		return -1;
	}

	tok = strtok_r(dup, ",", &saveptr);
	while (tok)
	{
		char* path = trim_whitespace(tok);
		if (path[0] != '\0')
		{
			if (arg_list_append(list, "-L") != 0 ||
			    arg_list_append(list, path) != 0)
			{
				free(dup);
				return -1;
			}
		}
		tok = strtok_r(NULL, ",", &saveptr);
	}

	free(dup);
	return 0;
}

static int arg_list_append_native_libraries(arg_list* list, const char* csv)
{
	char* dup;
	char* saveptr = NULL;
	char* tok;

	if (!csv || csv[0] == '\0')
	{
		return 0;
	}

	dup = strdup(csv);
	if (!dup)
	{
		return -1;
	}

	tok = strtok_r(dup, ",", &saveptr);
	while (tok)
	{
		char* item = trim_whitespace(tok);
		if (item[0] != '\0')
		{
			if (item[0] == '-' || path_exists(item) || strchr(item, '/') ||
			    strchr(item, '\\') || ends_with(item, ".a") ||
			    ends_with(item, ".o") || ends_with(item, ".obj") ||
			    ends_with(item, ".lib") || ends_with(item, ".so") ||
			    ends_with(item, ".dylib"))
			{
				if (arg_list_append(list, item) != 0)
				{
					free(dup);
					return -1;
				}
			}
			else
			{
				size_t flag_len = strlen(item) + 3;
				char* lib_flag = malloc(flag_len);
				if (!lib_flag)
				{
					free(dup);
					return -1;
				}
				snprintf(lib_flag, flag_len, "-l%s", item);
				if (arg_list_append(list, lib_flag) != 0)
				{
					free(lib_flag);
					free(dup);
					return -1;
				}
				free(lib_flag);
			}
		}
		tok = strtok_r(NULL, ",", &saveptr);
	}

	free(dup);
	return 0;
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
	if (append_string_item(ctx->link_items, ctx->link_count, tmpobj) != 0 ||
	    append_string_item(ctx->temp_objs, ctx->temp_count, tmpobj) != 0)
	{
		ctx->error = 1;
	}
}

static int collect_bundle_link_items(const char* bundle_csv, char*** link_items,
	                                 size_t* link_count, char*** temp_objs,
	                                 size_t* temp_count)
{
	char* dup;
	char* saveptr = NULL;
	char* tok;
	int pid = (int)getpid();
	int tmpid = 0;
	bundle_ctx ctx = {link_items, link_count, temp_objs, temp_count, &tmpid, pid, 0};

	if (!bundle_csv || bundle_csv[0] == '\0')
	{
		return 0;
	}

	dup = strdup(bundle_csv);
	if (!dup)
	{
		return -1;
	}

	tok = strtok_r(dup, ",", &saveptr);
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
			{
				free(dup);
				return -1;
			}
		}
		else if (ends_with(path, ".c"))
		{
			compile_c_file(path, &ctx);
			if (ctx.error)
			{
				free(dup);
				return -1;
			}
		}
		else if (ends_with(path, ".o") || ends_with(path, ".a") ||
		         ends_with(path, ".obj") || ends_with(path, ".lib") ||
		         path_exists(path))
		{
			if (append_string_item(link_items, link_count, path) != 0)
			{
				free(dup);
				return -1;
			}
		}
		else
		{
			fprintf(stderr, "linker: unknown bundle path '%s'\n", path);
			free(dup);
			return -1;
		}

		tok = strtok_r(NULL, ",", &saveptr);
	}

	free(dup);
	return 0;
}

static int collect_embedded_link_items(const char* modules_csv, char*** link_items,
	                                   size_t* link_count, char*** temp_objs,
	                                   size_t* temp_count)
{
	char* dup;
	char* saveptr = NULL;
	char* tok;
	int pid = (int)getpid();
	int tmpid = 0;

	if (!modules_csv || modules_csv[0] == '\0')
	{
		return 0;
	}

	dup = strdup(modules_csv);
	if (!dup)
	{
		return -1;
	}

	tok = strtok_r(dup, ",", &saveptr);
	while (tok)
	{
		char* module = trim_whitespace(tok);
		const char* c_src;
		const char* h_filename;
		const char* h_src;
		char tmp_dir[256];
		char tmp_c[300];
		char tmp_obj[256];
		FILE* f;

		if (!module[0])
		{
			tok = strtok_r(NULL, ",", &saveptr);
			continue;
		}

		c_src = embedded_lib_get_c_source(module);
		if (!c_src)
		{
			fprintf(stderr, "linker: no embedded C source for module '%s'.\n",
			        module);
			free(dup);
			return -1;
		}

		snprintf(tmp_dir, sizeof(tmp_dir), "%s/adan_emb_%d_%d", get_tmp_dir(), pid,
		         tmpid);
		snprintf(tmp_c, sizeof(tmp_c), "%s/source.c", tmp_dir);
		snprintf(tmp_obj, sizeof(tmp_obj), "%s/adan_emb_%d_%d.o", get_tmp_dir(), pid,
		         tmpid++);

		if (mkdir(tmp_dir, 0700) != 0)
		{
			fprintf(stderr, "linker: failed to create temp dir for '%s'.\n", module);
			free(dup);
			return -1;
		}

		f = fopen(tmp_c, "w");
		if (!f)
		{
			fprintf(stderr, "linker: failed to write embedded source for '%s'.\n",
			        module);
			rmdir(tmp_dir);
			free(dup);
			return -1;
		}
		fputs(c_src, f);
		fclose(f);

		h_filename = embedded_lib_get_h_filename(module);
		h_src = embedded_lib_get_h_source(module);
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

		{
			size_t cmdlen = strlen(tmp_c) + strlen(tmp_dir) + strlen(tmp_obj) + 64;
			char* cmd = malloc(cmdlen);
			size_t cmd_argc = 0;
			char* cmd_storage = NULL;
			char** cmd_argv;
			int r = -1;

			if (!cmd)
			{
				unlink(tmp_c);
				rmdir(tmp_dir);
				free(dup);
				return -1;
			}

			snprintf(cmd, cmdlen, "clang -c %s -I %s -o %s", tmp_c, tmp_dir, tmp_obj);
			cmd_argv = split_args(cmd, &cmd_argc, &cmd_storage);
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
				free(dup);
				return -1;
			}
		}

		if (append_string_item(link_items, link_count, tmp_obj) != 0 ||
		    append_string_item(temp_objs, temp_count, tmp_obj) != 0)
		{
			unlink(tmp_obj);
			free(dup);
			return -1;
		}

		tok = strtok_r(NULL, ",", &saveptr);
	}

	free(dup);
	return 0;
}

int linker_link(const char* input_ll_path, const char* output_path,
	            const LinkerConfig* config)
{
	char** link_items = NULL;
	size_t link_count = 0;
	char** temp_objs = NULL;
	size_t temp_count = 0;
	arg_list argv = {0};
	int result = -1;

	if (!input_ll_path || !output_path)
	{
		return -1;
	}

	if (config)
	{
		if (collect_bundle_link_items(config->bundle_csv, &link_items, &link_count,
		                             &temp_objs, &temp_count) != 0)
		{
			goto cleanup;
		}
		if (collect_embedded_link_items(config->embedded_modules_csv, &link_items,
		                               &link_count, &temp_objs, &temp_count) != 0)
		{
			goto cleanup;
		}
	}

	if (arg_list_append(&argv, "clang") != 0 ||
	    arg_list_append(&argv, input_ll_path) != 0)
	{
		goto cleanup;
	}

	for (size_t i = 0; i < link_count; ++i)
	{
		if (arg_list_append(&argv, link_items[i]) != 0)
		{
			goto cleanup;
		}
	}

	if (arg_list_append(&argv, "-o") != 0 ||
	    arg_list_append(&argv, output_path) != 0)
	{
		goto cleanup;
	}

	if (config)
	{
		if (arg_list_append_search_paths(&argv,
		                                config->native_search_paths_csv) != 0 ||
		    arg_list_append_native_libraries(&argv,
		                                   config->native_libraries_csv) != 0 ||
		    arg_list_append_raw_args(&argv, config->raw_link_args) != 0)
		{
			goto cleanup;
		}
	}

	result = run_cmd(argv.args);

cleanup:
	cleanup_temp_objects(temp_objs, temp_count);
	free_string_items(link_items, link_count);
	arg_list_free(&argv);
	return result;
}

int linker_link_with_clang(const char* input_ll_path, const char* output_path, const char* libs)
{
	LinkerConfig config = {0};
	config.raw_link_args = libs;
	return linker_link(input_ll_path, output_path, &config);
}

int linker_link_and_bundle(const char* input_ll_path, const char* output_path, const char* libs,
	                       const char* bundle_csv)
{
	LinkerConfig config = {0};
	config.raw_link_args = libs;
	config.bundle_csv = bundle_csv;
	return linker_link(input_ll_path, output_path, &config);
}

int linker_link_and_bundle_embedded(const char* input_ll_path, const char* output_path,
	                                const char* libs, const char* modules_csv)
{
	LinkerConfig config = {0};
	config.raw_link_args = libs;
	config.embedded_modules_csv = modules_csv;
	return linker_link(input_ll_path, output_path, &config);
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
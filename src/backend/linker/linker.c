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

static int is_path_separator(char ch)
{
	return ch == '/' || ch == '\\';
}

static const char* path_basename_ptr(const char* path)
{
	const char* base = path;

	if (!path)
	{
		return NULL;
	}

	for (const char* cursor = path; *cursor; ++cursor)
	{
		if (is_path_separator(*cursor))
		{
			base = cursor + 1;
		}
	}

	return base;
}

static int path_dirname(const char* path, char* buffer, size_t buffer_size)
{
	size_t length;

	if (!path || !buffer || buffer_size == 0)
	{
		return -1;
	}

	length = strlen(path);
	while (length > 0 && !is_path_separator(path[length - 1]))
	{
		length--;
	}
	while (length > 1 && is_path_separator(path[length - 1]))
	{
		length--;
	}

	if (length == 0)
	{
		if (buffer_size < 2)
		{
			return -1;
		}
		buffer[0] = '.';
		buffer[1] = '\0';
		return 0;
	}

	if (length + 1 > buffer_size)
	{
		return -1;
	}

	memcpy(buffer, path, length);
	buffer[length] = '\0';
	return 0;
}

static int path_join(char* buffer, size_t buffer_size, const char* left,
	                 const char* right)
{
	int written;
	int has_separator;

	if (!buffer || buffer_size == 0 || !left || !right)
	{
		return -1;
	}

	has_separator = left[0] != '\0' && is_path_separator(left[strlen(left) - 1]);
	written = snprintf(buffer, buffer_size, has_separator ? "%s%s" : "%s/%s",
	                  left, right);
	return written >= 0 && (size_t)written < buffer_size ? 0 : -1;
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

static int arg_list_append(arg_list* list, const char* value);
static void arg_list_free(arg_list* list);

static int arg_list_append_flagged_dir(arg_list* list, const char* flag,
	                                   const char* path)
{
	if (!list || !flag || !path || path[0] == '\0' || !path_is_dir(path))
	{
		return 0;
	}

	if (arg_list_append(list, flag) != 0 || arg_list_append(list, path) != 0)
	{
		return -1;
	}

	return 0;
}

static int csv_mentions_crypto(const char* csv)
{
	char* dup;
	char* saveptr = NULL;
	char* tok;
	int found = 0;

	if (!csv || csv[0] == '\0')
	{
		return 0;
	}

	dup = strdup(csv);
	if (!dup)
	{
		return 0;
	}

	tok = strtok_r(dup, ",", &saveptr);
	while (tok)
	{
		char* item = trim_whitespace(tok);
		const char* base = path_basename_ptr(item);

		if (strcmp(item, "crypto") == 0 || strcmp(item, "libcrypto") == 0 ||
		    strcmp(item, "-lcrypto") == 0 || strcmp(item, "-llibcrypto") == 0 ||
		    (base && (strcmp(base, "libcrypto.lib") == 0 ||
		              strcmp(base, "crypto.lib") == 0 ||
		              strcmp(base, "libcrypto.a") == 0 ||
		              strcmp(base, "libcrypto.so") == 0 ||
		              strcmp(base, "libcrypto.dylib") == 0)))
		{
			found = 1;
			break;
		}

		tok = strtok_r(NULL, ",", &saveptr);
	}

	free(dup);
	return found;
}

static int append_inferred_include_from_search_path(arg_list* list,
	                                                const char* search_path)
{
	char candidate[1024];
	char parent[1024];
	const char* base;

	if (!list || !search_path || search_path[0] == '\0' || !path_is_dir(search_path))
	{
		return 0;
	}

	base = path_basename_ptr(search_path);
	if (base && strcmp(base, "include") == 0)
	{
		return arg_list_append_flagged_dir(list, "-I", search_path);
	}

	if (path_join(candidate, sizeof(candidate), search_path, "include") == 0 &&
	    arg_list_append_flagged_dir(list, "-I", candidate) != 0)
	{
		return -1;
	}

	if (base && (strcmp(base, "lib") == 0 || strcmp(base, "lib64") == 0 ||
	             strcmp(base, "Lib") == 0 || strcmp(base, "libs") == 0 ||
	             strcmp(base, "Libraries") == 0))
	{
		if (path_dirname(search_path, parent, sizeof(parent)) == 0 &&
		    path_join(candidate, sizeof(candidate), parent, "include") == 0 &&
		    arg_list_append_flagged_dir(list, "-I", candidate) != 0)
		{
			return -1;
		}
	}

	return 0;
}

static int arg_list_append_inferred_include_paths(arg_list* list, const char* csv)
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
		if (path[0] != '\0' && append_inferred_include_from_search_path(list, path) != 0)
		{
			free(dup);
			return -1;
		}
		tok = strtok_r(NULL, ",", &saveptr);
	}

	free(dup);
	return 0;
}

static int append_openssl_root_dirs(arg_list* include_args, arg_list* link_args,
	                                const char* root)
{
	char candidate[1024];
	const char* lib_subdirs[] = {
		"lib",
		"lib64",
#ifdef _WIN32
		"lib/VC/x64/MD",
		"lib/VC/x64/MT",
		"lib/VC/x86/MD",
		"lib/VC/x86/MT",
		"lib/VC/static",
		"lib/MinGW",
#endif
	};

	if (!root || root[0] == '\0')
	{
		return 0;
	}

	if (include_args && path_join(candidate, sizeof(candidate), root, "include") == 0 &&
	    arg_list_append_flagged_dir(include_args, "-I", candidate) != 0)
	{
		return -1;
	}

	if (link_args)
	{
		for (size_t i = 0; i < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++i)
		{
			if (path_join(candidate, sizeof(candidate), root, lib_subdirs[i]) == 0 &&
			    arg_list_append_flagged_dir(link_args, "-L", candidate) != 0)
			{
				return -1;
			}
		}
	}

	return 0;
}

static int append_openssl_detected_dirs(arg_list* include_args, arg_list* link_args)
{
	const char* env;
	char candidate[1024];

	env = getenv("OPENSSL_ROOT_DIR");
	if (append_openssl_root_dirs(include_args, link_args, env) != 0)
	{
		return -1;
	}

	env = getenv("OPENSSL_DIR");
	if (append_openssl_root_dirs(include_args, link_args, env) != 0)
	{
		return -1;
	}

	env = getenv("OPENSSL_INCLUDE_DIR");
	if (include_args && arg_list_append_flagged_dir(include_args, "-I", env) != 0)
	{
		return -1;
	}

	env = getenv("OPENSSL_LIB_DIR");
	if (link_args && arg_list_append_flagged_dir(link_args, "-L", env) != 0)
	{
		return -1;
	}

#ifdef __APPLE__
	env = getenv("HOMEBREW_PREFIX");
	if (env && env[0] != '\0')
	{
		if (path_join(candidate, sizeof(candidate), env, "opt/openssl@3") == 0 &&
		    append_openssl_root_dirs(include_args, link_args, candidate) != 0)
		{
			return -1;
		}
		if (path_join(candidate, sizeof(candidate), env, "opt/openssl") == 0 &&
		    append_openssl_root_dirs(include_args, link_args, candidate) != 0)
		{
			return -1;
		}
	}

	if (append_openssl_root_dirs(include_args, link_args,
	                            "/opt/homebrew/opt/openssl@3") != 0 ||
	    append_openssl_root_dirs(include_args, link_args,
	                            "/opt/homebrew/opt/openssl") != 0 ||
	    append_openssl_root_dirs(include_args, link_args,
	                            "/usr/local/opt/openssl@3") != 0 ||
	    append_openssl_root_dirs(include_args, link_args,
	                            "/usr/local/opt/openssl") != 0 ||
	    append_openssl_root_dirs(include_args, link_args,
	                            "/opt/local/libexec/openssl3") != 0 ||
	    append_openssl_root_dirs(include_args, link_args,
	                            "/opt/local/libexec/openssl11") != 0)
	{
		return -1;
	}
#endif

#ifdef _WIN32
	env = getenv("ProgramFiles");
	if (env && env[0] != '\0')
	{
		if (path_join(candidate, sizeof(candidate), env, "OpenSSL-Win64") == 0 &&
		    append_openssl_root_dirs(include_args, link_args, candidate) != 0)
		{
			return -1;
		}
		if (path_join(candidate, sizeof(candidate), env, "OpenSSL-Win32") == 0 &&
		    append_openssl_root_dirs(include_args, link_args, candidate) != 0)
		{
			return -1;
		}
	}

	env = getenv("ProgramFiles(x86)");
	if (env && env[0] != '\0' &&
	    path_join(candidate, sizeof(candidate), env, "OpenSSL-Win32") == 0 &&
	    append_openssl_root_dirs(include_args, link_args, candidate) != 0)
	{
		return -1;
	}

	env = getenv("ProgramW6432");
	if (env && env[0] != '\0' &&
	    path_join(candidate, sizeof(candidate), env, "OpenSSL-Win64") == 0 &&
	    append_openssl_root_dirs(include_args, link_args, candidate) != 0)
	{
		return -1;
	}

	env = getenv("USERPROFILE");
	if (env && env[0] != '\0' &&
	    path_join(candidate, sizeof(candidate), env, "scoop/apps/openssl/current") == 0 &&
	    append_openssl_root_dirs(include_args, link_args, candidate) != 0)
	{
		return -1;
	}

	if (append_openssl_root_dirs(include_args, link_args, "C:/OpenSSL-Win64") != 0 ||
	    append_openssl_root_dirs(include_args, link_args, "C:/OpenSSL-Win32") != 0)
	{
		return -1;
	}
#endif

	return 0;
}

#ifdef _WIN32
static int find_library_in_directory(const char* directory,
	                                 const char* const* filenames,
	                                 size_t filename_count,
	                                 char* buffer,
	                                 size_t buffer_size)
{
	char candidate[1024];

	if (!directory || directory[0] == '\0' || !path_is_dir(directory))
	{
		return 0;
	}

	for (size_t i = 0; i < filename_count; ++i)
	{
		if (path_join(candidate, sizeof(candidate), directory, filenames[i]) == 0 &&
		    path_exists(candidate))
		{
			snprintf(buffer, buffer_size, "%s", candidate);
			return 1;
		}
	}

	return 0;
}

static int find_library_in_csv_directories(const char* csv,
	                                       const char* const* filenames,
	                                       size_t filename_count,
	                                       char* buffer,
	                                       size_t buffer_size)
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
		return 0;
	}

	tok = strtok_r(dup, ",", &saveptr);
	while (tok)
	{
		char* directory = trim_whitespace(tok);
		if (directory[0] != '\0' &&
		    find_library_in_directory(directory, filenames, filename_count, buffer,
		                             buffer_size))
		{
			free(dup);
			return 1;
		}
		tok = strtok_r(NULL, ",", &saveptr);
	}

	free(dup);
	return 0;
}

static int find_windows_openssl_library(const char* search_paths_csv,
	                                   char* buffer,
	                                   size_t buffer_size)
{
	const char* filenames[] = {"libcrypto.lib", "crypto.lib"};
	const char* env;
	char candidate[1024];
	const char* const roots[] = {
		"C:/OpenSSL-Win64",
		"C:/OpenSSL-Win32",
	};

	if (!buffer || buffer_size == 0)
	{
		return 0;
	}

	if (find_library_in_csv_directories(search_paths_csv, filenames,
	                                   sizeof(filenames) / sizeof(filenames[0]),
	                                   buffer, buffer_size))
	{
		return 1;
	}

	env = getenv("OPENSSL_LIB_DIR");
	if (find_library_in_directory(env, filenames,
	                              sizeof(filenames) / sizeof(filenames[0]),
	                              buffer, buffer_size))
	{
		return 1;
	}

	env = getenv("OPENSSL_ROOT_DIR");
	if (env && append_openssl_root_dirs(NULL, NULL, env) == 0)
	{
		const char* lib_subdirs[] = {
			"lib",
			"lib64",
			"lib/VC/x64/MD",
			"lib/VC/x64/MT",
			"lib/VC/x86/MD",
			"lib/VC/x86/MT",
			"lib/VC/static",
			"lib/MinGW",
		};
		for (size_t i = 0; i < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++i)
		{
			if (path_join(candidate, sizeof(candidate), env, lib_subdirs[i]) == 0 &&
			    find_library_in_directory(candidate, filenames,
			                          sizeof(filenames) / sizeof(filenames[0]),
			                          buffer, buffer_size))
			{
				return 1;
			}
		}
	}

	env = getenv("OPENSSL_DIR");
	if (env)
	{
		const char* lib_subdirs[] = {
			"lib",
			"lib64",
			"lib/VC/x64/MD",
			"lib/VC/x64/MT",
			"lib/VC/x86/MD",
			"lib/VC/x86/MT",
			"lib/VC/static",
			"lib/MinGW",
		};
		for (size_t i = 0; i < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++i)
		{
			if (path_join(candidate, sizeof(candidate), env, lib_subdirs[i]) == 0 &&
			    find_library_in_directory(candidate, filenames,
			                          sizeof(filenames) / sizeof(filenames[0]),
			                          buffer, buffer_size))
			{
				return 1;
			}
		}
	}

	env = getenv("ProgramFiles");
	if (env)
	{
		if (path_join(candidate, sizeof(candidate), env, "OpenSSL-Win64") == 0)
		{
			const char* lib_subdirs[] = {
				"lib",
				"lib64",
				"lib/VC/x64/MD",
				"lib/VC/x64/MT",
				"lib/VC/x86/MD",
				"lib/VC/x86/MT",
				"lib/VC/static",
				"lib/MinGW",
			};
			for (size_t i = 0; i < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++i)
			{
				char lib_dir[1024];
				if (path_join(lib_dir, sizeof(lib_dir), candidate, lib_subdirs[i]) == 0 &&
				    find_library_in_directory(lib_dir, filenames,
				                          sizeof(filenames) / sizeof(filenames[0]),
				                          buffer, buffer_size))
				{
					return 1;
				}
			}
		}
		if (path_join(candidate, sizeof(candidate), env, "OpenSSL-Win32") == 0)
		{
			const char* lib_subdirs[] = {
				"lib",
				"lib64",
				"lib/VC/x64/MD",
				"lib/VC/x64/MT",
				"lib/VC/x86/MD",
				"lib/VC/x86/MT",
				"lib/VC/static",
				"lib/MinGW",
			};
			for (size_t i = 0; i < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++i)
			{
				char lib_dir[1024];
				if (path_join(lib_dir, sizeof(lib_dir), candidate, lib_subdirs[i]) == 0 &&
				    find_library_in_directory(lib_dir, filenames,
				                          sizeof(filenames) / sizeof(filenames[0]),
				                          buffer, buffer_size))
				{
					return 1;
				}
			}
		}
	}

	env = getenv("ProgramFiles(x86)");
	if (env && path_join(candidate, sizeof(candidate), env, "OpenSSL-Win32") == 0)
	{
		const char* lib_subdirs[] = {
			"lib",
			"lib64",
			"lib/VC/x64/MD",
			"lib/VC/x64/MT",
			"lib/VC/x86/MD",
			"lib/VC/x86/MT",
			"lib/VC/static",
			"lib/MinGW",
		};
		for (size_t i = 0; i < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++i)
		{
			char lib_dir[1024];
			if (path_join(lib_dir, sizeof(lib_dir), candidate, lib_subdirs[i]) == 0 &&
			    find_library_in_directory(lib_dir, filenames,
			                          sizeof(filenames) / sizeof(filenames[0]),
			                          buffer, buffer_size))
			{
				return 1;
			}
		}
	}

	env = getenv("ProgramW6432");
	if (env && path_join(candidate, sizeof(candidate), env, "OpenSSL-Win64") == 0)
	{
		const char* lib_subdirs[] = {
			"lib",
			"lib64",
			"lib/VC/x64/MD",
			"lib/VC/x64/MT",
			"lib/VC/x86/MD",
			"lib/VC/x86/MT",
			"lib/VC/static",
			"lib/MinGW",
		};
		for (size_t i = 0; i < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++i)
		{
			char lib_dir[1024];
			if (path_join(lib_dir, sizeof(lib_dir), candidate, lib_subdirs[i]) == 0 &&
			    find_library_in_directory(lib_dir, filenames,
			                          sizeof(filenames) / sizeof(filenames[0]),
			                          buffer, buffer_size))
			{
				return 1;
			}
		}
	}

	env = getenv("USERPROFILE");
	if (env && path_join(candidate, sizeof(candidate), env, "scoop/apps/openssl/current") == 0)
	{
		const char* lib_subdirs[] = {
			"lib",
			"lib64",
			"lib/VC/x64/MD",
			"lib/VC/x64/MT",
			"lib/VC/x86/MD",
			"lib/VC/x86/MT",
			"lib/VC/static",
			"lib/MinGW",
		};
		for (size_t i = 0; i < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++i)
		{
			char lib_dir[1024];
			if (path_join(lib_dir, sizeof(lib_dir), candidate, lib_subdirs[i]) == 0 &&
			    find_library_in_directory(lib_dir, filenames,
			                          sizeof(filenames) / sizeof(filenames[0]),
			                          buffer, buffer_size))
			{
				return 1;
			}
		}
	}

	for (size_t i = 0; i < sizeof(roots) / sizeof(roots[0]); ++i)
	{
		const char* lib_subdirs[] = {
			"lib",
			"lib64",
			"lib/VC/x64/MD",
			"lib/VC/x64/MT",
			"lib/VC/x86/MD",
			"lib/VC/x86/MT",
			"lib/VC/static",
			"lib/MinGW",
		};
		for (size_t j = 0; j < sizeof(lib_subdirs) / sizeof(lib_subdirs[0]); ++j)
		{
			if (path_join(candidate, sizeof(candidate), roots[i], lib_subdirs[j]) == 0 &&
			    find_library_in_directory(candidate, filenames,
			                          sizeof(filenames) / sizeof(filenames[0]),
			                          buffer, buffer_size))
			{
				return 1;
			}
		}
	}

	return 0;
}
#endif

static int run_clang_compile(const char* srcpath, const char* output_path,
	                        const char* temp_include_dir,
	                        const char* native_libraries_csv,
	                        const char* native_search_paths_csv)
{
	arg_list argv = {0};
	int result = -1;

	if (!srcpath || !output_path)
	{
		return -1;
	}

	if (arg_list_append(&argv, "clang") != 0 ||
	    arg_list_append(&argv, "-c") != 0 ||
	    arg_list_append(&argv, srcpath) != 0)
	{
		goto cleanup;
	}

	if (temp_include_dir && temp_include_dir[0] != '\0' &&
	    (arg_list_append(&argv, "-I") != 0 ||
	     arg_list_append(&argv, temp_include_dir) != 0))
	{
		goto cleanup;
	}

	if (arg_list_append_inferred_include_paths(&argv, native_search_paths_csv) != 0)
	{
		goto cleanup;
	}

	if (csv_mentions_crypto(native_libraries_csv) &&
	    append_openssl_detected_dirs(&argv, NULL) != 0)
	{
		goto cleanup;
	}

	if (arg_list_append(&argv, "-o") != 0 ||
	    arg_list_append(&argv, output_path) != 0)
	{
		goto cleanup;
	}

	result = run_cmd(argv.args);

cleanup:
	arg_list_free(&argv);
	return result;
}

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

static int arg_list_append_native_libraries(arg_list* list, const char* csv,
	                                        const char* search_paths_csv)
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
#ifdef _WIN32
			if (strcmp(item, "crypto") == 0 || strcmp(item, "libcrypto") == 0)
			{
				char resolved_lib[1024];
				if (find_windows_openssl_library(search_paths_csv, resolved_lib,
				                              sizeof(resolved_lib)))
				{
					if (arg_list_append(list, resolved_lib) != 0)
					{
						free(dup);
						return -1;
					}
					tok = strtok_r(NULL, ",", &saveptr);
					continue;
				}
			}
#endif

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
	int r = run_clang_compile(srcpath, tmpobj, NULL, NULL, NULL);
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

static int collect_embedded_link_items(const char* modules_csv,
	                                   const char* native_libraries_csv,
	                                   const char* native_search_paths_csv,
	                                   char*** link_items,
	                                   size_t* link_count,
	                                   char*** temp_objs,
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
			int r = run_clang_compile(tmp_c, tmp_obj, tmp_dir,
			                        native_libraries_csv,
			                        native_search_paths_csv);
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
		if (collect_embedded_link_items(config->embedded_modules_csv,
		                               config->native_libraries_csv,
		                               config->native_search_paths_csv,
		                               &link_items, &link_count,
		                               &temp_objs, &temp_count) != 0)
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
		if (csv_mentions_crypto(config->native_libraries_csv) &&
		    append_openssl_detected_dirs(NULL, &argv) != 0)
		{
			goto cleanup;
		}
		if (arg_list_append_search_paths(&argv,
		                                config->native_search_paths_csv) != 0 ||
		    arg_list_append_native_libraries(&argv,
		                                   config->native_libraries_csv,
		                                   config->native_search_paths_csv) != 0 ||
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
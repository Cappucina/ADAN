#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process.h"

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <pwd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct
{
	char* data;
	size_t length;
	size_t capacity;
} AdnProcessStringBuilder;

static void adn_process_builder_init(AdnProcessStringBuilder* builder)
{
	if (!builder)
	{
		return;
	}
	builder->capacity = 64;
	builder->length = 0;
	builder->data = (char*)calloc(builder->capacity, 1);
}

static void adn_process_builder_reserve(AdnProcessStringBuilder* builder, size_t additional)
{
	if (!builder)
	{
		return;
	}
	if (builder->length + additional + 1 <= builder->capacity)
	{
		return;
	}
	size_t next_capacity = builder->capacity == 0 ? 64 : builder->capacity;
	while (builder->length + additional + 1 > next_capacity)
	{
		next_capacity *= 2;
	}
	char* resized = (char*)realloc(builder->data, next_capacity);
	if (!resized)
	{
		return;
	}
	builder->data = resized;
	builder->capacity = next_capacity;
}

static void adn_process_builder_append_n(AdnProcessStringBuilder* builder, const char* text,
                                         size_t length)
{
	if (!builder || !text)
	{
		return;
	}
	adn_process_builder_reserve(builder, length);
	memcpy(builder->data + builder->length, text, length);
	builder->length += length;
	builder->data[builder->length] = '\0';
}

static void adn_process_builder_append(AdnProcessStringBuilder* builder, const char* text)
{
	if (!text)
	{
		return;
	}
	adn_process_builder_append_n(builder, text, strlen(text));
}

static char* adn_process_builder_finish(AdnProcessStringBuilder* builder)
{
	if (!builder)
	{
		return NULL;
	}
	if (!builder->data)
	{
		return strdup("");
	}
	char* result = builder->data;
	builder->data = NULL;
	builder->length = 0;
	builder->capacity = 0;
	return result;
}

static char* adn_process_strdup_or_empty(const char* value)
{
	return strdup(value ? value : "");
}

static char* adn_process_unwrap_string(const char* value)
{
	if (!value)
	{
		return strdup("");
	}

	size_t len = strlen(value);
	const char* start = value;
	if (len >= 2 && ((value[0] == '"' && value[len - 1] == '"') ||
	                 (value[0] == '\'' && value[len - 1] == '\'') ||
	                 (value[0] == '`' && value[len - 1] == '`')))
	{
		start = value + 1;
		len -= 2;
	}

	char* result = (char*)malloc(len + 1);
	if (!result)
	{
		return NULL;
	}
	memcpy(result, start, len);
	result[len] = '\0';
	return result;
}

#ifndef _WIN32
static char* adn_process_read_all_file(const char* path)
{
	FILE* file = fopen(path, "rb");
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
	char* buffer = (char*)malloc((size_t)size + 1);
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
#endif

static int adn_process_result_code(int status)
{
#ifdef _WIN32
	return status;
#else
	if (WIFEXITED(status))
	{
		return WEXITSTATUS(status);
	}
	if (WIFSIGNALED(status))
	{
		return 128 + WTERMSIG(status);
	}
	return status;
#endif
}

static void* adn_process_make_result(int64_t code, const char* out, const char* err)
{
	void* result = adn_object_create();
	if (!result)
	{
		return NULL;
	}
	adn_object_set_i64(result, "code", code);
	adn_object_set_string(result, "stdout", out ? out : "");
	adn_object_set_string(result, "stderr", err ? err : "");
	adn_object_set_i64(result, "ok", code == 0 ? 1 : 0);
	return result;
}

static int adn_process_set_nonblocking(int fd)
{
#ifdef _WIN32
	(void)fd;
	return 0;
#else
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		return -1;
	}
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
}

#ifndef _WIN32
static char** adn_process_build_argv(const char* command, void* args, int* argc_out)
{
	if (argc_out)
	{
		*argc_out = 0;
	}
	char* normalized = adn_process_unwrap_string(command);
	if (!normalized || normalized[0] == '\0')
	{
		free(normalized);
		return NULL;
	}

	int64_t arg_count = args ? adn_array_length(args) : 0;
	char** argv = (char**)calloc((size_t)arg_count + 2, sizeof(char*));
	if (!argv)
	{
		free(normalized);
		return NULL;
	}

	argv[0] = normalized;
	for (int64_t i = 0; i < arg_count; i++)
	{
		char* value = adn_array_get_string(args, i);
		argv[(size_t)i + 1] = value ? value : strdup("");
	}
	argv[(size_t)arg_count + 1] = NULL;
	if (argc_out)
	{
		*argc_out = (int)arg_count + 1;
	}
	return argv;
}

static void adn_process_free_argv(char** argv, int argc)
{
	if (!argv)
	{
		return;
	}
	for (int i = 0; i < argc; i++)
	{
		free(argv[i]);
	}
	free(argv);
}

static void adn_process_read_pipe_into_builder(int fd, AdnProcessStringBuilder* builder,
                                               int* open_flag)
{
	char buffer[512];
	ssize_t read_count = read(fd, buffer, sizeof(buffer));
	if (read_count > 0)
	{
		adn_process_builder_append_n(builder, buffer, (size_t)read_count);
		return;
	}
	if (read_count == 0)
	{
		close(fd);
		*open_flag = 0;
		return;
	}
	if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
	{
		close(fd);
		*open_flag = 0;
	}
}

static void* adn_process_capture_exec(char* const argv[], int use_shell)
{
	int stdout_pipe[2];
	int stderr_pipe[2];
	if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0)
	{
		return adn_process_make_result(-1, "", "failed to create pipes");
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		close(stdout_pipe[0]);
		close(stdout_pipe[1]);
		close(stderr_pipe[0]);
		close(stderr_pipe[1]);
		return adn_process_make_result(-1, "", "failed to fork process");
	}

	if (pid == 0)
	{
		dup2(stdout_pipe[1], STDOUT_FILENO);
		dup2(stderr_pipe[1], STDERR_FILENO);
		close(stdout_pipe[0]);
		close(stdout_pipe[1]);
		close(stderr_pipe[0]);
		close(stderr_pipe[1]);
		if (use_shell)
		{
			execl("/bin/sh", "sh", "-c", argv[0], (char*)NULL);
		}
		else
		{
			execvp(argv[0], argv);
		}
		_exit(127);
	}

	close(stdout_pipe[1]);
	close(stderr_pipe[1]);
	adn_process_set_nonblocking(stdout_pipe[0]);
	adn_process_set_nonblocking(stderr_pipe[0]);

	AdnProcessStringBuilder out_builder = {0};
	AdnProcessStringBuilder err_builder = {0};
	adn_process_builder_init(&out_builder);
	adn_process_builder_init(&err_builder);

	int stdout_open = 1;
	int stderr_open = 1;
	while (stdout_open || stderr_open)
	{
		fd_set read_set;
		FD_ZERO(&read_set);
		int max_fd = -1;
		if (stdout_open)
		{
			FD_SET(stdout_pipe[0], &read_set);
			max_fd = stdout_pipe[0];
		}
		if (stderr_open)
		{
			FD_SET(stderr_pipe[0], &read_set);
			if (stderr_pipe[0] > max_fd)
			{
				max_fd = stderr_pipe[0];
			}
		}
		if (max_fd < 0)
		{
			break;
		}
		if (select(max_fd + 1, &read_set, NULL, NULL, NULL) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			break;
		}
		if (stdout_open && FD_ISSET(stdout_pipe[0], &read_set))
		{
			adn_process_read_pipe_into_builder(stdout_pipe[0], &out_builder, &stdout_open);
		}
		if (stderr_open && FD_ISSET(stderr_pipe[0], &read_set))
		{
			adn_process_read_pipe_into_builder(stderr_pipe[0], &err_builder, &stderr_open);
		}
	}

	int status = 0;
	waitpid(pid, &status, 0);
	char* out = adn_process_builder_finish(&out_builder);
	char* err = adn_process_builder_finish(&err_builder);
	void* result = adn_process_make_result((int64_t)adn_process_result_code(status), out, err);
	free(out);
	free(err);
	return result;
}
#endif

int64_t adn_process_id(void)
{
#ifdef _WIN32
	return (int64_t)GetCurrentProcessId();
#else
	return (int64_t)getpid();
#endif
}

int64_t adn_process_parent_id(void)
{
#ifdef _WIN32
	return 0;
#else
	return (int64_t)getppid();
#endif
}

char* adn_process_executable_path(void)
{
#ifdef _WIN32
	char buffer[MAX_PATH];
	DWORD length = GetModuleFileNameA(NULL, buffer, MAX_PATH);
	if (length == 0 || length >= MAX_PATH)
	{
		return strdup("");
	}
	buffer[length] = '\0';
	return strdup(buffer);
#else
	char buffer[PATH_MAX];
	ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (length < 0)
	{
		return strdup("");
	}
	buffer[length] = '\0';
	return strdup(buffer);
#endif
}

char* adn_process_name(void)
{
	char* path = adn_process_executable_path();
	if (!path)
	{
		return strdup("");
	}
	char* slash = strrchr(path, '/');
#ifdef _WIN32
	char* backslash = strrchr(path, '\\');
	if (backslash && (!slash || backslash > slash))
	{
		slash = backslash;
	}
#endif
	char* result = strdup(slash ? slash + 1 : path);
	free(path);
	return result ? result : strdup("");
}

void* adn_process_args(void)
{
	void* result = adn_array_create();
	if (!result)
	{
		return NULL;
	}
#ifdef _WIN32
	char* command_line = GetCommandLineA();
	if (command_line)
	{
		adn_array_push_string(result, command_line);
	}
#else
	char* cmdline = adn_process_read_all_file("/proc/self/cmdline");
	if (!cmdline)
	{
		char* exe = adn_process_executable_path();
		adn_array_push_string(result, exe ? exe : "");
		free(exe);
		return result;
	}
	char* cursor = cmdline;
	while (*cursor)
	{
		size_t length = strlen(cursor);
		adn_array_push_string(result, cursor);
		cursor += length + 1;
	}
	free(cmdline);
#endif
	return result;
}

int64_t adn_process_arg_count(void)
{
	void* values = adn_process_args();
	if (!values)
	{
		return 0;
	}
	return adn_array_length(values);
}

char* adn_process_arg(int64_t index)
{
	void* values = adn_process_args();
	if (!values)
	{
		return strdup("");
	}
	return adn_array_get_string(values, index);
}

char* adn_process_env(const char* name)
{
	char* normalized = adn_process_unwrap_string(name);
	if (!normalized)
	{
		return strdup("");
	}
	const char* value = getenv(normalized);
	free(normalized);
	return adn_process_strdup_or_empty(value);
}

int64_t adn_process_set_env(const char* name, const char* value)
{
	char* normalized_name = adn_process_unwrap_string(name);
	char* normalized_value = adn_process_unwrap_string(value);
	if (!normalized_name || !normalized_value || normalized_name[0] == '\0')
	{
		free(normalized_name);
		free(normalized_value);
		return -1;
	}
#ifdef _WIN32
	int result = _putenv_s(normalized_name, normalized_value);
#else
	int result = setenv(normalized_name, normalized_value, 1);
#endif
	free(normalized_name);
	free(normalized_value);
	return result == 0 ? 0 : -1;
}

int64_t adn_process_has_env(const char* name)
{
	char* normalized = adn_process_unwrap_string(name);
	if (!normalized || normalized[0] == '\0')
	{
		free(normalized);
		return 0;
	}
	const char* value = getenv(normalized);
	free(normalized);
	return value ? 1 : 0;
}

void* adn_process_env_keys(void)
{
	void* result = adn_array_create();
	if (!result)
	{
		return NULL;
	}
#ifdef _WIN32
	LPCH env_block = GetEnvironmentStringsA();
	if (!env_block)
	{
		return result;
	}
	for (LPCH entry = env_block; *entry != '\0'; entry += strlen(entry) + 1)
	{
		char* equal = strchr(entry, '=');
		if (!equal || equal == entry)
		{
			continue;
		}
		size_t key_len = (size_t)(equal - entry);
		char* key = (char*)malloc(key_len + 1);
		if (!key)
		{
			continue;
		}
		memcpy(key, entry, key_len);
		key[key_len] = '\0';
		adn_array_push_string(result, key);
		free(key);
	}
	FreeEnvironmentStringsA(env_block);
#else
	for (char** entry = environ; entry && *entry; entry++)
	{
		char* equal = strchr(*entry, '=');
		if (!equal || equal == *entry)
		{
			continue;
		}
		size_t key_len = (size_t)(equal - *entry);
		char* key = (char*)malloc(key_len + 1);
		if (!key)
		{
			continue;
		}
		memcpy(key, *entry, key_len);
		key[key_len] = '\0';
		adn_array_push_string(result, key);
		free(key);
	}
#endif
	return result;
}

char* adn_process_cwd(void)
{
#ifdef _WIN32
	char buffer[MAX_PATH];
	if (!_getcwd(buffer, sizeof(buffer)))
	{
		return strdup("");
	}
	return strdup(buffer);
#else
	char buffer[PATH_MAX];
	if (!getcwd(buffer, sizeof(buffer)))
	{
		return strdup("");
	}
	return strdup(buffer);
#endif
}

int64_t adn_process_chdir(const char* path)
{
	char* normalized = adn_process_unwrap_string(path);
	if (!normalized || normalized[0] == '\0')
	{
		free(normalized);
		return -1;
	}
#ifdef _WIN32
	int result = _chdir(normalized);
#else
	int result = chdir(normalized);
#endif
	free(normalized);
	return result == 0 ? 0 : -1;
}

void adn_process_exit(int64_t code)
{
	exit((int)code);
}

void adn_process_abort(void)
{
	abort();
}

char* adn_process_os(void)
{
#if defined(_WIN32)
	return strdup("windows");
#elif defined(__APPLE__)
	return strdup("macos");
#elif defined(__linux__)
	return strdup("linux");
#else
	return strdup("unknown");
#endif
}

char* adn_process_arch(void)
{
#if defined(__x86_64__) || defined(_M_X64)
	return strdup("x86_64");
#elif defined(__aarch64__) || defined(_M_ARM64)
	return strdup("arm64");
#elif defined(__i386__) || defined(_M_IX86)
	return strdup("x86");
#elif defined(__arm__) || defined(_M_ARM)
	return strdup("arm");
#else
	return strdup("unknown");
#endif
}

int64_t adn_process_is_windows(void)
{
#ifdef _WIN32
	return 1;
#else
	return 0;
#endif
}

int64_t adn_process_is_linux(void)
{
#ifdef __linux__
	return 1;
#else
	return 0;
#endif
}

int64_t adn_process_is_macos(void)
{
#ifdef __APPLE__
	return 1;
#else
	return 0;
#endif
}

int64_t adn_process_run(const char* command)
{
	char* normalized = adn_process_unwrap_string(command);
	if (!normalized || normalized[0] == '\0')
	{
		free(normalized);
		return -1;
	}
	int status = system(normalized);
	free(normalized);
	return (int64_t)adn_process_result_code(status);
}

int64_t adn_process_run_args(const char* command, void* args)
{
#ifdef _WIN32
	(void)args;
	return adn_process_run(command);
#else
	int argc = 0;
	char** argv = adn_process_build_argv(command, args, &argc);
	if (!argv)
	{
		return -1;
	}
	pid_t pid = fork();
	if (pid < 0)
	{
		adn_process_free_argv(argv, argc);
		return -1;
	}
	if (pid == 0)
	{
		execvp(argv[0], argv);
		_exit(127);
	}
	int status = 0;
	waitpid(pid, &status, 0);
	adn_process_free_argv(argv, argc);
	return (int64_t)adn_process_result_code(status);
#endif
}

void* adn_process_run_capture(const char* command)
{
#ifdef _WIN32
	return adn_process_make_result(adn_process_run(command), "", "capture not supported on windows");
#else
	char* normalized = adn_process_unwrap_string(command);
	if (!normalized || normalized[0] == '\0')
	{
		free(normalized);
		return adn_process_make_result(-1, "", "empty command");
	}
	char* shell_argv[] = {normalized, NULL};
	void* result = adn_process_capture_exec(shell_argv, 1);
	free(normalized);
	return result;
#endif
}

void* adn_process_run_capture_args(const char* command, void* args)
{
#ifdef _WIN32
	(void)args;
	return adn_process_make_result(adn_process_run(command), "", "capture not supported on windows");
#else
	int argc = 0;
	char** argv = adn_process_build_argv(command, args, &argc);
	if (!argv)
	{
		return adn_process_make_result(-1, "", "empty command");
	}
	void* result = adn_process_capture_exec(argv, 0);
	adn_process_free_argv(argv, argc);
	return result;
#endif
}

int64_t adn_process_spawn(const char* command, void* args)
{
#ifdef _WIN32
	(void)args;
	return -1;
#else
	int argc = 0;
	char** argv = adn_process_build_argv(command, args, &argc);
	if (!argv)
	{
		return -1;
	}
	pid_t pid = fork();
	if (pid < 0)
	{
		adn_process_free_argv(argv, argc);
		return -1;
	}
	if (pid == 0)
	{
		execvp(argv[0], argv);
		_exit(127);
	}
	adn_process_free_argv(argv, argc);
	return (int64_t)pid;
#endif
}

int64_t adn_process_kill(int64_t pid)
{
#ifdef _WIN32
	(void)pid;
	return 0;
#else
	if (pid <= 0)
	{
		return 0;
	}
	return kill((pid_t)pid, SIGTERM) == 0 ? 1 : 0;
#endif
}

int64_t adn_process_is_running(int64_t pid)
{
#ifdef _WIN32
	(void)pid;
	return 0;
#else
	if (pid <= 0)
	{
		return 0;
	}
	if (kill((pid_t)pid, 0) == 0)
	{
		return 1;
	}
	return errno == EPERM ? 1 : 0;
#endif
}

char* adn_process_home_dir(void)
{
	const char* home = getenv("HOME");
	if (home && home[0] != '\0')
	{
		return strdup(home);
	}
#ifdef _WIN32
	home = getenv("USERPROFILE");
	if (home && home[0] != '\0')
	{
		return strdup(home);
	}
#else
	struct passwd* pw = getpwuid(getuid());
	if (pw && pw->pw_dir)
	{
		return strdup(pw->pw_dir);
	}
#endif
	return strdup("");
}

char* adn_process_temp_dir(void)
{
	const char* temp = getenv("TMPDIR");
	if (temp && temp[0] != '\0')
	{
		return strdup(temp);
	}
#ifdef _WIN32
	temp = getenv("TEMP");
	if (temp && temp[0] != '\0')
	{
		return strdup(temp);
	}
	temp = getenv("TMP");
	if (temp && temp[0] != '\0')
	{
		return strdup(temp);
	}
	return strdup(".");
#else
	return strdup("/tmp");
#endif
}

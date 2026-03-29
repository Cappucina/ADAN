#ifndef ADAN_PROCESS_H
#define ADAN_PROCESS_H

#include <stdint.h>

void* adn_object_create(void);

void adn_object_set_i64(void* object, const char* key, int64_t value);

void adn_object_set_string(void* object, const char* key, const char* value);

void* adn_array_create(void);

int64_t adn_array_length(void* array);

void adn_array_push_string(void* array, const char* value);

char* adn_array_get_string(void* array, int64_t index);

int64_t adn_process_id(void);

int64_t adn_process_parent_id(void);

char* adn_process_name(void);

void* adn_process_args(void);

int64_t adn_process_arg_count(void);

char* adn_process_arg(int64_t index);

char* adn_process_env(const char* name);

int64_t adn_process_set_env(const char* name, const char* value);

int64_t adn_process_has_env(const char* name);

void* adn_process_env_keys(void);

char* adn_process_cwd(void);

int64_t adn_process_chdir(const char* path);

void adn_process_exit(int64_t code);

void adn_process_abort(void);

char* adn_process_os(void);

char* adn_process_arch(void);

int64_t adn_process_is_windows(void);

int64_t adn_process_is_linux(void);

int64_t adn_process_is_macos(void);

int64_t adn_process_run(const char* command);

int64_t adn_process_run_args(const char* command, void* args);

void* adn_process_run_capture(const char* command);

void* adn_process_run_capture_args(const char* command, void* args);

int64_t adn_process_spawn(const char* command, void* args);

int64_t adn_process_kill(int64_t pid);

int64_t adn_process_is_running(int64_t pid);

char* adn_process_executable_path(void);

char* adn_process_home_dir(void);

char* adn_process_temp_dir(void);

#endif

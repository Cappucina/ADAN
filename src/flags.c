#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "flags.h"

static bool parse_bool(const char *val, bool defaultVal) {
    if (!val) return defaultVal;
    return (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
}

void set_help(compiler_flags *f, void* extra) { f->help = parse_bool((char*)extra, true); }
void set_verbose(compiler_flags *f, void* extra) { f->verbose = parse_bool((char*)extra, true); }
void set_keep_asm(compiler_flags *f, void* extra) { f->keep_asm = parse_bool((char*)extra, true); }
void set_warnings_as_errors(compiler_flags *f, void* extra) { f->warnings_as_errors = parse_bool((char*)extra, true); }
void set_run_post_compile(compiler_flags *f, void* extra) { f->run_post_compile = parse_bool((char*)extra, true); }
void set_compile_time(compiler_flags *f, void* extra) { f->compile_time = parse_bool((char*)extra, true); }
void set_input(compiler_flags *f, void* extra) {
    if (f->input) free(f->input);
    if (extra) f->input = strdup((char*)extra);
    else f->input = strdup("");
}

void set_output(compiler_flags *f, void* extra) {
    if (f->output) free(f->output);
    if (extra) f->output = strdup((char*)extra);
    else f->output = strdup("");
}
void set_unknown_flag(compiler_flags *f, void* extra) { f->unknown_flag = true; }

compiler_flags* flags_init() {
    compiler_flags *flags = malloc(sizeof(compiler_flags));
    if (!flags) return NULL;

#ifdef __APPLE__
    flags->target.os = DARWIN;
#else
    flags->target.os = LINUX;
#endif

    flags->target.arch = X86_64;
    flags->help = false;
    flags->verbose = false;
    flags->keep_asm = false;
    flags->input = NULL;
    flags->output = NULL;
    flags->warnings_as_errors = false;
    flags->run_post_compile = false;
    flags->compile_time = true;
    flags->unknown_flag = false;
    return flags;
}

const char* help_aliases[] = {"h"};
const char* verbose_aliases[] = {"v"};
const char* keep_asm_aliases[] = {"k"};
const char* warnings_aliases[] = {"w"};
const char* output_aliases[] = {"o"};
const char* input_aliases[] = {"i"};
const char* compile_time_aliases[] = {"c"};
const char* execute_after_run_aliases[] = {"e"};

flag_entry flag_registry[] = {
    {"help", help_aliases, 1, set_help, NULL},
    {"verbose", verbose_aliases, 1, set_verbose, NULL},
    {"keep-asm", keep_asm_aliases, 1, set_keep_asm, NULL},
    {"warnings-as-errors", warnings_aliases, 1, set_warnings_as_errors, NULL},
    {"output", output_aliases, 1, set_output, NULL},
    {"input", input_aliases, 1, set_input, NULL},
    {"compile-time", compile_time_aliases, 1, set_compile_time, NULL},
    {"execute-after-run", execute_after_run_aliases, 1, set_run_post_compile, NULL},
};

const int flag_count = sizeof(flag_registry) / sizeof(flag_registry[0]);

int parse_flags(int argc, char **argv, compiler_flags *flags) {
    int arg_index = 1;
    if (argc > 1 && argv[1][0] != '-') {
        set_input(flags, argv[1]);
        arg_index = 2;
    }

    for (int i = arg_index; i < argc; i++) {
        char* arg = argv[i];
        if (arg[0] != '-') continue;

        char* flag_name = NULL;
        char* extra_value = NULL;

        if (arg[1] == '-') {
            char* eq = strchr(arg + 2, '=');
            if (eq) {
                *eq = '\0';
                flag_name = arg + 2;
                extra_value = eq + 1;
            } else {
                flag_name = arg + 2;
                if (i + 1 < argc && argv[i+1][0] != '-') {
                    extra_value = argv[i+1];
                    i++;
                }
            }
        } else {
            flag_name = arg + 1;
            if (i + 1 < argc && argv[i+1][0] != '-') {
                extra_value = argv[i+1];
                i++;
            }
        }

        bool matched = false;
        for (int j = 0; j < flag_count; j++) {
            flag_entry *entry = &flag_registry[j];

            if (strcmp(flag_name, entry->name) == 0) {
                entry->setter(flags, extra_value);
                matched = true;
                break;
            }
            for (int k = 0; k < entry->alias_count; k++) {
                if (strcmp(flag_name, entry->aliases[k]) == 0) {
                    entry->setter(flags, extra_value);
                    matched = true;
                    break;
                }
            }
            if (matched) break;
        }

        if (!matched) {
            set_unknown_flag(flags, NULL);
        }
    }

    if (!flags->input) {
        set_help(flags, NULL);
    }
    
    return 0;
}
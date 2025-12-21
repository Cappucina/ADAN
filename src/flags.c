#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "flags.h"

static bool parse_bool(const char *val, bool default_LVal) {
    if (!val) return default_LVal;
    return (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
}

void set_help(CompilerFlags *f, void* extra) { f->help = parse_bool((char*)extra, true); }
void set_verbose(CompilerFlags *f, void* extra) { f->verbose = parse_bool((char*)extra, true); }
void set_keepASM(CompilerFlags *f, void* extra) { f->keep_asm = parse_bool((char*)extra, true); }
void set_warningsAsErrors(CompilerFlags *f, void* extra) { f->warnings_as_errors = parse_bool((char*)extra, true); }
void set_runAfterCompile(CompilerFlags *f, void* extra) { f->run_after_compile = parse_bool((char*)extra, true); }
void set_compileTime(CompilerFlags *f, void* extra) { f->compile_time = parse_bool((char*)extra, true); }
void set_input(CompilerFlags *f, void* extra) {
    if (f->input) free(f->input);
    if (extra) f->input = strdup((char*)extra);
    else f->input = strdup("");
}
void set_lib(CompilerFlags *f, void* extra) {
    if (!extra) return;
    char** newLibs = realloc(f->libs, sizeof(char*) * (f->lib_count + 1));
    if (!newLibs) return; // handle allocation failure
    f->libs = newLibs;
    f->libs[f->lib_count] = strdup((char*)extra);
    f->lib_count++;
}
void set_output(CompilerFlags *f, void* extra) {
    if (f->output) free(f->output);
    if (extra) f->output = strdup((char*)extra);
    else f->output = strdup("");
}
void set_unknownFlag(CompilerFlags *f, void* extra) { f->unknown_flag = true; }

CompilerFlags* flags_init() {
    CompilerFlags *flags = malloc(sizeof(CompilerFlags));
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
    flags->run_after_compile = false;
    flags->compile_time = true;
    flags->unknown_flag = false;
    
    flags->lib_count = 1;
    flags->libs = malloc(sizeof(char*) * 1);
    if (!flags->libs) {
        free(flags);
        return NULL;
    }
    flags->libs[0] = strdup("./lib");
    if (!flags->libs[0]) {
        free(flags->libs);
        free(flags);
        return NULL;
    }

    return flags;
}

const char* helpAliases[] = {"h"};
const char* verboseAliases[] = {"v"};
const char* keepASMAliases[] = {"k"};
const char* warningsAliases[] = {"w"};
const char* outputAliases[] = {"o"};
const char* inputAliases[] = {"i"};
const char* compileTimeAliases[] = {"c"};
const char* executeAfterRunAliases[] = {"e"};
const char* includeAliases[] = {"l", "i"};

FlagEntry flagRegistry[] = {
    {"help", helpAliases, 1, set_help, NULL},
    {"verbose", verboseAliases, 1, set_verbose, NULL},
    {"keep-asm", keepASMAliases, 1, set_keepASM, NULL},
    {"warnings-as-errors", warningsAliases, 1, set_warningsAsErrors, NULL},
    {"output", outputAliases, 1, set_output, NULL},
    {"input", inputAliases, 1, set_input, NULL},
    {"compile-time", compileTimeAliases, 1, set_compileTime, NULL},
    {"execute-after-run", executeAfterRunAliases, 1, set_runAfterCompile, NULL},
    {"include", includeAliases, 2, set_lib, NULL},
};

const int flagCount = sizeof(flagRegistry) / sizeof(flagRegistry[0]);

int parse_flags(int argc, char **argv, CompilerFlags *flags) {
    int argIndex = 1;
    if (argc > 1 && argv[1][0] != '-') {
        set_input(flags, argv[1]);
        argIndex = 2;
    }

    for (int i = argIndex; i < argc; i++) {
        char* arg = argv[i];
        if (arg[0] != '-') continue;

        char* flagName = NULL;
        char* extraValue = NULL;

        if (arg[1] == '-') {
            char* eq = strchr(arg + 2, '=');
            if (eq) {
                *eq = '\0';
                flagName = arg + 2;
                extraValue = eq + 1;
            } else {
                flagName = arg + 2;
                if (i + 1 < argc && argv[i+1][0] != '-') {
                    extraValue = argv[i+1];
                    i++;
                }
            }
        } else {
            flagName = arg + 1;
            if (i + 1 < argc && argv[i+1][0] != '-') {
                extraValue = argv[i+1];
                i++;
            }
        }

        bool matched = false;
        for (int j = 0; j < flagCount; j++) {
            FlagEntry *entry = &flagRegistry[j];

            if (strcmp(flagName, entry->name) == 0) {
                entry->setter(flags, extraValue);
                matched = true;
                break;
            }
            for (int k = 0; k < entry->alias_count; k++) {
                if (strcmp(flagName, entry->aliases[k]) == 0) {
                    entry->setter(flags, extraValue);
                    matched = true;
                    break;
                }
            }
            if (matched) break;
        }

        if (!matched) {
            set_unknownFlag(flags, NULL);
        }
    }

    if (!flags->input) {
        set_help(flags, NULL);
    }
    
    return 0;
}
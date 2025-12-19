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
void set_keepASM(compiler_flags *f, void* extra) { f->keepASM = parse_bool((char*)extra, true); }
void set_warningsAsErrors(compiler_flags *f, void* extra) { f->warningsAsErrors = parse_bool((char*)extra, true); }
void set_runAfterCompile(compiler_flags *f, void* extra) { f->runAfterCompile = parse_bool((char*)extra, true); }
void set_compileTime(compiler_flags *f, void* extra) { f->compileTime = parse_bool((char*)extra, true); }
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
void set_unknownFlag(compiler_flags *f, void* extra) { f->unknownFlag = true; }

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
    flags->keepASM = false;
    flags->input = NULL;
    flags->output = NULL;
    flags->warningsAsErrors = false;
    flags->runAfterCompile = false;
    flags->compileTime = true;
    flags->unknownFlag = false;
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

FlagEntry flagRegistry[] = {
    {"help", helpAliases, 1, set_help, NULL},
    {"verbose", verboseAliases, 1, set_verbose, NULL},
    {"keep-asm", keepASMAliases, 1, set_keepASM, NULL},
    {"warnings-as-errors", warningsAliases, 1, set_warningsAsErrors, NULL},
    {"output", outputAliases, 1, set_output, NULL},
    {"input", inputAliases, 1, set_input, NULL},
    {"compile-time", compileTimeAliases, 1, set_compileTime, NULL},
    {"execute-after-run", executeAfterRunAliases, 1, set_runAfterCompile, NULL},
};

const int flagCount = sizeof(flagRegistry) / sizeof(flagRegistry[0]);

int parse_flags(int argc, char **argv, compiler_flags *flags) {
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
            for (int k = 0; k < entry->aliasCount; k++) {
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
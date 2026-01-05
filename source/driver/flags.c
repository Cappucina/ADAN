#include "flags.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INCLUDES 256

typedef int (*flag_setter)(CompilerFlags*, void*);

typedef enum
{
    FLAG_OK = 0,
    FLAG_ERR_UNKNOWN = 1,
    FLAG_ERR_INVALID_VALUE = 2,
    FLAG_ERR_MISSING_VALUE = 3,
    FLAG_ERR_MEMORY = 4
} FlagError;

static const char* error_messages[] = {"No error", "Unknown flag", "Invalid value for flag",
                                       "Missing value for flag", "Memory allocation failed"};

static FlagError last_error = FLAG_OK;

const char* flags_get_error(void)
{
    return error_messages[last_error];
}

static void set_error(FlagError err)
{
    last_error = err;
}

void set_default_flags(CompilerFlags* flags)
{
    if (!flags) return;

    Target* target = get_self_as_target();
    if (target)
    {
        flags->target = *target;
    }
    flags->help = false;
    flags->verbose = false;
    flags->warnings_as_errors = false;
    flags->tests = false;
    flags->optimazation_level = 0;
    flags->input = "main.adn";
    flags->output = "a.out";
    flags->compile_to = EXECUTABLE;
    flags->suppress_warnings = false;
    flags->include = (char**)malloc(sizeof(char*) * 2);
    if (flags->include)
    {
        flags->include[0] = "";
        flags->include[1] = NULL;
    }
    flags->error = NULL;
    last_error = FLAG_OK;
}

static int set_help(CompilerFlags* flags, void* value)
{
    (void)value;
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->help = true;
    return 0;
}

static int set_verbose(CompilerFlags* flags, void* value)
{
    (void)value;
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->verbose = true;
    return 0;
}

static int set_suppress_warnings(CompilerFlags* flags, void* value)
{
    (void)value;
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->suppress_warnings = true;
    return 0;
}

static int set_warnings_as_errors(CompilerFlags* flags, void* value)
{
    (void)value;
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->warnings_as_errors = true;
    return 0;
}

static int set_tests(CompilerFlags* flags, void* value)
{
    (void)value;
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->tests = true;
    return 0;
}

static int set_output(CompilerFlags* flags, void* value)
{
    if (!flags || !value)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->output = (char*)value;
    return 0;
}

static int set_input(CompilerFlags* flags, void* value)
{
    if (!flags || !value)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->input = (char*)value;
    return 0;
}

static int set_include(CompilerFlags* flags, void* value)
{
    if (!flags || !value)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }

    int count = 0;
    if (flags->include)
    {
        while (flags->include[count]) count++;
    }

    if (count >= MAX_INCLUDES - 1)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }

    char** new_includes = (char**)realloc(flags->include, sizeof(char*) * (size_t)(count + 2));
    if (!new_includes)
    {
        set_error(FLAG_ERR_MEMORY);
        return -1;
    }

    flags->include = new_includes;
    flags->include[count] = (char*)value;
    flags->include[count + 1] = NULL;

    return 0;
}

static int set_optimazation(CompilerFlags* flags, void* value)
{
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }

    if (!value)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }

    const char* opt = (const char*)value;

    if (strlen(opt) == 2 && opt[0] == 'O' && opt[1] >= '0' && opt[1] <= '3')
    {
        flags->optimazation_level = (uint8_t)(opt[1] - '0');
        return 0;
    }

    set_error(FLAG_ERR_INVALID_VALUE);
    return -1;
}

static int set_compile_to_asm(CompilerFlags* flags, void* value)
{
    (void)value;
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->compile_to = ASM;
    return 0;
}

static int set_compile_to_object(CompilerFlags* flags, void* value)
{
    (void)value;
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->compile_to = OBJECT;
    return 0;
}

static int set_compile_to_executable(CompilerFlags* flags, void* value)
{
    (void)value;
    if (!flags)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }
    flags->compile_to = EXECUTABLE;
    return 0;
}

typedef struct
{
    const char** names;
    int count;
    flag_setter setter;
} FlagEntry;

static FlagEntry flag_registry[] = {
    {(const char*[]){"h", "help", NULL}, 2, set_help},
    {(const char*[]){"v", "verbose", NULL}, 2, set_verbose},
    {(const char*[]){"o", "output", NULL}, 2, set_output},
    {(const char*[]){"i", "input", NULL}, 2, set_input},
    {(const char*[]){"I", "include", NULL}, 2, set_include},
    {(const char*[]){"O0", "O1", "O2", "O3", NULL}, 4, set_optimazation},
    {(const char*[]){"s", "compile-to-asm", NULL}, 2, set_compile_to_asm},
    {(const char*[]){"S", "suppress-warnings", NULL}, 2, set_suppress_warnings},
    {(const char*[]){"a", "compile-to-object", NULL}, 2, set_compile_to_object},
    {(const char*[]){"e", "compile-to-executable", NULL}, 2, set_compile_to_executable},
    {(const char*[]){"w", "W", "warnings-as-errors", NULL}, 3, set_warnings_as_errors},
    {(const char*[]){"t", "tests", NULL}, 2, set_tests},
    {NULL, 0, NULL}};

static int find_flag(const char* arg, flag_setter* setter)
{
    if (!arg || !setter)
    {
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }

    for (int i = 0; flag_registry[i].names; i++)
    {
        const char** names = flag_registry[i].names;
        for (int j = 0; names[j]; j++)
        {
            if (strcmp(names[j], arg) == 0)
            {
                *setter = flag_registry[i].setter;
                return 0;
            }
        }
    }

    set_error(FLAG_ERR_UNKNOWN);
    return -1;
}

static int needs_value(const char* arg)
{
    const char* value_flags[] = {"o", "output", "i", "input", "I", "include", NULL};

    for (int i = 0; value_flags[i]; i++)
    {
        if (strcmp(arg, value_flags[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static int parse_long_flag_with_value(const char* arg, char** flag_name, char** value)
{
    const char* equals = strchr(arg, '=');
    if (!equals)
    {
        return -1;
    }

    size_t flag_len = (size_t)(equals - arg);
    *flag_name = (char*)malloc(flag_len + 1);
    if (!*flag_name)
    {
        set_error(FLAG_ERR_MEMORY);
        return -1;
    }

    strncpy(*flag_name, arg, flag_len);
    (*flag_name)[flag_len] = '\0';

    if (*(equals + 1) == '\0')
    {
        free(*flag_name);
        *flag_name = NULL;
        set_error(FLAG_ERR_INVALID_VALUE);
        return -1;
    }

    *value = (char*)(equals + 1);
    return 0;
}

CompilerFlags* flags_init(int argc, char* argv[])
{
    CompilerFlags* flags = (CompilerFlags*)malloc(sizeof(CompilerFlags));

    if (!flags)
    {
        set_error(FLAG_ERR_MEMORY);
        return NULL;
    }

    set_default_flags(flags);

    for (int i = 1; i < argc; i++)
    {
        const char* arg = argv[i];

        if (!arg || strlen(arg) == 0)
        {
            continue;
        }

        if (arg[0] != '-')
        {
            flags->input = (char*)arg;
            continue;
        }

        if (arg[0] == '-' && arg[1] == '-')
        {
            const char* flag_start = arg + 2;
            char* flag_name = NULL;
            char* flag_value = NULL;
            flag_setter setter = NULL;
            int has_inline_value = 0;

            if (parse_long_flag_with_value(flag_start, &flag_name, &flag_value) == 0)
            {
                has_inline_value = 1;
            }
            else
            {
                flag_name = (char*)flag_start;
            }

            if (find_flag(flag_name, &setter) != 0)
            {
                if (has_inline_value) free(flag_name);
                flags->error = (char*)flags_get_error();
                return flags;
            }

            if (needs_value(flag_name))
            {
                if (has_inline_value)
                {
                    if (setter(flags, flag_value) != 0)
                    {
                        free(flag_name);
                        flags->error = (char*)flags_get_error();
                        return flags;
                    }
                    free(flag_name);
                }
                else if (i + 1 >= argc)
                {
                    set_error(FLAG_ERR_MISSING_VALUE);
                    flags->error = (char*)flags_get_error();
                    return flags;
                }
                else
                {
                    i++;
                    if (setter(flags, argv[i]) != 0)
                    {
                        flags->error = (char*)flags_get_error();
                        return flags;
                    }
                }
            }
            else
            {
                if (has_inline_value)
                {
                    set_error(FLAG_ERR_INVALID_VALUE);
                    free(flag_name);
                    flags->error = (char*)flags_get_error();
                    return flags;
                }

                if (setter(flags, (void*)flag_name) != 0)
                {
                    flags->error = (char*)flags_get_error();
                    return flags;
                }

                if (has_inline_value) free(flag_name);
            }
        }
        else if (arg[0] == '-')
        {
            for (int j = 1; arg[j]; j++)
            {
                char flag_char[2] = {arg[j], '\0'};
                flag_setter setter = NULL;

                if (find_flag(flag_char, &setter) != 0)
                {
                    flags->error = (char*)flags_get_error();
                    return flags;
                }

                if (needs_value(flag_char))
                {
                    if (j + 1 < (int)strlen(arg))
                    {
                        char* value = (char*)(arg + j + 1);
                        if (setter(flags, value) != 0)
                        {
                            flags->error = (char*)flags_get_error();
                            return flags;
                        }
                        break;
                    }
                    else if (i + 1 < argc)
                    {
                        i++;
                        if (setter(flags, argv[i]) != 0)
                        {
                            flags->error = (char*)flags_get_error();
                            return flags;
                        }
                        break;
                    }
                    else
                    {
                        set_error(FLAG_ERR_MISSING_VALUE);
                        flags->error = (char*)flags_get_error();
                        return flags;
                    }
                }
                else
                {
                    if (setter(flags, NULL) != 0)
                    {
                        flags->error = (char*)flags_get_error();
                        return flags;
                    }
                }
            }
        }
    }

    return flags;
}

void flags_free(CompilerFlags* flags)
{
    if (!flags) return;

    if (flags->include)
    {
        free(flags->include);
        flags->include = NULL;
    }

    free(flags);
}

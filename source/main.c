#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "diagnostic.h"
#include "error.h"
#include "flags.h"
#include "test.h"

int main(int argc, char* argv[])
{
    int res = 0;
    ErrorList* error_list = create_errors();
    CompilorFlags* flags = flags_init(argc, argv);

    if (!error_list)
    {
        fprintf(stderr, "Failed to allocate memory for error_list\n");
        return 1;
    }

    g_error_list = error_list;

    if (!flags)
    {
        fprintf(stderr, "Failed to allocate memory for flags\n");
        res = 1;
        goto out;
    }

    set_verbose_mode(flags->verbose);
    set_suppress_warnings(flags->suppress_warnings);
    set_warnings_as_errors(flags->warnings_as_errors);

    if (flags->error)
    {
        fprintf(stderr, "\033[0;31merror\033[0m: %s\n", flags->error);
        goto out;
    }

    if (flags->help)
    {
        printf("Usage: adan <input-file> [options]\n");
        printf("Options:\n");
        printf("  -h, --help                 Show this help message\n");
        printf("  -v, --verbose              Enable verbose output\n");
        printf("  -o, --output <file>        Output file name (default: a.out)\n");
        printf("  -i, --input <file>         Input file name (default: main.adn)\n");
        printf("  -I, --include <path>       Add include path\n");
        printf("  -O0, -O1, -O2, -O3         Optimization level\n");
        printf("  -s, --compile-to-asm       Compile to assembly\n");
        printf("  -a, --compile-to-object    Compile to object file\n");
        printf("  -e, --compile-to-executable Compile to executable (default)\n");
        printf("  -S, --suppress-warnings    Suppress warnings\n");
        printf("  -w, --warnings-as-errors   Treat warnings as errors\n");
        printf("  -t, --tests                Run test suite\n");

        goto out;
    }

    if (flags->tests)
    {
        int test_result = run_all_tests();
        res = test_result;
        goto out;
    }

    printf("I am compiling correctly\n");

out:
    if (error_list) free_errors(error_list);
    if (flags) flags_free(flags);

    return res;
}

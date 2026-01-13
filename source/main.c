#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "buffer.h"
#include "diagnostic.h"
#include "driver/flags.h"
#include "error.h"
#include "fs.h"
#include "lex/lexer.h"
#include "parse/parser.h"
#include "semantic/semantic.h"
#include "tests/test.h"

int main(int argc, char* argv[])
{
    int res = 0;
    ErrorList* errors = create_errors();
    CompilerFlags* flags = flags_init(argc, argv);
    Buffer* tokens = NULL;
    Lexer* lexer = NULL;
    Analyzer* parser = NULL;
    Analyzer* semantic = NULL;

    if (!errors)
    {
        fprintf(stderr, "Failed to allocate memory for error_list\n");
        return -ENOMEM;
    }

    g_error_list = errors;

    if (!flags)
    {
        fprintf(stderr, "Failed to allocate memory for flags\n");
        res = -ENOMEM;
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
        printf("  -h, --help                        Show this help message\n");
        printf("  -v, --verbose                     Enable verbose output\n");
        printf("  -o, --output <file>               Output file name (default: a.out)\n");
        printf("  -i, --input <file>                Input file name (default: main.adn)\n");
        printf("  -I, --include <path>              Add include path\n");
        printf("  -O0, -O1, -O2, -O3                Optimization level\n");
        printf("  -s, --compile-to-asm              Compile to assembly\n");
        printf("  -a, --compile-to-object           Compile to object file\n");
        printf("  -e, --compile-to-executable       Compile to executable (default)\n");
        printf("  -S, --suppress-warnings           Suppress warnings\n");
        printf("  -W, -w, --warnings-as-errors      Treat warnings as errors\n");
        printf("  -t, --tests                       Run test suite\n");

        goto out;
    }

    if (flags->tests)
    {
        int test_result = run_all_tests();
        res = test_result;
        goto out;
    }

    FILE* input = fopen(flags->input, "r");

    if (!input)
    {
        error(errors, "input", 0, 0, GENERIC, "Input file does not exist or is not accessible");

        res = -EINVAL;
        goto out;
    }

    const char* source = file_to_string(input, errors);
    if (!source)
    {
        res = -EINVAL;
        goto out;
    }

    lexer = create_lexer(source, errors, flags->input);
    tokens = buffer_create(sizeof(Token));

    if (!tokens)
    {
        free_lexer(lexer);
        res = -ENOMEM;
        goto out;
    }

    for (;;)
    {
        Token token = lex(lexer);
        buffer_push(tokens, &token);

        if (token.type == TOKEN_EOF)
        {
            break;
        }
    }

    if (errors->size > 0)
    {
        res = -EINVAL;
        goto out;
    }

    parser = create_parser(tokens, errors);

    parse(parser);

    if (errors->size > 0)
    {
        res = -EINVAL;
        goto out;
    }

    semantic = create_semantic(tokens, errors);

    semantic_analysis(semantic);

    if (errors->size > 0)
    {
        res = -EINVAL;
        goto out;
    }

out:
    if (semantic) free_semantic(semantic);
    if (parser) free_parser(parser);
    if (lexer) free_lexer(lexer);
    if (tokens) buffer_free(tokens);
    if (errors) free_errors(errors);
    if (flags) flags_free(flags);

    return res;
}

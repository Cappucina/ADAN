#include "flags.h"

#include "test.h"

static int test_flags_init(void)
{
    const char* argv[] = {"adan", "test.adn"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_NOT_NULL(flags->input, "input should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_help(void)
{
    const char* argv[] = {"adan", "-h"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_TRUE(flags->help, "help flag should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_verbose(void)
{
    const char* argv[] = {"adan", "--verbose"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_TRUE(flags->verbose, "verbose flag should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_tests(void)
{
    const char* argv[] = {"adan", "-t"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_TRUE(flags->tests, "tests flag should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_output(void)
{
    const char* argv[] = {"adan", "-o", "output.adn"};
    CompilerFlags* flags = flags_init(3, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_NOT_NULL(flags->output, "output should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_long_help(void)
{
    const char* argv[] = {"adan", "--help"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_TRUE(flags->help, "help flag should be set with long form");

    flags_free(flags);
    return 0;
}

static int test_flags_long_verbose(void)
{
    const char* argv[] = {"adan", "--verbose"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_TRUE(flags->verbose, "verbose flag should be set with long form");

    flags_free(flags);
    return 0;
}

static int test_flags_long_output(void)
{
    const char* argv[] = {"adan", "--output", "myfile"};
    CompilerFlags* flags = flags_init(3, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_NOT_NULL(flags->output, "output should be set with long form");

    flags_free(flags);
    return 0;
}

static int test_flags_long_input(void)
{
    const char* argv[] = {"adan", "--input", "input.adn"};
    CompilerFlags* flags = flags_init(3, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_NOT_NULL(flags->input, "input should be set with long form");

    flags_free(flags);
    return 0;
}

static int test_flags_suppress_warnings(void)
{
    const char* argv[] = {"adan", "-S"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_TRUE(flags->suppress_warnings, "suppress_warnings flag should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_warnings_as_errors(void)
{
    const char* argv[] = {"adan", "-w"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_TRUE(flags->warnings_as_errors, "warnings_as_errors flag should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_compile_to_asm(void)
{
    const char* argv[] = {"adan", "-s"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_EQ(flags->compile_to, ASM, "compile_to should be ASM");

    flags_free(flags);
    return 0;
}

static int test_flags_compile_to_object(void)
{
    const char* argv[] = {"adan", "-a"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_EQ(flags->compile_to, OBJECT, "compile_to should be OBJECT");

    flags_free(flags);
    return 0;
}

static int test_flags_compile_to_executable(void)
{
    const char* argv[] = {"adan", "-e"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_EQ(flags->compile_to, EXECUTABLE, "compile_to should be EXECUTABLE");

    flags_free(flags);
    return 0;
}

static int test_flags_optimization_o0(void)
{
    const char* argv[] = {"adan", "--O0"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_EQ(flags->optimazation_level, 0, "optimization level should be 0");

    flags_free(flags);
    return 0;
}

static int test_flags_optimization_o2(void)
{
    const char* argv[] = {"adan", "--O2"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_EQ(flags->optimazation_level, 2, "optimization level should be 2");

    flags_free(flags);
    return 0;
}

static int test_flags_multiple_short_flags(void)
{
    const char* argv[] = {"adan", "-vh"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_TRUE(flags->verbose, "verbose flag should be set");
    ASSERT_TRUE(flags->help, "help flag should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_input_positional(void)
{
    const char* argv[] = {"adan", "myfile.adn"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_NOT_NULL(flags->input, "input should be set");

    flags_free(flags);
    return 0;
}

static int test_flags_output_with_equals(void)
{
    const char* argv[] = {"adan", "--output=output.txt"};
    CompilerFlags* flags = flags_init(2, (char**)argv);

    ASSERT_NOT_NULL(flags, "flags_init should not return NULL");
    ASSERT_NOT_NULL(flags->output, "output should be set with long form equals");

    flags_free(flags);
    return 0;
}

int run_flags_tests(TestSuite* suite)
{
    if (!suite)
    {
        return -1;
    }

    test_suite_run_test(suite, "flags_init", test_flags_init);
    test_suite_run_test(suite, "flags_help", test_flags_help);
    test_suite_run_test(suite, "flags_verbose", test_flags_verbose);
    test_suite_run_test(suite, "flags_tests", test_flags_tests);
    test_suite_run_test(suite, "flags_output", test_flags_output);
    test_suite_run_test(suite, "flags_long_help", test_flags_long_help);
    test_suite_run_test(suite, "flags_long_verbose", test_flags_long_verbose);
    test_suite_run_test(suite, "flags_long_output", test_flags_long_output);
    test_suite_run_test(suite, "flags_long_input", test_flags_long_input);
    test_suite_run_test(suite, "flags_suppress_warnings", test_flags_suppress_warnings);
    test_suite_run_test(suite, "flags_warnings_as_errors", test_flags_warnings_as_errors);
    test_suite_run_test(suite, "flags_compile_to_asm", test_flags_compile_to_asm);
    test_suite_run_test(suite, "flags_compile_to_object", test_flags_compile_to_object);
    test_suite_run_test(suite, "flags_compile_to_executable", test_flags_compile_to_executable);
    test_suite_run_test(suite, "flags_optimization_o0", test_flags_optimization_o0);
    test_suite_run_test(suite, "flags_optimization_o2", test_flags_optimization_o2);
    test_suite_run_test(suite, "flags_multiple_short_flags", test_flags_multiple_short_flags);
    test_suite_run_test(suite, "flags_input_positional", test_flags_input_positional);
    test_suite_run_test(suite, "flags_output_with_equals", test_flags_output_with_equals);

    return 0;
}

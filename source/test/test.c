#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TestSuite* test_suite_create(void)
{
    TestSuite* suite = (TestSuite*)malloc(sizeof(TestSuite));
    if (!suite)
    {
        return NULL;
    }

    suite->results = NULL;
    suite->count = 0;
    suite->capacity = 0;
    return suite;
}

void test_suite_free(TestSuite* suite)
{
    if (!suite)
    {
        return;
    }

    if (suite->results)
    {
        free(suite->results);
    }

    free(suite);
}

int test_suite_add_result(TestSuite* suite, const char* name, bool passed, const char* message)
{
    if (!suite || !name)
    {
        return -1;
    }

    if (suite->count >= suite->capacity)
    {
        size_t new_capacity = suite->capacity == 0 ? 10 : suite->capacity * 2;
        TestResult* tmp = (TestResult*)realloc(suite->results, sizeof(TestResult) * new_capacity);
        if (!tmp)
        {
            return -1;
        }

        suite->results = tmp;
        suite->capacity = new_capacity;
    }

    suite->results[suite->count].name = name;
    suite->results[suite->count].passed = passed;
    suite->results[suite->count].message = message;
    suite->count++;

    return 0;
}

int test_suite_run_test(TestSuite* suite, const char* name, TestFunc test_func)
{
    if (!suite || !name || !test_func)
    {
        return -1;
    }

    int result = test_func();
    bool passed = result == 0;
    const char* message = passed ? "\033[32;1m[PASS]\033[0m" : "\033[31;1m[FAIL]\033[0m";

    return test_suite_add_result(suite, name, passed, message);
}

void test_suite_print_results(TestSuite* suite)
{
    if (!suite)
    {
        return;
    }

    printf("\n========== Test Results ==========\n");

    int passed_count = 0;
    int failed_count = 0;

    for (size_t i = 0; i < suite->count; i++)
    {
        TestResult* result = &suite->results[i];
        const char* status = result->passed ? "\033[32;1m✓\033[0m" : "\033[31;1m✗\033[0m";

        printf("%s %s %s\n", status, result->name, result->message);

        if (result->passed)
        {
            passed_count++;
        }
        else
        {
            failed_count++;
        }
    }

    printf("==================================\n");
    printf("Total: %zu | Passed: \033[32;1m%d\033[0m | Failed: \033[31;1m%d\n\033[0m", suite->count,
           passed_count, failed_count);
    printf("==================================\n\n");
}

bool test_suite_all_passed(TestSuite* suite)
{
    if (!suite)
    {
        return false;
    }

    for (size_t i = 0; i < suite->count; i++)
    {
        if (!suite->results[i].passed)
        {
            return false;
        }
    }

    return true;
}

extern int run_diagnostic_tests(TestSuite* suite);
extern int run_flags_tests(TestSuite* suite);
extern int run_lexer_tests(TestSuite* suite);

int run_all_tests(void)
{
    TestSuite* suite = test_suite_create();
    if (!suite)
    {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    run_diagnostic_tests(suite);
    run_flags_tests(suite);
    run_lexer_tests(suite);

    test_suite_print_results(suite);
    bool all_passed = test_suite_all_passed(suite);

    test_suite_free(suite);

    return all_passed ? 0 : 1;
}

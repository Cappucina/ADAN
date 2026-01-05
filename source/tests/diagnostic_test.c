#include "diagnostic.h"

#include <stdlib.h>

#include "test.h"

static int test_create_errors(void)
{
    ErrorList* list = create_errors();
    ASSERT_NOT_NULL(list, "create_errors should not return NULL");
    ASSERT_EQ(list->size, 0, "Initial error list size should be 0");

    free_errors(list);
    return 0;
}

static int test_push_error(void)
{
    ErrorList* list = create_errors();
    ASSERT_NOT_NULL(list, "create_errors should not return NULL");

    ErrorList* result = push_error(list, "main.adn", "Test error", 1, 5, ERROR, LEXER);
    ASSERT_NOT_NULL(result, "push_error should return non-NULL");
    ASSERT_EQ(list->size, 1, "Error list size should be 1 after push");
    ASSERT_EQ(list->errors[0].severity, ERROR, "Error severity should match");
    ASSERT_EQ(list->errors[0].category, LEXER, "Error category should match");

    free_errors(list);
    return 0;
}

static int test_multiple_errors(void)
{
    ErrorList* list = create_errors();
    ASSERT_NOT_NULL(list, "create_errors should not return NULL");

    push_error(list, "main.adn", "Error 1", 1, 0, ERROR, LEXER);
    push_error(list, "main.adn", "Error 2", 2, 0, WARNING, PARSER);
    push_error(list, "main.adn", "Error 3", 3, 0, CRITICAL, SEMANTIC);

    ASSERT_EQ(list->size, 3, "Error list size should be 3");
    ASSERT_EQ(list->errors[0].category, LEXER, "First error category should be LEXER");
    ASSERT_EQ(list->errors[1].category, PARSER, "Second error category should be PARSER");
    ASSERT_EQ(list->errors[2].category, SEMANTIC, "Third error category should be SEMANTIC");

    free_errors(list);
    return 0;
}

static int test_error_message(void)
{
    ErrorList* list = create_errors();
    ASSERT_NOT_NULL(list, "create_errors should not return NULL");

    const char* msg = "Custom error message";
    push_error(list, "main.adn", msg, 5, 10, ERROR, CODEGEN);

    ASSERT_NOT_NULL(list->errors[0].message, "Error message should not be NULL");

    free_errors(list);
    return 0;
}

static int test_error_location(void)
{
    ErrorList* list = create_errors();
    ASSERT_NOT_NULL(list, "create_errors should not return NULL");

    push_error(list, "main.adn", "Error at location", 42, 15, ERROR, GENERIC);

    ASSERT_EQ(list->errors[0].line, 42, "Error line should be 42");
    ASSERT_EQ(list->errors[0].column, 15, "Error column should be 15");
    ASSERT_EQ(list->errors[0].file, "main.adn", "Error file file should be main.adn");

    free_errors(list);
    return 0;
}

int run_diagnostic_tests(TestSuite* suite)
{
    if (!suite)
    {
        return -1;
    }

    test_suite_run_test(suite, "create_errors", test_create_errors);
    test_suite_run_test(suite, "push_error", test_push_error);
    test_suite_run_test(suite, "multiple_errors", test_multiple_errors);
    test_suite_run_test(suite, "error_message", test_error_message);
    test_suite_run_test(suite, "error_location", test_error_location);

    return 0;
}

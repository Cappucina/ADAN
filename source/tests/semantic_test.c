#include "../semantic/semantic.h"

#include "buffer.h"
#include "test.h"

static int test_create_semantic(void)
{
    Buffer* buffer = buffer_create(sizeof(Token));

    Parser* parser = create_semantic(buffer, create_errors());
    ASSERT_NOT_NULL(parser, "Parser should be created successfully");

    free_semantic(parser);

    return 0;
}

static int test_free_semantic(void)
{
    Buffer* buffer = buffer_create(sizeof(Token));

    Parser* parser = create_semantic(buffer, create_errors());
    ASSERT_NOT_NULL(parser, "Parser should be created successfully");

    free_semantic(parser);

    return 0;
}

int run_semantic_tests(TestSuite* suite)
{
    if (!suite)
    {
        return -1;
    }

    test_suite_run_test(suite, "semantic_create_parser", test_create_semantic);
    test_suite_run_test(suite, "semantic_free_parser", test_free_semantic);

    return 0;
}

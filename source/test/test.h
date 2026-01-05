#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct
{
    const char* name;
    bool passed;
    const char* message;
} TestResult;

typedef struct
{
    TestResult* results;
    size_t count;
    size_t capacity;
} TestSuite;

typedef int (*TestFunc)(void);

TestSuite* test_suite_create(void);
void test_suite_free(TestSuite* suite);

int test_suite_add_result(TestSuite* suite, const char* name, bool passed, const char* message);
int test_suite_run_test(TestSuite* suite, const char* name, TestFunc test_func);

void test_suite_print_results(TestSuite* suite);
bool test_suite_all_passed(TestSuite* suite);

int run_all_tests(void);

#define ASSERT_EQ(actual, expected, message) \
    do                                       \
    {                                        \
        if ((actual) != (expected))          \
        {                                    \
            return 1;                        \
        }                                    \
    } while (0)

#define ASSERT_TRUE(condition, message) \
    do                                  \
    {                                   \
        if (!(condition))               \
        {                               \
            return 1;                   \
        }                               \
    } while (0)

#define ASSERT_FALSE(condition, message) \
    do                                   \
    {                                    \
        if ((condition))                 \
        {                                \
            return 1;                    \
        }                                \
    } while (0)

#define ASSERT_NULL(ptr, message) \
    do                            \
    {                             \
        if ((ptr) != NULL)        \
        {                         \
            return 1;             \
        }                         \
    } while (0)

#define ASSERT_NOT_NULL(ptr, message) \
    do                                \
    {                                 \
        if ((ptr) == NULL)            \
        {                             \
            return 1;                 \
        }                             \
    } while (0)

#define ASSERT_STREQ(actual, expected, message) \
    do                                          \
    {                                           \
        if (strcmp((actual), (expected)) != 0)  \
        {                                       \
            return 1;                           \
        }                                       \
    } while (0)

#endif

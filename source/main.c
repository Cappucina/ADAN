
#include "tests/test.h"

int main(void)
{
    int failed = run_all_tests();
    return failed ? 1 : 0;
}
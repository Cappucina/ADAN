#include "tests/test.h"
#include "../include/diagnostics.h"

int main(void)
{
    set_silent_errors(true);
    int failed = run_all_tests();
    return failed ? 1 : 0;
}
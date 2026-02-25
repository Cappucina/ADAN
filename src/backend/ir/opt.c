#include "opt.h"
#include <stdio.h>

void ir_opt_const_fold(void *module)
{
    if (!module) {
        fprintf(stderr, "ir_opt_const_fold called with NULL module. (Warning)\n");
        return;
    }
    fprintf(stderr, "Starting constant-folding optimization. (Info)\n");
    // @todo implement constant folding
    fprintf(stderr, "Finished constant-folding optimization. (Info)\n");
}

void ir_opt_dead_code_elim(void *module)
{
    if (!module) {
        fprintf(stderr, "ir_opt_dead_code_elim called with NULL module. (Warning)\n");
        return;
    }
    fprintf(stderr, "Starting dead-code elimination. (Info)\n");
    // @todo implement dead-code elimination
    fprintf(stderr, "Finished dead-code elimination. (Info)\n");
}

void ir_opt_simplify(void *module)
{
    if (!module) {
        fprintf(stderr, "ir_opt_simplify called with NULL module. (Warning)\n");
        return;
    }
    fprintf(stderr, "Starting IR simplification. (Info)\n");
    // @todo implement IR simplification
    fprintf(stderr, "Finished IR simplification. (Info)\n");
}

void ir_opt_run_all(void *module)
{
    if (!module) {
        fprintf(stderr, "ir_opt_run_all called with NULL module. (Warning)\n");
        return;
    }
    fprintf(stderr, "Running all IR optimizations. (Info)\n");
    ir_opt_const_fold(module);
    ir_opt_simplify(module);
    ir_opt_dead_code_elim(module);
    fprintf(stderr, "Completed all IR optimizations. (Info)\n");
}
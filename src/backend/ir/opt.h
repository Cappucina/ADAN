#ifndef BACKEND_IR_OPT_H
#define BACKEND_IR_OPT_H

void ir_opt_const_fold(void *module);

void ir_opt_dead_code_elim(void *module);

void ir_opt_simplify(void *module);

void ir_opt_run_all(void *module);

#endif

#ifndef CODEGEN_TESTS_H
#define CODEGEN_TESTS_H

void tests_init_target_config();

void test_get_register_name();

void test_assign_stack_offsets();

void test_compute_spill_frame_size();

void test_emit_prologue_epilogue();

void test_get_location();

void test_generate_asm_add();

void test_generate_asm_label_jmp();

void test_generate_asm_comparison();

void tests_print_target_config();

void create_codegen_tests();

#endif
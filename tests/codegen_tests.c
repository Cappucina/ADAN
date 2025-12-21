// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <assert.h>
// #include "liveness.h"
// #include "ir.h"
// #include "codegen.h"
// #include <unistd.h>

// void test_init_target_config() {
// 	TargetConfig cfg;

// 	char* reg_names[] = {"rbx", "r10", "r11", "r12"};
// 	int caller_saved[] = {0, 1};
	
// 	init_target_config(&cfg, 4, reg_names, 2, caller_saved, 8);
	
// 	assert(cfg.available_registers == 4);
// 	assert(cfg.register_names == reg_names);
// 	assert(cfg.caller_saved_count == 2);
// 	assert(cfg.spill_slot_size == 8);
	
// 	// printf("CODE GENERATION PASSED: init_target_config\n");
// }

// void test_get_register_name() {
// 	TargetConfig cfg;
// 	char* reg_names[] = {"rbx", "r10", "r11", "r12"};
	
// 	cfg.available_registers = 4;
// 	cfg.register_names = reg_names;
	
// 	assert(strcmp(get_register_name(&cfg, 0), "rbx") == 0);
// 	assert(strcmp(get_register_name(&cfg, 1), "r10") == 0);
// 	assert(strcmp(get_register_name(&cfg, 3), "r12") == 0);
// 	assert(get_register_name(&cfg, 4) == NULL);
// 	assert(get_register_name(&cfg, -1) == NULL);
// 	assert(get_register_name(NULL, 0) == NULL);
	
// 	// printf("CODE GENERATION PASSED: get_register_name\n");
// }

// void test_assign_stack_offsets() {
// 	TargetConfig cfg;
// 	cfg.spill_slot_size = 8;
	
// 	LiveInterval interval1 = {
// 		.variable_name = "var1",
// 		.registry = -1,
// 		.spilled = true,
// 		.stack_offset = 0,
// 		.next = NULL
// 	};
	
// 	LiveInterval interval2 = {
// 		.variable_name = "var2",
// 		.registry = 0,
// 		.spilled = false,
// 		.stack_offset = 0,
// 		.next = NULL
// 	};
	
// 	LiveInterval interval3 = {
// 		.variable_name = "var3",
// 		.registry = -1,
// 		.spilled = true,
// 		.stack_offset = 0,
// 		.next = NULL
// 	};
	
// 	interval1.next = &interval2;
// 	interval2.next = &interval3;
	
// 	assign_stack_offsets(&interval1, &cfg);
	
// 	assert(interval1.stack_offset == -8);
// 	assert(interval2.stack_offset == 0);
// 	assert(interval3.stack_offset == -16);
	
// 	// printf("CODE GENERATION PASSED: assign_stack_offsets\n");
// }

// void test_compute_spill_frame_size() {
// 	TargetConfig cfg;
// 	cfg.spill_slot_size = 8;
	
// 	LiveInterval interval1 = {
// 		.variable_name = "var1",
// 		.registry = -1,
// 		.spilled = true,
// 		.next = NULL
// 	};
	
// 	LiveInterval interval2 = {
// 		.variable_name = "var2",
// 		.registry = 0,
// 		.spilled = false,
// 		.next = NULL
// 	};
	
// 	LiveInterval interval3 = {
// 		.variable_name = "var3",
// 		.registry = -1,
// 		.spilled = true,
// 		.next = NULL
// 	};
	
// 	interval1.next = &interval2;
// 	interval2.next = &interval3;
	
// 	int frame_size = compute_spill_frame_size(&interval1, &cfg);
// 	assert(frame_size == 16);
	
// 	// printf("CODE GENERATION PASSED: compute_spill_frame_size\n");
// }

// void test_emit_prologue_epilogue() {
// 	FILE* temp_file = tmpfile();
// 	assert(temp_file != NULL);
	
// 	TargetConfig cfg;
// 	cfg.available_registers = 4;
	
// 	emit_prologue(temp_file, &cfg, 32);
	
// 	rewind(temp_file);
// 	char buffer[256];
// 	char output[1024] = {0};
	
// 	while (fgets(buffer, sizeof(buffer), temp_file)) {
// 		strcat(output, buffer);
// 	}
	
// 	// TODO: Full ARM64 code generation requires rewriting all instruction generation
// 	// For now, always check for x86-64 assembly
// 	assert(strstr(output, "pushq %rbp") != NULL);
// 	assert(strstr(output, "movq %rsp, %rbp") != NULL);
// 	assert(strstr(output, "subq $32, %rsp") != NULL);
	
// 	rewind(temp_file);
// 	ftruncate(fileno(temp_file), 0);
	
// 	emit_epilogue(temp_file, &cfg, stack_bytes);
	
// 	rewind(temp_file);
// 	output[0] = '\0';
	
// 	while (fgets(buffer, sizeof(buffer), temp_file)) {
// 		strcat(output, buffer);
// 	}
	
// 	// TODO: Full ARM64 code generation requires rewriting all instruction generation
// 	// For now, always check for x86-64 assembly
// 	assert(strstr(output, "addq $32, %rsp") != NULL);
// 	assert(strstr(output, "popq %rbp") != NULL);
// 	assert(strstr(output, "ret") != NULL);
	
// 	fclose(temp_file);
	
// 	// printf("CODE GENERATION PASSED: emit_prologue and emit_epilogue\n");
// }

// void test_get_location() {
// 	TargetConfig cfg;
// 	// For now, use x86-64 format even on ARM64 (full ARM64 codegen not yet implemented)
// 	char* reg_names[] = {"rbx", "r10", "r11", "r12"};
// 	const char* expected_reg = "%rbx";
// 	const char* expected_stack = "-8(%rbp)";
// 	cfg.available_registers = 4;
// 	cfg.register_names = reg_names;
	
// 	LiveInterval interval1 = {
// 		.variable_name = "_t0",
// 		.registry = 0,
// 		.spilled = false,
// 		.stack_offset = 0,
// 		.next = NULL
// 	};
	
// 	LiveInterval interval2 = {
// 		.variable_name = "_t1",
// 		.registry = -1,
// 		.spilled = true,
// 		.stack_offset = -8,
// 		.next = NULL
// 	};
	
// 	interval1.next = &interval2;
	
// 	char result[64];
	
// 	get_location(result, "_t0", &interval1, &cfg);
// 	assert(strcmp(result, expected_reg) == 0);
	
// 	get_location(result, "_t1", &interval1, &cfg);
// 	assert(strcmp(result, expected_stack) == 0);
	
// 	get_location(result, "5", &interval1, &cfg);
// 	assert(strcmp(result, "$5") == 0);
	
// 	get_location(result, "unknown", &interval1, &cfg);
// 	assert(strcmp(result, "unknown") == 0);
	
// 	// printf("CODE GENERATION PASSED: get_location\n");
// }

// void test_generate_asm_add() {
// 	FILE* temp_file = tmpfile();
// 	assert(temp_file != NULL);
	
// 	TargetConfig cfg;
// 	char* reg_names[] = {"rbx", "r10"};
// 	cfg.available_registers = 2;
// 	cfg.register_names = reg_names;
	
// 	LiveInterval interval1 = {
// 		.variable_name = "_t0",
// 		.registry = 0,
// 		.spilled = false,
// 		.next = NULL
// 	};
	
// 	LiveInterval interval2 = {
// 		.variable_name = "_t1",
// 		.registry = 1,
// 		.spilled = false,
// 		.next = NULL
// 	};
	
// 	LiveInterval interval3 = {
// 		.variable_name = "_t2",
// 		.registry = 0,
// 		.spilled = false,
// 		.next = NULL
// 	};
	
// 	interval1.next = &interval2;
// 	interval2.next = &interval3;
	
// 	IRInstruction instr = {
// 		.op = IR_ADD,
// 		.arg1 = "_t0",
// 		.arg2 = "_t1",
// 		.result = "_t2",
// 		.next = NULL
// 	};
	
// 	generate_asm(&instr, &interval1, &cfg, temp_file, 32);
	
// 	rewind(temp_file);
// 	char buffer[256];
// 	char output[1024] = {0};
	
// 	while (fgets(buffer, sizeof(buffer), temp_file)) {
// 		strcat(output, buffer);
// 	}
	
// 	assert(strstr(output, "movq %rbx, %rbx") != NULL);
// 	assert(strstr(output, "addq %r10, %rbx") != NULL);
	
// 	fclose(temp_file);
	
// 	// printf("CODE GENERATION PASSED: generate_asm IR_ADD\n");
// }

// void test_generate_asm_label_jmp() {
// 	FILE* temp_file = tmpfile();
// 	assert(temp_file != NULL);
	
// 	TargetConfig cfg;
// 	char* reg_names[] = {"rbx"};
// 	cfg.available_registers = 1;
// 	cfg.register_names = reg_names;
	
// 	IRInstruction label_instr = {
// 		.op = IR_LABEL,
// 		.arg1 = "L1",
// 		.arg2 = NULL,
// 		.result = NULL,
// 		.next = NULL
// 	};
	
// 	IRInstruction jmp_instr = {
// 		.op = IR_JMP,
// 		.arg1 = "L2",
// 		.arg2 = NULL,
// 		.result = NULL,
// 		.next = NULL
// 	};
	
// 	label_instr.next = &jmp_instr;
	
// 	generate_asm(&label_instr, NULL, &cfg, temp_file, 32);
	
// 	rewind(temp_file);
// 	char buffer[256];
// 	char output[1024] = {0};
	
// 	while (fgets(buffer, sizeof(buffer), temp_file)) {
// 		strcat(output, buffer);
// 	}
	
// 	assert(strstr(output, "L1:") != NULL);
// 	assert(strstr(output, "jmp L2") != NULL);
	
// 	fclose(temp_file);
	
// 	// printf("CODE GENERATION PASSED: generate_asm IR_LABEL and IR_JMP\n");
// }

// void test_generate_asm_comparison() {
// 	FILE* temp_file = tmpfile();
// 	assert(temp_file != NULL);
	
// 	TargetConfig cfg;
// 	char* reg_names[] = {"rbx", "r10"};
// 	cfg.available_registers = 2;
// 	cfg.register_names = reg_names;
	
// 	LiveInterval interval1 = {
// 		.variable_name = "_t0",
// 		.registry = 0,
// 		.spilled = false,
// 		.next = NULL
// 	};
	
// 	LiveInterval interval2 = {
// 		.variable_name = "_t1",
// 		.registry = 1,
// 		.spilled = false,
// 		.next = NULL
// 	};
	
// 	interval1.next = &interval2;
	
// 	IRInstruction instr = {
// 		.op = IR_JEQ,
// 		.arg1 = "_t0",
// 		.arg2 = "0",
// 		.result = "L_END",
// 		.next = NULL
// 	};
	
// 	generate_asm(&instr, &interval1, &cfg, temp_file, 32);
	
// 	rewind(temp_file);
// 	char buffer[256];
// 	char output[1024] = {0};
	
// 	while (fgets(buffer, sizeof(buffer), temp_file)) {
// 		strcat(output, buffer);
// 	}
	
// 	assert(strstr(output, "cmpq $0, %rbx") != NULL);
// 	assert(strstr(output, "je L_END") != NULL);
	
// 	fclose(temp_file);
	
// 	// printf("CODE GENERATION PASSED: generate_asm comparison operations\n");
// }

// void test_print_target_config() {
// 	FILE* temp_file = tmpfile();
// 	assert(temp_file != NULL);
	
// 	TargetConfig cfg;
// 	char* reg_names[] = {"rbx", "r10", "r11", "r12"};
// 	int caller_saved[] = {0, 1};
	
// 	cfg.available_registers = 4;
// 	cfg.register_names = reg_names;
// 	cfg.caller_saved_indices = caller_saved;
// 	cfg.caller_saved_count = 2;
// 	cfg.spill_slot_size = 8;
	
// 	print_target_config(&cfg, temp_file);
	
// 	rewind(temp_file);
// 	char buffer[256];
// 	char output[2048] = {0};
	
// 	while (fgets(buffer, sizeof(buffer), temp_file)) {
// 		strcat(output, buffer);
// 	}
	
// 	assert(strstr(output, "Target Configuration:") != NULL);
// 	assert(strstr(output, "Available Registers: 4") != NULL);
// 	assert(strstr(output, "Spill Slot Size: 8 bytes") != NULL);
// 	assert(strstr(output, "rbx") != NULL);
// 	assert(strstr(output, "r10") != NULL);
// 	assert(strstr(output, "Caller-Saved Registers: 2") != NULL);
	
// 	fclose(temp_file);
	
// 	// printf("CODE GENERATION PASSED: print_target_config\n");
// }

// void create_codegen_tests() {
// 	test_init_target_config();
// 	test_get_register_name();
// 	test_assign_stack_offsets();
// 	test_compute_spill_frame_size();
// 	test_emit_prologue_epilogue();
// 	test_get_location();
// 	test_generate_asm_add();
// 	test_generate_asm_label_jmp();
// 	test_generate_asm_comparison();
// 	test_print_target_config();
// }


// have to fix with new prototypes
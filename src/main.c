#include <stdio.h>
#include "lexer_tests.h"
#include "parser_tests.h"
#include "codegen_tests.h"
#include "lexer.h"
#include "codegen.h"
#include "ir.h"
#include "liveness.h"

int main(int argc, char** argv) {
	create_lexer_tests();
	create_parser_tests();
	create_codegen_tests();

	init_ir();
	
	IRInstruction* ir_head = NULL;
	IRInstruction* add_instr = create_instruction(IR_ADD, "5", "3", "_t0");
	
	emit(add_instr);
	ir_head = add_instr;
	
	char* register_names[] = {"rbx", "r10", "r11", "r12"};
	int caller_saved[] = {0, 1};

	TargetConfig my_config;
	init_target_config(&my_config, 4, register_names, 2, caller_saved, 8);
	
	LiveInterval interval1 = {
		.variable_name = "_t0",
		.registry = 0,
		.spilled = false,
		.stack_offset = 0,
		.next = NULL
	};

	LiveInterval* my_intervals = &interval1;
	FILE* asm_file = fopen("output.s", "w");

	if (asm_file == NULL) {
		printf("FILE CREATION FAILED: Failed to create Assembly output file!");
		return 1;
	}

	generate_asm(ir_head, my_intervals, &my_config, asm_file);
	fclose(asm_file);

	printf("FILE CREATION PASSED: Created Assembly output file.");
	free_ir();

	return 0;
}
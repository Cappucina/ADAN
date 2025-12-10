#include "codegen.h"
#include <stdlib.h>
#include "liveness.h"
#include "ir.h"
#include "string.h"

static int last_frame_adjust = 0;

bool init_target_config(TargetConfig* cfg, int available_registers, char** register_names, int caller_saved_count, int* caller_saved_indices, int spill_slot_size) {
	cfg->available_registers = available_registers;
	cfg->register_names = register_names;
	cfg->caller_saved_indices = caller_saved_indices;
	cfg->caller_saved_count = caller_saved_count;
	cfg->spill_slot_size = spill_slot_size;
}

void free_target_config(TargetConfig* cfg) {
	if (cfg == NULL) return;
	if (cfg->register_names != NULL) {
		for (int i = 0; i < cfg->available_registers; i++) {
			if (cfg->register_names[i] != NULL) free(cfg->register_names[i]);
		}

		free(cfg->register_names);
	}

	if (cfg->caller_saved_indices != NULL) free(cfg->caller_saved_indices);
	free(cfg);
}

const char* get_register_name(const TargetConfig* cfg, int index) {
	if (cfg == NULL) return NULL;
	if (cfg->register_names == NULL) return NULL;
	if (index < 0 || index >= cfg->available_registers) return NULL;

	return cfg->register_names[index];
}

void get_location(char* result_buffer, char* variable_name, LiveInterval* intervals, const TargetConfig* cfg) {
	if (is_digit(variable_name[0])) {
		sprintf(result_buffer, "$%s", variable_name);
		return;
	}

	LiveInterval* current = intervals;
	while (current != NULL) {
		if (strcmp(current->variable_name, variable_name) == 0) {
			if (current->registry != -1) {
				char* register_name = cfg->register_names[current->registry];
				sprintf(result_buffer, "%%%s", register_name);
				return;
			} else {
				sprintf(result_buffer, "%d(%%rbp)", current->stack_offset);
				return;
			}
		}

		current = current->next;
	}

	strcpy(result_buffer, variable_name);
}

void generate_asm(IRInstruction* ir_head, LiveInterval* intervals, const TargetConfig* cfg, FILE* out) {
	IRInstruction* current = ir_head;

	char loc1[64];
	char loc2[64];
	char result_loc[64];

	while (current != NULL) {
		loc1[0] = '\0'; 
		loc2[0] = '\0'; 
		result_loc[0] = '\0';

		if (current->arg1 != NULL) get_location(loc1, current->arg1, intervals, cfg);
		if (current->arg2 != NULL) get_location(loc2, current->arg2, intervals, cfg);
		if (current->result != NULL) get_location(result_loc, current->result, intervals, cfg);

		switch (current->op) {
			case IR_ADD:
				fprintf(out, "  movq %s, %s\n", loc1, result_loc);
				fprintf(out, "  addq %s, %s\n", loc2, result_loc);
				break;

			case IR_SUB:
				fprintf(out, "  movq %s, %s\n", loc1, result_loc);
				fprintf(out, "  subq %s, %s\n", loc2, result_loc);
				break;

			case IR_MUL:
				fprintf(out, "  movq %s, %%rax\n", loc1);
				fprintf(out, "  imulq %s\n", loc2);
				fprintf(out, "  movq %%rax, %s\n", result_loc);
				break;

			case IR_DIV:
				fprintf(out, "  movq %s, %%rax\n", loc1);
				fprintf(out, "  cqto\n");
				fprintf(out, "  idivq %s\n", loc2);
				fprintf(out, "  movq %%rax, %s\n", result_loc);
				break;

			case IR_ASSIGN:
				fprintf(out, "  movq %s, %s\n", loc1, result_loc);
				break;

			case IR_LABEL:
				fprintf(out, "%s:\n", current->arg1);
				break;

			case IR_JMP:
				fprintf(out, "  jmp %s\n", current->arg1);
				break;

			case IR_JEQ:
				fprintf(out, "  cmpq %s, %s\n", loc2, loc1);
				fprintf(out, "  je %s\n", result_loc);
				break;

			case IR_JNE:
				fprintf(out, "  cmpq %s, %s\n", loc2, loc1);
				fprintf(out, "  jne %s\n", result_loc);
				break;

			case IR_LT:
				fprintf(out, "  cmpq %s, %s\n", loc2, loc1);
				fprintf(out, "  jl %s\n", result_loc);
				break;

			case IR_GT:
				fprintf(out, "  cmpq %s, %s\n", loc2, loc1);
				fprintf(out, "  jg %s\n", result_loc);
				break;

			case IR_LTE:
				fprintf(out, "  cmpq %s, %s\n", loc2, loc1);
				fprintf(out, "  jle %s\n", result_loc);
				break;

			case IR_GTE:
				fprintf(out, "  cmpq %s, %s\n", loc2, loc1);
				fprintf(out, "  jge %s\n", result_loc);
				break;

			case IR_PARAM:
				fprintf(out, "  pushq %s\n", loc1);
				break;

			case IR_CALL:
				fprintf(out, "  call %s\n", loc1);
				fprintf(out, "  movq %%rax, %s\n", result_loc);
				break;
		}

		current = current->next;
	}
}

void emit_prologue(FILE* out, const TargetConfig* cfg, int stack_bytes) {
	if (out == NULL) return;
	if (stack_bytes < 0) stack_bytes = 0;

	int padded = stack_bytes;
	int rem = padded % 16;

	if (rem != 0) padded += 16 - rem;

	last_frame_adjust = padded;

	fprintf(out, "	pushq %%rbp\n");
	fprintf(out, "	movq %%rsp, %%rbp\n");
	if (padded > 0) {
		fprintf(out, "	subq $%d, %%rsp\n", padded);
	}
}

void emit_epilogue(FILE* out, const TargetConfig* cfg) {
	if (out == NULL) return;
	if (last_frame_adjust > 0) {
		fprintf(out, "addq $%d, %%rsp\n", last_frame_adjust);
	}

	fprintf(out, "popq %%rbp\n");
	fprintf(out, "ret\n");
}

void assign_stack_offsets(LiveInterval* intervals, const TargetConfig* cfg) {
	int current_stack_position = 0;
	LiveInterval* current = intervals;
	
	while (current != NULL) {
		if (current->registry == -1) {
			current_stack_position -= cfg->spill_slot_size;
			current->stack_offset = current_stack_position;
		}

		current = current->next;
	}
}

int compute_spill_frame_size(LiveInterval* intervals, const TargetConfig* cfg) {
	int total_size = 0;
	LiveInterval* current = intervals;

	while (current != NULL) {
		if (current->registry == -1) total_size += cfg->spill_slot_size;
		current = current->next;
	}

	return total_size;
}

void print_target_config(const TargetConfig* cfg, FILE* out) {
	if (out == NULL || cfg == NULL) return;

	fprintf(out, "Target Configuration:\n");
	fprintf(out, "  Available Registers: %d\n", cfg->available_registers);
	fprintf(out, "  Spill Slot Size: %d bytes\n", cfg->spill_slot_size);	
	fprintf(out, "  Register Names:\n");

	if (cfg->register_names == NULL) return;
	for (int i = 0; i < cfg->available_registers; i++) {
		if (cfg->register_names[i] != NULL) {
			fprintf(out, "	[%d] %s\n", i, cfg->register_names[i]);
		}
	}

	fprintf(out, "  Caller-Saved Registers: %d\n", cfg->caller_saved_count);
	if (cfg->caller_saved_indices != NULL && cfg->caller_saved_count > 0) {
		fprintf(out, "	Indices: ");
		for (int i = 0; i < cfg->caller_saved_count; i++) {
			fprintf(out, "%d", cfg->caller_saved_indices[i]);
			if (i < cfg->caller_saved_count - 1) fprintf(out, ", ");
		}

		fprintf(out, "\n");
	}
}
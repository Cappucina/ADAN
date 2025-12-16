#include "codegen.h"
#include <stdlib.h>
#include "liveness.h"
#include "ir.h"
#include "string.h"


// working out arm but tests I need to look into fixing

// #ifdef __aarch64__
// 	#define ARCH_ARM64 1
// #elif defined(__x86_64__) || defined(_M_X64)
// 	#define ARCH_X86_64 1
// #else
	#define ARCH_X86_64 1  // Default to x86_64
// #endif

static int last_frame_adjust = 0;

static const char* mangle_symbol(const char* name) {
	#ifdef __APPLE__
		static char mangled[256];
		snprintf(mangled, sizeof(mangled), "_%s", name);
		return mangled;
	#else
		return name;
	#endif
}

bool init_target_config(TargetConfig* cfg, int available_registers, char** register_names, int caller_saved_count, int* caller_saved_indices, int spill_slot_size) {
	cfg->available_registers = available_registers;
	cfg->register_names = register_names;
	cfg->caller_saved_indices = caller_saved_indices;
	cfg->caller_saved_count = caller_saved_count;
	cfg->spill_slot_size = spill_slot_size;

	return true;
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
	if (variable_name[0] == '+' || variable_name[0] == '-') {
		if (is_digit(variable_name[1])) {
			sprintf(result_buffer, "$%s", variable_name);
			return;
		}
	} else if (is_digit(variable_name[0])) {
		sprintf(result_buffer, "$%s", variable_name);
		return;
	}

	if (variable_name[0] == '.' && variable_name[1] == 'S' && variable_name[2] == 'T' && variable_name[3] == 'R') {
#if ARCH_X86_64
		sprintf(result_buffer, "%s(%%rip)", variable_name);
#elif ARCH_ARM64
		sprintf(result_buffer, "%s", variable_name);
#endif
		return;
	}

	if (variable_name[0] == 'G' && variable_name[1] == '_') {
#if ARCH_X86_64
		sprintf(result_buffer, "%s(%%rip)", variable_name);
#elif ARCH_ARM64
		sprintf(result_buffer, "%s", variable_name);
#endif
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
#if ARCH_X86_64
				sprintf(result_buffer, "%d(%%rbp)", current->stack_offset);
#elif ARCH_ARM64
				sprintf(result_buffer, "[x29, #%d]", current->stack_offset);
#endif
				return;
			}
		}

		current = current->next;
	}

	strcpy(result_buffer, variable_name);
}

void generate_asm(IRInstruction* ir_head, LiveInterval* intervals, const TargetConfig* cfg, FILE* out, int stack_bytes) {
	IRInstruction* current = ir_head;
	
	char arg_locs[8][64];
	int arg_is_lea[8];
	int arg_count = 0;
	
#if ARCH_X86_64
	const char* arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
#elif ARCH_ARM64
	const char* arg_regs[] = {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"};
#endif
	
	char loc1[64];
	char loc2[64];
	char result_loc[64];

	bool in_function = false;

	while (current != NULL) {
		loc1[0] = '\0'; 
		loc2[0] = '\0'; 
		result_loc[0] = '\0';

		if (current->arg1 != NULL) get_location(loc1, current->arg1, intervals, cfg);
		if (current->arg2 != NULL) get_location(loc2, current->arg2, intervals, cfg);
		if (current->result != NULL) get_location(result_loc, current->result, intervals, cfg);

		int reset_args = 1;

		printf("%d", current->op);

		switch (current->op) {
#if ARCH_X86_64
			case IR_ADD:
				{
					int result_is_mem = strchr(result_loc, '(') != NULL;
					if (result_is_mem) {
						fprintf(out, "movq %s, %%r11\n", loc1);
						fprintf(out, "addq %s, %%r11\n", loc2);
						fprintf(out, "movq %%r11, %s\n", result_loc);
					} else {
						fprintf(out, "movq %s, %s\n", loc1, result_loc);
						fprintf(out, "addq %s, %s\n", loc2, result_loc);
					}
				}
				break;

			case IR_SUB:
				{
					int result_is_mem = strchr(result_loc, '(') != NULL;
					if (result_is_mem) {
						fprintf(out, "movq %s, %%r11\n", loc1);
						fprintf(out, "subq %s, %%r11\n", loc2);
						fprintf(out, "movq %%r11, %s\n", result_loc);
					} else {
						fprintf(out, "movq %s, %s\n", loc1, result_loc);
						fprintf(out, "subq %s, %s\n", loc2, result_loc);
					}
				}
				break;

			case IR_MUL:
				fprintf(out, "movq %s, %%rax\n", loc1);
				fprintf(out, "imulq %s\n", loc2);
				fprintf(out, "movq %%rax, %s\n", result_loc);
				break;

			case IR_DIV:
				fprintf(out, "movq %s, %%rax\n", loc1);
				fprintf(out, "cqto\n");
				if (loc2[0] == '$') {
					fprintf(out, "movq %s, %%r11\n", loc2);
					fprintf(out, "idivq %%r11\n");
				} else {
					fprintf(out, "idivq %s\n", loc2);
				}
				fprintf(out, "movq %%rax, %s\n", result_loc);
				break;

			case IR_MOD:
				fprintf(out, "movq %s, %%rax\n", loc1);
				fprintf(out, "cqto\n");
				if (loc2[0] == '$') {
					fprintf(out, "movq %s, %%r11\n", loc2);
					fprintf(out, "idivq %%r11\n");
				} else {
					fprintf(out, "idivq %s\n", loc2);
				}
				fprintf(out, "movq %%rdx, %s\n", result_loc);
				break;

			case IR_POW:
				if (loc1[0] == '$') {
					fprintf(out, "movq %s, %%rax\n", loc1);
					fprintf(out, "cvtsi2sdq %%rax, %%xmm0\n");
				} else {
					fprintf(out, "cvtsi2sdq %s, %%xmm0\n", loc1);
				}
				if (loc2[0] == '$') {
					fprintf(out, "movq %s, %%rax\n", loc2);
					fprintf(out, "cvtsi2sdq %%rax, %%xmm1\n");
				} else {
					fprintf(out, "cvtsi2sdq %s, %%xmm1\n", loc2);
				}
				fprintf(out, "call pow\n");
				fprintf(out, "cvttsd2siq %%xmm0, %%r11\n");
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;

			case IR_AND:
				if (strchr(result_loc, '(') != NULL) {
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "andq %s, %%r11\n", loc2);
					fprintf(out, "movq %%r11, %s\n", result_loc);
				} else {
					fprintf(out, "movq %s, %s\n", loc1, result_loc);
					fprintf(out, "addq %s, %s\n", loc2, result_loc);
				}
				break;

			
			case IR_OR:
    if (strchr(result_loc, '(') != NULL) {
        // memory destination
        fprintf(out, "movq %s, %%r11\n", loc1);
        fprintf(out, "orq %s, %%r11\n", loc2);
        fprintf(out, "movq %%r11, %s\n", result_loc);
    } else {
        // register destination
        fprintf(out, "movq %s, %s\n", loc1, result_loc);
        fprintf(out, "orq %s, %s\n", loc2, result_loc);
    }
    break;


			case IR_ASSIGN:
				if (current->arg1[0] == '.' && current->arg1[1] == 'S' && current->arg1[2] == 'T' && current->arg1[3] == 'R') {
					fprintf(out, "leaq %s, %%rax\n", loc1);
					fprintf(out, "movq %%rax, %s\n", result_loc);
				} else {
					int src_is_mem = (strchr(loc1, '(') != NULL);
					int dst_is_mem = (strchr(result_loc, '(') != NULL);
					if (src_is_mem && dst_is_mem) {
						fprintf(out, "movq %s, %%r11\n", loc1);
						fprintf(out, "movq %%r11, %s\n", result_loc);
					} else {
						fprintf(out, "movq %s, %s\n", loc1, result_loc);
					}
				}
				break;

			case IR_ADDR_OF:
				fprintf(out, "leaq %s, %%r11\n", loc1);
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;

			case IR_DEREF: {
				int src_is_mem = (strchr(loc1, '(') != NULL);
				if (src_is_mem) {
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "movq (%%r11), %%r11\n");
				} else {
					fprintf(out, "movq (%s), %%r11\n", loc1);
				}
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;
			}

			case IR_LOAD_IDX: {
				fprintf(out, "movq %s, %%r10\n", loc1);
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "movq (%%r10, %%r11, 8), %%r11\n");
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;
			}

			case IR_STORE_IDX: {
				fprintf(out, "movq %s, %%r10\n", loc1);
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "movq %s, %%rax\n", result_loc);
				fprintf(out, "movq %%rax, (%%r10, %%r11, 8)\n");
				break;
			}
			
			case IR_LABEL: {
				int is_block_label = (current->arg1 && current->arg1[0] == '_');
				if (!is_block_label) {
					if (in_function) emit_epilogue(out, cfg);
					#ifdef __APPLE__
						fprintf(out, "_%s:\n", current->arg1);
					#else
						fprintf(out, "%s:\n", current->arg1);
					#endif
					emit_prologue(out, cfg, stack_bytes);
					in_function = true;
				} else {
					fprintf(out, "%s:\n", current->arg1);
				}
				break;
			}

			case IR_RETURN:
				fprintf(out, "movq %s, %%rax\n", loc1);
				break;

			case IR_JMP:
				fprintf(out, "jmp %s\n", current->arg1);
				break;

			case IR_JEQ: {
				int loc2_is_zero = (strcmp(loc2, "$0") == 0);
				if (loc2_is_zero && loc1[0] != '$') {
					fprintf(out, "cmpq %s, %s\n", loc2, loc1);
				} else if (loc1[0] == '$' && loc2[0] == '$') {
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "cmpq %s, %%r11\n", loc2);
				} else if (loc1[0] == '$') {
					fprintf(out, "cmpq %s, %s\n", loc1, loc2);
				} else if (loc2[0] == '$' || loc2[0] == '%') {
					fprintf(out, "cmpq %s, %s\n", loc2, loc1);
				} else {
					fprintf(out, "movq %s, %%r11\n", loc2);
					fprintf(out, "cmpq %%r11, %s\n", loc1);
				}
				fprintf(out, "je %s\n", current->result);
				break;
			}

			case IR_CONTINUE:
				fprintf(out, "jmp %s\n", current->result);
				break;

			case IR_JNE: {
				int loc1_is_imm = (loc1[0] == '$');
				int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
				if (loc1_is_imm) {
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "cmpq %s, %%r11\n", loc2);
				} else if (loc2_is_reg_or_imm) {
					fprintf(out, "cmpq %s, %s\n", loc2, loc1);
				} else {
					fprintf(out, "movq %s, %%r11\n", loc2);
					fprintf(out, "cmpq %%r11, %s\n", loc1);
				}
				fprintf(out, "jne %s\n", current->result);
				break;
			}

			case IR_LT: {
				int loc1_is_imm = (loc1[0] == '$');
				int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
				if (loc1_is_imm) {
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "cmpq %s, %%r11\n", loc2);
				} else if (loc2_is_reg_or_imm) {
					fprintf(out, "cmpq %s, %s\n", loc2, loc1);
				} else {
					fprintf(out, "movq %s, %%r11\n", loc2);
					fprintf(out, "cmpq %%r11, %s\n", loc1);
				}
				fprintf(out, "jl %s\n", current->result);
				break;
			}

			case IR_GT: {
				int loc1_is_imm = (loc1[0] == '$');
				int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
				if (loc1_is_imm) {
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "cmpq %s, %%r11\n", loc2);
				} else if (loc2_is_reg_or_imm) {
					fprintf(out, "cmpq %s, %s\n", loc2, loc1);
				} else {
					fprintf(out, "movq %s, %%r11\n", loc2);
					fprintf(out, "cmpq %%r11, %s\n", loc1);
				}
				fprintf(out, "jg %s\n", current->result);
				break;
			}

			case IR_LTE: {
				int loc1_is_imm = (loc1[0] == '$');
				int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
				if (loc1_is_imm) {
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "cmpq %s, %%r11\n", loc2);
				} else if (loc2_is_reg_or_imm) {
					fprintf(out, "cmpq %s, %s\n", loc2, loc1);
				} else {
					fprintf(out, "movq %s, %%r11\n", loc2);
					fprintf(out, "cmpq %%r11, %s\n", loc1);
				}
				fprintf(out, "jle %s\n", current->result);
				break;
			}

			case IR_GTE: {
				int loc1_is_imm = (loc1[0] == '$');
				int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
				if (loc1_is_imm) {
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "cmpq %s, %%r11\n", loc2);
				} else if (loc2_is_reg_or_imm) {
					fprintf(out, "cmpq %s, %s\n", loc2, loc1);
				} else {
					fprintf(out, "movq %s, %%r11\n", loc2);
					fprintf(out, "cmpq %%r11, %s\n", loc1);
				}
				fprintf(out, "jge %s\n", current->result);
				break;
			}

			case IR_PARAM: {
				reset_args = 0;
				if (arg_count < 8) {
					strcpy(arg_locs[arg_count], loc1);
					arg_is_lea[arg_count] = (current->arg1 && strncmp(current->arg1, ".STR", 4) == 0);
					arg_count++;
				}
				break;
			}

			case IR_CALL: {
				reset_args = 0;
				int pass = arg_count;
				if (pass > 6) pass = 6;
				for (int i = 0; i < pass; i++) {
					if (arg_is_lea[i]) {
						fprintf(out, "leaq %s, %%%s\n", arg_locs[i], arg_regs[i]);
					} else {
						fprintf(out, "movq %s, %%%s\n", arg_locs[i], arg_regs[i]);
					}
				}
				arg_count = 0;
				const char* target = current->arg1 ? current->arg1 : loc1;
				#ifdef __APPLE__
					fprintf(out, "call _%s\n", target);
				#else
					fprintf(out, "call %s\n", target);
				#endif
				fprintf(out, "movq %%rax, %s\n", result_loc);
				break;
			}

			case IR_NEG:
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "negq %%r11\n");
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;

			case IR_NOT:
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "xorq $1, %%r11\n");
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;

			case IR_BIT_AND:
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "andq %s, %%r11\n", loc2);
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;

			case IR_BIT_OR: {
    int result_is_mem = strchr(result_loc, '(') != NULL;

    // Always use a register for the operation
    fprintf(out, "movq %s, %%r11\n", loc1);   // load first operand
    fprintf(out, "orq %s, %%r11\n", loc2);    // OR with second operand

    // Store result back
    if (result_is_mem) {
        fprintf(out, "movq %%r11, %s\n", result_loc);
    } else {
        fprintf(out, "movq %%r11, %s\n", result_loc);
    }
    break;
}

			case IR_BIT_XOR:
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "xorq %s, %%r11\n", loc2);
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;

			case IR_SHL:
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "movq %s, %%rcx\n", loc2);
				fprintf(out, "shlq %%cl, %%r11\n");
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;

			case IR_SHR:
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "movq %s, %%rcx\n", loc2);
				fprintf(out, "shrq %%cl, %%r11\n");
				fprintf(out, "movq %%r11, %s\n", result_loc);
				break;
#elif ARCH_ARM64
			case IR_ADD: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				// Load loc1 into x11
				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				// Load loc2 into x12
				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "add x11, x11, x12\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_SUB: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "sub x11, x11, x12\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_MUL: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x0, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x0, %s\n", loc1);
				} else {
					fprintf(out, "mov x0, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x1, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x1, %s\n", loc2);
				} else {
					fprintf(out, "mov x1, %s\n", loc2);
				}

				fprintf(out, "mul x0, x0, x1\n");

				if (result_is_mem) {
					fprintf(out, "str x0, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x0\n", result_loc);
				}
				break;
			}

			case IR_DIV: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x0, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x0, %s\n", loc1);
				} else {
					fprintf(out, "mov x0, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x1, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x1, %s\n", loc2);
				} else {
					fprintf(out, "mov x1, %s\n", loc2);
				}

				fprintf(out, "sdiv x2, x0, x1\n");

				if (result_is_mem) {
					fprintf(out, "str x2, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x2\n", result_loc);
				}
				break;
			}

			case IR_MOD: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x0, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x0, %s\n", loc1);
				} else {
					fprintf(out, "mov x0, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x1, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x1, %s\n", loc2);
				} else {
					fprintf(out, "mov x1, %s\n", loc2);
				}

				fprintf(out, "sdiv x2, x0, x1\n");     // quotient
				fprintf(out, "msub x3, x2, x1, x0\n"); // remainder = x0 - x2*x1

				if (result_is_mem) {
					fprintf(out, "str x3, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x3\n", result_loc);
				}
				break;
			}

			case IR_POW: {
				int result_is_mem = strchr(result_loc, '[') != NULL;

				if (loc1[0] == '$') {
					fprintf(out, "mov x0, %s\n", loc1);
				} else if (strchr(loc1, '[')) {
					fprintf(out, "ldr x0, %s\n", loc1);
				} else {
					fprintf(out, "mov x0, %s\n", loc1);
				}
				fprintf(out, "scvtf d0, x0\n");

				if (loc2[0] == '$') {
					fprintf(out, "mov x1, %s\n", loc2);
				} else if (strchr(loc2, '[')) {
					fprintf(out, "ldr x1, %s\n", loc2);
				} else {
					fprintf(out, "mov x1, %s\n", loc2);
				}
				fprintf(out, "scvtf d1, x1\n");

				fprintf(out, "bl pow\n");

				fprintf(out, "fcvtzs x11, d0\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_ASSIGN: {
				int src_is_mem = strchr(loc1, '[') != NULL;
				int dst_is_mem = strchr(result_loc, '[') != NULL;

				if (src_is_mem && dst_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
					fprintf(out, "str x11, %s\n", result_loc);
				} else if (src_is_mem && !dst_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
					fprintf(out, "mov %s, x11\n", result_loc);
				} else if (!src_is_mem && dst_is_mem) {
					fprintf(out, "mov x11, %s\n", loc1);
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, %s\n", result_loc, loc1);
				}
				break;
			}

			case IR_ADDR_OF: {
				// For ARM64, if loc1 is a label (no brackets or prefix), use adrp + add sequence otherwise fallback
				int is_label = (loc1[0] != '[' && loc1[0] != '$' && loc1[0] != '%');
				if (is_label) {
					#ifdef __APPLE__
					fprintf(out, "adrp x11, _%s\n", current->arg1);
					fprintf(out, "add x11, x11, :lo12:_ %s\n", current->arg1);
					#else
					fprintf(out, "adrp x11, %s\n", current->arg1);
					fprintf(out, "add x11, x11, :lo12:%s\n", current->arg1);
					#endif
					fprintf(out, "mov %s, x11\n", result_loc);
				} else if (strchr(loc1, '[') != NULL) {
					// Address is stack slot or memory reference
					// Attempt to parse offset from [x29, #offset]
					// Since we can't parse here easily, just emit add with zero offset
					// TODO: refine offset calculation
					fprintf(out, "mov x11, x29\n");
					fprintf(out, "mov %s, x11\n", result_loc);
				} else {
					// Fallback
					fprintf(out, "// TODO: IR_ADDR_OF unhandled case\n");
					fprintf(out, "mov x11, x29\n");
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_DEREF: {
				int src_is_mem = strchr(loc1, '[') != NULL;
				if (src_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
					fprintf(out, "ldr x11, [x11]\n");
				} else {
					fprintf(out, "ldr x11, [%s]\n", loc1);
				}
				int result_is_mem = strchr(result_loc, '[') != NULL;
				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_LOAD_IDX: {
				fprintf(out, "ldr x10, %s\n", loc1);
				fprintf(out, "ldr x11, %s\n", loc2);
				fprintf(out, "ldr x11, [x10, x11, lsl #3]\n");
				int result_is_mem = strchr(result_loc, '[') != NULL;
				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_STORE_IDX: {
				fprintf(out, "ldr x10, %s\n", loc1);
				fprintf(out, "ldr x11, %s\n", loc2);
				int src_is_mem = strchr(result_loc, '[') != NULL;
				if (src_is_mem) {
					fprintf(out, "ldr x0, %s\n", result_loc);
					fprintf(out, "str x0, [x10, x11, lsl #3]\n");
				} else {
					fprintf(out, "str %s, [x10, x11, lsl #3]\n", result_loc);
				}
				break;
			}

			case IR_LABEL: {
				int is_block_label = (current->arg1 && current->arg1[0] == '_');
				if (!is_block_label) {
					if (in_function) emit_epilogue(out, cfg);
					#ifdef __APPLE__
						fprintf(out, "_%s:\n", current->arg1);
					#else
						fprintf(out, "%s:\n", current->arg1);
					#endif
					emit_prologue(out, cfg, stack_bytes);
					in_function = true;
				} else {
					fprintf(out, "%s:\n", current->arg1);
				}
				break;
			}

			case IR_RETURN: {
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				if (loc1_is_mem) {
					fprintf(out, "ldr x0, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x0, %s\n", loc1);
				} else {
					fprintf(out, "mov x0, %s\n", loc1);
				}
				break;
			}

			case IR_JMP: {
				fprintf(out, "b %s\n", current->arg1);
				break;
			}

			case IR_JEQ: {
				int loc2_is_zero = (strcmp(loc2, "$0") == 0);
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				// Load loc1 into x11
				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				// Load loc2 into x12
				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "cmp x11, x12\n");
				fprintf(out, "b.eq %s\n", current->result);
				break;
			}

			case IR_CONTINUE:
				fprintf(out, "b %s\n", current->result);
				break;

			case IR_JNE: {
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "cmp x11, x12\n");
				fprintf(out, "b.ne %s\n", current->result);
				break;
			}

			case IR_LT: {
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "cmp x11, x12\n");
				fprintf(out, "b.lt %s\n", current->result);
				break;
			}

			case IR_GT: {
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "cmp x11, x12\n");
				fprintf(out, "b.gt %s\n", current->result);
				break;
			}

			case IR_LTE: {
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "cmp x11, x12\n");
				fprintf(out, "b.le %s\n", current->result);
				break;
			}

			case IR_GTE: {
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "cmp x11, x12\n");
				fprintf(out, "b.ge %s\n", current->result);
				break;
			}

			case IR_PARAM: {
				reset_args = 0;
				if (arg_count < 8) {
					strcpy(arg_locs[arg_count], loc1);
					arg_is_lea[arg_count] = (current->arg1 && strncmp(current->arg1, ".STR", 4) == 0);
					arg_count++;
				}
				break;
			}

			case IR_CALL: {
				reset_args = 0;
				int pass = arg_count;
				if (pass > 8) pass = 8;
				for (int i = 0; i < pass; i++) {
					if (arg_is_lea[i]) {
						fprintf(out, "adrp x%d, %s\n", i, arg_locs[i]);
						fprintf(out, "add x%d, x%d, :lo12:%s\n", i, i, arg_locs[i]);
					} else {
						fprintf(out, "mov %s, %s\n", arg_regs[i], arg_locs[i]);
					}
				}
				arg_count = 0;
				const char* target = current->arg1 ? current->arg1 : loc1;
				#ifdef __APPLE__
					fprintf(out, "bl _%s\n", target);
				#else
					fprintf(out, "bl %s\n", target);
				#endif
				int result_is_mem = strchr(result_loc, '[') != NULL;
				if (result_is_mem) {
					fprintf(out, "str x0, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x0\n", result_loc);
				}
				break;
			}

			case IR_NEG: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				fprintf(out, "neg x11, x11\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_NOT: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				fprintf(out, "eor x11, x11, #1\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_BIT_AND: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "and x11, x11, x12\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_BIT_OR: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "orr x11, x11, x12\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_BIT_XOR: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x12, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x12, %s\n", loc2);
				} else {
					fprintf(out, "mov x12, %s\n", loc2);
				}

				fprintf(out, "eor x11, x11, x12\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_SHL: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x2, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x2, %s\n", loc2);
				} else {
					fprintf(out, "mov x2, %s\n", loc2);
				}

				fprintf(out, "lsl x11, x11, x2\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			case IR_SHR: {
				int result_is_mem = strchr(result_loc, '[') != NULL;
				int loc1_is_mem = strchr(loc1, '[') != NULL;
				int loc2_is_mem = strchr(loc2, '[') != NULL;

				if (loc1_is_mem) {
					fprintf(out, "ldr x11, %s\n", loc1);
				} else if (loc1[0] == '$') {
					fprintf(out, "mov x11, %s\n", loc1);
				} else {
					fprintf(out, "mov x11, %s\n", loc1);
				}

				if (loc2_is_mem) {
					fprintf(out, "ldr x2, %s\n", loc2);
				} else if (loc2[0] == '$') {
					fprintf(out, "mov x2, %s\n", loc2);
				} else {
					fprintf(out, "mov x2, %s\n", loc2);
				}

				fprintf(out, "lsr x11, x11, x2\n");

				if (result_is_mem) {
					fprintf(out, "str x11, %s\n", result_loc);
				} else {
					fprintf(out, "mov %s, x11\n", result_loc);
				}
				break;
			}

			default:
				break;
#endif
		}

		if (reset_args) arg_count = 0;

		current = current->next;
	}

	if (in_function) emit_epilogue(out, cfg);
}

void emit_prologue(FILE* out, const TargetConfig* cfg, int stack_bytes) {
	if (out == NULL) return;
	if (stack_bytes < 0) stack_bytes = 0;

	last_frame_adjust = stack_bytes;

#if ARCH_X86_64
	// TODO: Full ARM64 code generation requires rewriting all instruction generation
	fprintf(out, "\tpushq %%rbp\n");
	fprintf(out, "\tmovq %%rsp, %%rbp\n");
	if (stack_bytes > 0) {
		fprintf(out, "\tsubq $%d, %%rsp\n", stack_bytes);
	}
#elif ARCH_ARM64
	fprintf(out, "\tstp x29, x30, [sp, #-16]!\n");
	fprintf(out, "\tmov x29, sp\n");
	if (stack_bytes > 0) {
		// Align to 16 bytes
		int adjust = ((stack_bytes + 15) / 16) * 16;
		last_frame_adjust = adjust;
		fprintf(out, "\tsub sp, sp, #%d\n", adjust);
	}
#endif
}

void emit_epilogue(FILE* out, const TargetConfig* cfg) {
	if (out == NULL) return;
#if ARCH_X86_64
	// TODO: Full ARM64 code generation requires rewriting all instruction generation
	if (last_frame_adjust > 0) {
		fprintf(out, "addq $%d, %%rsp\n", last_frame_adjust);
	}
	fprintf(out, "popq %%rbp\n");
	fprintf(out, "ret\n");
#elif ARCH_ARM64
	if (last_frame_adjust > 0) {
		fprintf(out, "add sp, sp, #%d\n", last_frame_adjust);
	}
	fprintf(out, "ldp x29, x30, [sp], #16\n");
	fprintf(out, "ret\n");
#endif
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

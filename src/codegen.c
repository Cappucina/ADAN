#include "codegen.h"
#include <stdlib.h>
#include "liveness.h"
#include "ir.h"
#include "string.h"

static int last_frame_adjust = 0;

static const char *mangle_symbol(const char *name)
{
#ifdef __APPLE__
	static char mangled[256];
	snprintf(mangled, sizeof(mangled), "_%s", name);
	return mangled;
#else
	return name;
#endif
}

bool init_target_config(TargetConfig *cfg, int available_registers, char **register_names, int caller_saved_count, int *caller_saved_indices, int spill_slot_size)
{
	cfg->available_registers = available_registers;
	cfg->register_names = register_names;
	cfg->caller_saved_indices = caller_saved_indices;
	cfg->caller_saved_count = caller_saved_count;
	cfg->spill_slot_size = spill_slot_size;

	return true;
}

void free_target_config(TargetConfig *cfg)
{
	if (cfg == NULL)
		return;
	if (cfg->register_names != NULL)
	{
		for (int i = 0; i < cfg->available_registers; i++)
		{
			if (cfg->register_names[i] != NULL)
				free(cfg->register_names[i]);
		}

		free(cfg->register_names);
	}

	if (cfg->caller_saved_indices != NULL)
		free(cfg->caller_saved_indices);
	free(cfg);
}

const char *get_register_name(const TargetConfig *cfg, int index)
{
	if (cfg == NULL)
		return NULL;
	if (cfg->register_names == NULL)
		return NULL;
	if (index < 0 || index >= cfg->available_registers)
		return NULL;

	return cfg->register_names[index];
}

void get_location(char *result_buffer, char *variable_name, LiveInterval *intervals, const TargetConfig *cfg)
{
	if (variable_name[0] == '+' || variable_name[0] == '-')
	{
		if (is_digit(variable_name[1]))
		{
			sprintf(result_buffer, "$%s", variable_name);
			return;
		}
	}
	else if (is_digit(variable_name[0]))
	{
		sprintf(result_buffer, "$%s", variable_name);
		return;
	}

	if (variable_name[0] == '.' && variable_name[1] == 'S' && variable_name[2] == 'T' && variable_name[3] == 'R')
	{
		sprintf(result_buffer, "%s(%%rip)", variable_name);
		return;
	}

	if (variable_name[0] == 'G' && variable_name[1] == '_')
	{
		sprintf(result_buffer, "%s(%%rip)", variable_name);
		return;
	}

	LiveInterval *current = intervals;
	while (current != NULL)
	{
		if (strcmp(current->variable_name, variable_name) == 0)
		{
			if (current->registry != -1)
			{
				char *register_name = cfg->register_names[current->registry];
				sprintf(result_buffer, "%%%s", register_name);
				return;
			}
			else
			{
				sprintf(result_buffer, "%d(%%rbp)", current->stack_offset);
				return;
			}
		}

		current = current->next;
	}

	strcpy(result_buffer, variable_name);
}

void generate_asm(IRInstruction *ir_head, LiveInterval *intervals, const TargetConfig *cfg, FILE *out, int stack_bytes)
{
	IRInstruction *current = ir_head;

	char arg_locs[8][64];
	int arg_is_lea[8];
	int arg_count = 0;

	const char *arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

	char loc1[64];
	char loc2[64];
	char result_loc[64];

	bool in_function = false;

	while (current != NULL)
	{
		loc1[0] = '\0';
		loc2[0] = '\0';
		result_loc[0] = '\0';

		if (current->arg1 != NULL)
			get_location(loc1, current->arg1, intervals, cfg);
		if (current->arg2 != NULL)
			get_location(loc2, current->arg2, intervals, cfg);
		if (current->result != NULL)
			get_location(result_loc, current->result, intervals, cfg);

		int reset_args = 1;

		switch (current->op)
		{
		case IR_ADD:
		{
			int result_is_mem = strchr(result_loc, '(') != NULL;
			int arg1_is_mem = strchr(loc1, '(') != NULL;
			int arg2_is_mem = strchr(loc2, '(') != NULL;

			if (result_is_mem && (arg1_is_mem || arg2_is_mem))
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "addq %s, %%r11\n", loc2);
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %s, %s\n", loc1, result_loc);
				fprintf(out, "addq %s, %s\n", loc2, result_loc);
			}
		}
		break;

		case IR_SUB:
		{
			int result_is_mem = strchr(result_loc, '(') != NULL;
			if (result_is_mem)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "subq %s, %%r11\n", loc2);
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %s, %s\n", loc1, result_loc);
				fprintf(out, "subq %s, %s\n", loc2, result_loc);
			}
		}
		break;

case IR_STRING_EQ:
case IR_STRING_NEQ:
{
    if (current->arg1[0] == '.' && current->arg1[1] == 'S' &&
        current->arg1[2] == 'T' && current->arg1[3] == 'R') {
        fprintf(out, "leaq %s, %%rdi\n", loc1);
    } else {
        fprintf(out, "movq %s, %%rdi\n", loc1);
    }

    if (current->arg2[0] == '.' && current->arg2[1] == 'S' &&
        current->arg2[2] == 'T' && current->arg2[3] == 'R') {
        fprintf(out, "leaq %s, %%rsi\n", loc2);
    } else {
        fprintf(out, "movq %s, %%rsi\n", loc2);
    }

#ifdef __APPLE__
    fprintf(out, "call _string_eq\n");
#else
    fprintf(out, "call string_eq\n");
#endif

    fprintf(out, "movq %%rax, %s\n", result_loc);

    if (current->op == IR_STRING_NEQ) {
        fprintf(out, "xor $1, %s\n", result_loc);
    }

#ifdef __APPLE__
    fprintf(out, "call _to_bool\n");
#else
    fprintf(out, "call to_bool\n");
#endif

    break;
}

		case IR_MUL:
			fprintf(out, "movq %s, %%rax\n", loc1);
			fprintf(out, "imulq %s\n", loc2);
			fprintf(out, "movq %%rax, %s\n", result_loc);
			break;

		case IR_DIV:
			fprintf(out, "movq %s, %%rax\n", loc1);
			fprintf(out, "cqto\n");
			if (loc2[0] == '$')
			{
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "idivq %%r11\n");
			}
			else
			{
				fprintf(out, "idivq %s\n", loc2);
			}
			fprintf(out, "movq %%rax, %s\n", result_loc);
			break;

		case IR_MOD:
			fprintf(out, "movq %s, %%rax\n", loc1);
			fprintf(out, "cqto\n");
			if (loc2[0] == '$')
			{
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "idivq %%r11\n");
			}
			else
			{
				fprintf(out, "idivq %s\n", loc2);
			}
			fprintf(out, "movq %%rdx, %s\n", result_loc);
			break;

		case IR_POW:
		{
			if (loc1[0] == '$')
			{
				fprintf(out, "movq %s, %%rax\n", loc1);
				fprintf(out, "cvtsi2sdq %%rax, %%xmm0\n");
			}
			else
			{
				fprintf(out, "cvtsi2sdq %s, %%xmm0\n", loc1);
			}

			if (loc2[0] == '$')
			{
				fprintf(out, "movq %s, %%rax\n", loc2);
				fprintf(out, "cvtsi2sdq %%rax, %%xmm1\n");
			}
			else
			{
				fprintf(out, "cvtsi2sdq %s, %%xmm1\n", loc2);
			}

#ifdef __APPLE__
			fprintf(out, "call _pow\n");
#else
			fprintf(out, "subq $8, %%rsp\n");
			fprintf(out, "call pow@PLT\n");
			fprintf(out, "addq $8, %%rsp\n");
#endif

			fprintf(out, "cvttsd2siq %%xmm0, %%r11\n");
			fprintf(out, "movq %%r11, %s\n", result_loc);

			break;
		}

		case IR_AND:
			if (strchr(result_loc, '(') != NULL)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "andq %s, %%r11\n", loc2);
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %s, %s\n", loc1, result_loc);
				fprintf(out, "addq %s, %s\n", loc2, result_loc);
			}
			break;

		case IR_OR:
			if (strchr(result_loc, '(') != NULL)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "orq %s, %%r11\n", loc2);
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %s, %s\n", loc1, result_loc);
				fprintf(out, "orq %s, %s\n", loc2, result_loc);
			}
			break;

		case IR_ASSIGN:
			if (current->arg1[0] == '.' && current->arg1[1] == 'S' && current->arg1[2] == 'T' && current->arg1[3] == 'R')
			{
				fprintf(out, "leaq %s, %%rax\n", loc1);
				fprintf(out, "movq %%rax, %s\n", result_loc);
			}
			else
			{
				int src_is_mem = (strchr(loc1, '(') != NULL);
				int dst_is_mem = (strchr(result_loc, '(') != NULL);
				if (src_is_mem && dst_is_mem)
				{
					fprintf(out, "movq %s, %%r11\n", loc1);
					fprintf(out, "movq %%r11, %s\n", result_loc);
				}
				else
				{
					fprintf(out, "movq %s, %s\n", loc1, result_loc);
				}
			}
			break;

		case IR_ADDR_OF:
			fprintf(out, "leaq %s, %%r11\n", loc1);
			fprintf(out, "movq %%r11, %s\n", result_loc);
			break;

		case IR_DEREF:
		{
			int src_is_mem = (strchr(loc1, '(') != NULL);
			if (src_is_mem)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "movq (%%r11), %%r11\n");
			}
			else
			{
				fprintf(out, "movq (%s), %%r11\n", loc1);
			}
			fprintf(out, "movq %%r11, %s\n", result_loc);
			break;
		}

		case IR_LOAD_IDX:
		{
			fprintf(out, "movq %s, %%r10\n", loc1);
			fprintf(out, "movq %s, %%r11\n", loc2);
			fprintf(out, "movq (%%r10, %%r11, 8), %%r11\n");
			fprintf(out, "movq %%r11, %s\n", result_loc);
			break;
		}

		case IR_STORE_IDX:
		{
			fprintf(out, "movq %s, %%r10\n", loc1);
			fprintf(out, "movq %s, %%r11\n", loc2);
			fprintf(out, "movq %s, %%rax\n", result_loc);
			fprintf(out, "movq %%rax, (%%r10, %%r11, 8)\n");
			break;
		}

		case IR_LABEL:
		{
			int is_block_label = (current->arg1 && current->arg1[0] == '_');
			if (!is_block_label)
			{
				if (in_function)
					emit_epilogue(out, cfg, stack_bytes);
#ifdef __APPLE__
				fprintf(out, "_%s:\n", current->arg1);
#else
				fprintf(out, "%s:\n", current->arg1);
#endif
				emit_prologue(out, cfg, stack_bytes);
				in_function = true;
			}
			else
			{
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

		case IR_JEQ:
		{
			int loc2_is_zero = (strcmp(loc2, "$0") == 0);
			if (loc2_is_zero && loc1[0] != '$')
			{
				fprintf(out, "cmpq %s, %s\n", loc2, loc1);
			}
			else if (loc1[0] == '$' && loc2[0] == '$')
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "cmpq %s, %%r11\n", loc2);
			}
			else if (loc1[0] == '$')
			{
				fprintf(out, "cmpq %s, %s\n", loc1, loc2);
			}
			else if (loc2[0] == '$' || loc2[0] == '%')
			{
				fprintf(out, "cmpq %s, %s\n", loc2, loc1);
			}
			else
			{
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "cmpq %%r11, %s\n", loc1);
			}
			fprintf(out, "je %s\n", current->result);
			break;
		}

		case IR_CONTINUE:
			fprintf(out, "jmp %s\n", current->result);
			break;

		case IR_JNE:
		{
			int loc1_is_imm = (loc1[0] == '$');
			int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
			if (loc1_is_imm)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "cmpq %s, %%r11\n", loc2);
			}
			else if (loc2_is_reg_or_imm)
			{
				fprintf(out, "cmpq %s, %s\n", loc2, loc1);
			}
			else
			{
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "cmpq %%r11, %s\n", loc1);
			}
			fprintf(out, "jne %s\n", current->result);
			break;
		}

		case IR_LT:
		{
			int loc1_is_imm = (loc1[0] == '$');
			int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
			if (loc1_is_imm)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "cmpq %s, %%r11\n", loc2);
			}
			else if (loc2_is_reg_or_imm)
			{
				fprintf(out, "cmpq %s, %s\n", loc2, loc1);
			}
			else
			{
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "cmpq %%r11, %s\n", loc1);
			}
			fprintf(out, "jl %s\n", current->result);
			break;
		}

		case IR_GT:
		{
			int loc1_is_imm = (loc1[0] == '$');
			int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
			if (loc1_is_imm)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "cmpq %s, %%r11\n", loc2);
			}
			else if (loc2_is_reg_or_imm)
			{
				fprintf(out, "cmpq %s, %s\n", loc2, loc1);
			}
			else
			{
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "cmpq %%r11, %s\n", loc1);
			}
			fprintf(out, "jg %s\n", current->result);
			break;
		}

		case IR_LTE:
		{
			int loc1_is_imm = (loc1[0] == '$');
			int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
			if (loc1_is_imm)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "cmpq %s, %%r11\n", loc2);
			}
			else if (loc2_is_reg_or_imm)
			{
				fprintf(out, "cmpq %s, %s\n", loc2, loc1);
			}
			else
			{
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "cmpq %%r11, %s\n", loc1);
			}
			fprintf(out, "jle %s\n", current->result);
			break;
		}

		case IR_GTE:
		{
			int loc1_is_imm = (loc1[0] == '$');
			int loc2_is_reg_or_imm = (loc2[0] == '$' || loc2[0] == '%');
			if (loc1_is_imm)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "cmpq %s, %%r11\n", loc2);
			}
			else if (loc2_is_reg_or_imm)
			{
				fprintf(out, "cmpq %s, %s\n", loc2, loc1);
			}
			else
			{
				fprintf(out, "movq %s, %%r11\n", loc2);
				fprintf(out, "cmpq %%r11, %s\n", loc1);
			}
			fprintf(out, "jge %s\n", current->result);
			break;
		}

		case IR_PARAM:
		{
			reset_args = 0;
			if (arg_count < 8)
			{
				strcpy(arg_locs[arg_count], loc1);
				arg_is_lea[arg_count] = (current->arg1 && strncmp(current->arg1, ".STR", 4) == 0);
				arg_count++;
			}
			break;
		}

		case IR_CALL:
		{
			reset_args = 0;
			int pass = arg_count;
			if (pass > 6)
				pass = 6;
			for (int i = 0; i < pass; i++)
			{
				if (arg_is_lea[i])
					fprintf(out, "leaq %s, %%%s\n", arg_locs[i], arg_regs[i]);
				else
					fprintf(out, "movq %s, %%%s\n", arg_locs[i], arg_regs[i]);
			}
			arg_count = 0;
			const char *target = current->arg1 ? current->arg1 : loc1;
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

		case IR_BIT_OR:
		{
			int result_is_mem = strchr(result_loc, '(') != NULL;

			fprintf(out, "movq %s, %%r11\n", loc1);
			fprintf(out, "orq %s, %%r11\n", loc2);

			if (result_is_mem)
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			break;
		}

		case IR_BIT_NOT:
		{
			int result_is_mem = strchr(result_loc, '(') != NULL;
			if (result_is_mem)
			{
				fprintf(out, "movq %s, %%r11\n", loc1);
				fprintf(out, "notq %%r11\n");
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %s, %s\n", loc1, result_loc);
				fprintf(out, "notq %s\n", result_loc);
			}
			break;
		}

		case IR_BIT_ZERO_FILL_LEFT_SHIFT:
		{
			int result_is_mem = strchr(result_loc, '(') != NULL;

			fprintf(out, "movq %s, %%r11\n", loc1);
			fprintf(out, "movb %s, %%cl\n", loc2);
			fprintf(out, "salq %%cl, %%r11\n");

			if (result_is_mem)
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			break;
		}

		case IR_BIT_ZERO_FILL_RIGHT_SHIFT:
		{
			int result_is_mem = strchr(result_loc, '(') != NULL;

			fprintf(out, "movq %s, %%r11\n", loc1);
			fprintf(out, "movb %s, %%cl\n", loc2);
			fprintf(out, "shrq %%cl, %%r11\n");

			if (result_is_mem)
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			break;
		}

		case IR_BIT_SIGNED_RIGHT_SHIFT:
		{
			int result_is_mem = strchr(result_loc, '(') != NULL;

			fprintf(out, "movq %s, %%r11\n", loc1);
			fprintf(out, "movb %s, %%cl\n", loc2);
			fprintf(out, "sarq %%cl, %%r11\n");

			if (result_is_mem)
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			break;
		}

		case IR_BIT_AND:
		{
			int result_is_mem = strchr(result_loc, '(') != NULL;

			fprintf(out, "movq %s, %%r11\n", loc1);
			fprintf(out, "andq %s, %%r11\n", loc2);

			if (result_is_mem)
			{
				fprintf(out, "movq %%r11, %s\n", result_loc);
			}
			else
			{
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
		}

		if (reset_args)
			arg_count = 0;

		current = current->next;
	}

	if (in_function)
		emit_epilogue(out, cfg, stack_bytes);
}

void emit_prologue(FILE *out, const TargetConfig *cfg, int stack_bytes)
{
	if (!out)
		return;
	fprintf(out, "\tpushq %%rbp\n");
	fprintf(out, "\tmovq %%rsp, %%rbp\n");

	if (stack_bytes > 0)
	{
		int aligned_stack = (stack_bytes + 8 + 15) & ~15;
		fprintf(out, "\tsubq $%d, %%rsp\n", aligned_stack);
	}
}

void emit_epilogue(FILE *out, const TargetConfig *cfg, int stack_bytes)
{
	if (!out)
		return;

	if (stack_bytes > 0)
	{
		int aligned_stack = (stack_bytes + 8 + 15) & ~15;
		fprintf(out, "\taddq $%d, %%rsp\n", aligned_stack);
	}

	fprintf(out, "\tpopq %%rbp\n");
	fprintf(out, "\tret\n");
}

void assign_stack_offsets(LiveInterval *intervals, const TargetConfig *cfg)
{
	int frame_size = compute_spill_frame_size(intervals, cfg);
	int current_stack_position = 0;
	
	LiveInterval *cur = intervals;
	while (cur)
	{
		if (cur->registry == -1)
		{
			int slot = (cfg->spill_slot_size + 7) & ~7;
			current_stack_position -= slot;
			cur->stack_offset = current_stack_position;
		}
		cur = cur->next;
	}
}

int compute_spill_frame_size(LiveInterval *intervals, const TargetConfig *cfg)
{
	int total_size = 0;
	LiveInterval *cur = intervals;
	while (cur)
	{
		if (cur->registry == -1)
			total_size += cfg->spill_slot_size;
		cur = cur->next;
	}

	return (total_size + 15) & ~15;
}

void print_target_config(const TargetConfig *cfg, FILE *out)
{
	if (out == NULL || cfg == NULL)
		return;

	fprintf(out, "Target Configuration:\n");
	fprintf(out, "  Available Registers: %d\n", cfg->available_registers);
	fprintf(out, "  Spill Slot Size: %d bytes\n", cfg->spill_slot_size);
	fprintf(out, "  Register Names:\n");

	if (cfg->register_names == NULL)
		return;
	for (int i = 0; i < cfg->available_registers; i++)
	{
		if (cfg->register_names[i] != NULL)
		{
			fprintf(out, "	[%d] %s\n", i, cfg->register_names[i]);
		}
	}

	fprintf(out, "  Caller-Saved Registers: %d\n", cfg->caller_saved_count);
	if (cfg->caller_saved_indices != NULL && cfg->caller_saved_count > 0)
	{
		fprintf(out, "	Indices: ");
		for (int i = 0; i < cfg->caller_saved_count; i++)
		{
			fprintf(out, "%d", cfg->caller_saved_indices[i]);
			if (i < cfg->caller_saved_count - 1)
				fprintf(out, ", ");
		}

		fprintf(out, "\n");
	}
}

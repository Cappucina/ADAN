#include <stdio.h>
#include <string.h>
#include "ir.h"
#include "ast.h"

static IRInstruction* ir_head = NULL;
static IRInstruction* ir_tail = NULL;

static int temp_counter = 0;

void init_ir() {
	ir_head = NULL;
	ir_tail = NULL;
	
	temp_counter = 0;
}

char* new_temporary() {
	char buffer[32];
	snprintf(buffer, 32, "_t%d", temp_counter);
	temp_counter++;
	return strdup(buffer);
}

IRInstruction* create_instruction(IROp op, char* arg1, char* arg2, char* result) {
	IRInstruction* new_instruction = malloc(sizeof(IRInstruction));
	
	new_instruction->op = op;
	new_instruction->arg1 = arg1 ? strdup(arg1) : NULL;
	new_instruction->arg2 = arg2 ? strdup(arg2) : NULL;
	new_instruction->result = result ? strdup(result) : NULL;
	new_instruction->next = NULL;

	return new_instruction;
}

void emit(IRInstruction* instruction) {
	if (ir_head == NULL) {
		ir_head = instruction;
		ir_tail = instruction;
	} else {
		ir_tail->next = instruction;
		ir_tail = instruction;
	}
}

void print_ir() {
	IRInstruction* current = ir_head;
	while (current != NULL) {
		switch (current->op) {
			case IR_ADD:
				printf("%s = %s + %s\n", current->result, current->arg1, current->arg2);
				break;
			
			case IR_SUB:
				printf("%s = %s - %s\n", current->result, current->arg1, current->arg2);
				break;
			
			case IR_MUL:
				printf("%s = %s * %s\n", current->result, current->arg1, current->arg2);
				break;
			
			case IR_DIV:
				printf("%s = %s / %s\n", current->result, current->arg1, current->arg2);
				break;
			
			case IR_ASSIGN:
				printf("%s = %s\n", current->result, current->arg1);
				break;
			
			case IR_LABEL:
				printf("%s:\n", current->result);
				break;
			
			case IR_JMP:
				printf("GOTO %s\n", current->result);
				break;
			
			case IR_JEQ:
				printf("IF %s == %s GOTO %s\n", current->arg1, current->arg2, current->result);
				break;
			
			case IR_PRINT:
				printf("PRINT %s\n", current->arg1);
				break;
		}
		current = current->next;
	}
}

char* generate_ir(ASTNode* node) {

}

void free_ir() {
	while (ir_head != NULL) {
		IRInstruction* temp = ir_head;
		ir_head = ir_head->next;
		
		free(temp->arg1);
		free(temp->arg2);
		free(temp->result);
		free(temp);
	}
}
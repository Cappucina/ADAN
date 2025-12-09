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
	if (node == NULL) return NULL;
	
	switch (node->type) {
		case AST_LITERAL: {
			// Literals return their text value directly
			return node->token.text ? strdup(node->token.text) : NULL;
		}
		
		case AST_IDENTIFIER: {
			// Identifiers return their name
			return node->token.text ? strdup(node->token.text) : NULL;
		}
		
		case AST_BINARY_OP: {
			// Generate IR for left and right operands
			if (node->child_count < 2) return NULL;
			
			char* left = generate_ir(node->children[0]);
			char* right = generate_ir(node->children[1]);
			
			if (!left || !right) {
				free(left);
				free(right);
				return NULL;
			}
			
			// Create a temporary for the result
			char* result = new_temporary();
			
			// Determine the operation based on the operator token
			IROp op;
			if (strcmp(node->token.text, "+") == 0) {
				op = IR_ADD;
			} else if (strcmp(node->token.text, "-") == 0) {
				op = IR_SUB;
			} else if (strcmp(node->token.text, "*") == 0) {
				op = IR_MUL;
			} else if (strcmp(node->token.text, "/") == 0) {
				op = IR_DIV;
			} else {
				// Unknown operator, just create an assignment
				free(left);
				free(right);
				return result;
			}
			
			// Emit the instruction
			IRInstruction* instr = create_instruction(op, left, right, result);
			emit(instr);
			
			free(left);
			free(right);
			
			return result;
		}
		
		case AST_UNARY_OP: {
			// Generate IR for the operand
			if (node->child_count < 1) return NULL;
			
			char* operand = generate_ir(node->children[0]);
			if (!operand) return NULL;
			
			// Create a temporary for the result
			char* result = new_temporary();
			
			// For unary minus, emit: result = 0 - operand
			if (strcmp(node->token.text, "-") == 0) {
				IRInstruction* instr = create_instruction(IR_SUB, "0", operand, result);
				emit(instr);
			}
			// For logical NOT, we'd need additional IR operations (not implemented)
			
			free(operand);
			return result;
		}
		
		case AST_ASSIGNMENT: {
			// Generate IR for the right-hand side
			if (node->child_count < 2) return NULL;
			
			// children[0] is the identifier, children[1] is the expression or type
			char* identifier = node->children[0]->token.text;
			char* value = NULL;
			
			// Check if we have an initial value expression (3 children means declaration with initialization)
			if (node->child_count >= 3 && node->children[2]) {
				value = generate_ir(node->children[2]);
			} else if (node->child_count >= 2 && node->children[1]) {
				// Could be direct assignment or type (check node type)
				if (node->children[1]->type != AST_TYPE) {
					value = generate_ir(node->children[1]);
				}
			}
			
			if (value) {
				// Emit assignment: identifier = value
				IRInstruction* instr = create_instruction(IR_ASSIGN, value, NULL, identifier);
				emit(instr);
				free(value);
			}
			
			return identifier ? strdup(identifier) : NULL;
		}
		
		case AST_FUNCTION_CALL: {
			// For function calls like print, emit IR_PRINT
			if (node->token.text && strcmp(node->token.text, "printf") == 0) {
				// Generate IR for arguments
				for (int i = 0; i < node->child_count; i++) {
					char* arg = generate_ir(node->children[i]);
					if (arg) {
						IRInstruction* instr = create_instruction(IR_PRINT, arg, NULL, NULL);
						emit(instr);
						free(arg);
					}
				}
			}
			return NULL;
		}
		
		case AST_BLOCK:
		case AST_STATEMENT:
		case AST_PROGRAM: {
			// Process all children sequentially
			for (int i = 0; i < node->child_count; i++) {
				char* result = generate_ir(node->children[i]);
				free(result);
			}
			return NULL;
		}
		
		case AST_IF: {
			// IF statements: children[0] = condition, children[1] = then block, children[2] = else block (optional)
			if (node->child_count < 2) return NULL;
			
			// Generate labels
			static int label_counter = 0;
			char else_label[32], end_label[32];
			snprintf(else_label, 32, "L%d", label_counter++);
			snprintf(end_label, 32, "L%d", label_counter++);
			
			// Generate condition code
			char* cond = generate_ir(node->children[0]);
			
			// Jump to else/end if condition is false (simplified - should use comparison)
			IRInstruction* jmp_instr = create_instruction(IR_JEQ, cond ? cond : "0", "0", 
			                                               node->child_count > 2 ? else_label : end_label);
			emit(jmp_instr);
			free(cond);
			
			// Generate then block
			generate_ir(node->children[1]);
			
			// Jump to end
			if (node->child_count > 2) {
				IRInstruction* jmp_end = create_instruction(IR_JMP, NULL, NULL, end_label);
				emit(jmp_end);
				
				// Else label
				IRInstruction* else_lbl = create_instruction(IR_LABEL, NULL, NULL, else_label);
				emit(else_lbl);
				
				// Generate else block
				generate_ir(node->children[2]);
			}
			
			// End label
			IRInstruction* end_lbl = create_instruction(IR_LABEL, NULL, NULL, end_label);
			emit(end_lbl);
			
			return NULL;
		}
		
		case AST_WHILE: {
			// WHILE loops: children[0] = condition, children[1] = body
			if (node->child_count < 2) return NULL;
			
			// Generate labels
			static int while_counter = 0;
			char start_label[32], end_label[32];
			snprintf(start_label, 32, "L%d", while_counter++);
			snprintf(end_label, 32, "L%d", while_counter++);
			
			// Start label
			IRInstruction* start_lbl = create_instruction(IR_LABEL, NULL, NULL, start_label);
			emit(start_lbl);
			
			// Generate condition
			char* cond = generate_ir(node->children[0]);
			
			// Jump to end if condition is false
			IRInstruction* jmp_end = create_instruction(IR_JEQ, cond ? cond : "0", "0", end_label);
			emit(jmp_end);
			free(cond);
			
			// Generate body
			generate_ir(node->children[1]);
			
			// Jump back to start
			IRInstruction* jmp_start = create_instruction(IR_JMP, NULL, NULL, start_label);
			emit(jmp_start);
			
			// End label
			IRInstruction* end_lbl = create_instruction(IR_LABEL, NULL, NULL, end_label);
			emit(end_lbl);
			
			return NULL;
		}
		
		case AST_FOR: {
			// FOR loops: typically has initialization, condition, increment, and body
			// Process all children sequentially
			for (int i = 0; i < node->child_count; i++) {
				char* result = generate_ir(node->children[i]);
				free(result);
			}
			return NULL;
		}
		
		case AST_COMPARISON:
		case AST_BINARY_EXPR:
		case AST_EXPRESSION: {
			// Recursively process expression nodes
			if (node->child_count > 0) {
				return generate_ir(node->children[0]);
			}
			return NULL;
		}
		
		default: {
			// For unhandled node types, try to process children
			for (int i = 0; i < node->child_count; i++) {
				char* result = generate_ir(node->children[i]);
				free(result);
			}
			return NULL;
		}
	}
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
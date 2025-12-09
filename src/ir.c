#include <stdio.h>
#include <string.h>
#include "ir.h"
#include "ast.h"

// 
//  TODO:
// 		- Do IR for if-statements
// 		- Control flow
// 

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
		}
		current = current->next;
	}
}

char* generate_ir(ASTNode* node) {
	if (node == NULL) return NULL;
	switch (node->type) {
		case AST_LITERAL: {
			if (node->token.text) return strdup(node->token.text);
			return NULL;
		}

		case AST_IDENTIFIER: {
			if (node->token.text) return strdup(node->token.text);
			return NULL;
		}

		case AST_BINARY_OP:
		case AST_BINARY_EXPR: {
			if (node->child_count < 2) return NULL;

			char* left = generate_ir(node->children[0]);
			char* right = generate_ir(node->children[1]);
			if (!left || !right) {
				free(left);
				free(right);

				return NULL;
			}

			IROp opcode;
			switch (node->token.type) {
				case TOKEN_PLUS:
					opcode = IR_ADD;
					break;

				case TOKEN_MINUS:
					opcode = IR_SUB;
					break;

				case TOKEN_ASTERISK:
					opcode = IR_MUL;
					break;

				case TOKEN_SLASH:
					opcode = IR_DIV;
					break;

				default:
					free(left);
					free(right);

					return NULL;
			}

			char* temp = new_temporary();
			IRInstruction* new_instruction = create_instruction(opcode, left, right, temp);
			
			emit(new_instruction);
			
			free(left);
			free(right);

			return temp;
		}

		case AST_ASSIGNMENT: {
			if (node->child_count < 2) return NULL;

			ASTNode* id = node->children[0];
			ASTNode* expr = node->children[1];

			char* val = generate_ir(expr);
			if (!val) return NULL;

			const char* name = id->token.text ? id->token.text : NULL;
			if (!name) {
				free(val);
				return NULL;
			}

			char* dest = strdup(name);
			IRInstruction* new_instruction = create_instruction(IR_ASSIGN, val, NULL, dest);

			emit(new_instruction);

			free(val);
			free(dest);

			return strdup(name);
		}

		case AST_DECLARATION: {
			if (node->child_count >= 3) {
				ASTNode* id = node->children[0];
				ASTNode* expr = node->children[2];

				char* val = generate_ir(expr);
				if (!val) return NULL;

				const char* name = id->token.text ? id->token.text : NULL;
				if (!name) {
					free(val);
					return NULL;
				}

				char* dest = strdup(name);
				IRInstruction* new_instruction = create_instruction(IR_ASSIGN, val, NULL, dest);

				emit(new_instruction);

				free(val);
				free(dest);

				return strdup(name);
			}

			if (node->child_count >= 1 && node->children[0]->token.text) return strdup(node->children[0]->token.text);
			return NULL;
		}

		case AST_FUNCTION_CALL: {
			if (node->child_count < 1) return NULL;

			ASTNode* function = node->children[0];
			const char* fname = function->token.text ? function->token.text : NULL;

			if (!fname) return NULL;
			if (node->child_count > 1) {
				ASTNode* params = node->children[1];

				if (params && params->type == AST_PARAMS) {
					for (int i = 0; i < params->child_count; i++) {
						char* arg = generate_ir(params->children[i]);
						IRInstruction* new_instruction = create_instruction(IR_PARAM, arg, NULL, NULL);

						emit(new_instruction);
						free(arg);
					}
				}
			}

			char* result_var = new_temporary();
			IRInstruction* new_instruction = create_instruction(IR_CALL, fname, NULL, result_var);

			emit(new_instruction);

			return result_var;
		}

		default:
			return NULL;
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
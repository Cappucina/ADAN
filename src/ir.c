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
				printf("%s:\n", current->arg1);
				break;
			
			case IR_JMP:
				printf("GOTO %s\n", current->arg1);
				break;
			
			case IR_JEQ:
				printf("IF %s == %s GOTO %s\n", current->arg1, current->arg2, current->result);
				break;
			
			case IR_JNE:
				printf("IF %s != %s GOTO %s\n", current->arg1, current->arg2, current->result);
				break;
			
			case IR_LT:
				printf("IF %s < %s GOTO %s\n", current->arg1, current->arg2, current->result);
				break;
			
			case IR_GT:
				printf("IF %s > %s GOTO %s\n", current->arg1, current->arg2, current->result);
				break;
			
			case IR_LTE:
				printf("IF %s <= %s GOTO %s\n", current->arg1, current->arg2, current->result);
				break;
			
			case IR_GTE:
				printf("IF %s >= %s GOTO %s\n", current->arg1, current->arg2, current->result);
				break;
			
			case IR_PARAM:
				printf("PARAM %s\n", current->arg1);
				break;
			
			case IR_CALL:
				printf("%s = CALL %s\n", current->result, current->arg1);
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
			char* fname = function->token.text ? function->token.text : NULL;
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

		case AST_UNARY_OP:
		case AST_UNARY_EXPR: {
			if (node->child_count < 1) return NULL;
			char* operand = generate_ir(node->children[0]);
			if (!operand) return NULL;
			char* temp = new_temporary();
			if (node->token.type == TOKEN_MINUS || node->token.type == TOKEN_NOT) {
				IRInstruction* new_instruction = create_instruction(IR_SUB, operand, NULL, temp);
				emit(new_instruction);
				free(operand);
				return temp;
			}

			free(operand);
			free(temp);

			return NULL;
		}

		case AST_COMPARISON: {
			if (node->child_count < 2) return NULL;
			char* left = generate_ir(node->children[0]);
			char* right = generate_ir(node->children[1]);
			if (!left || !right) {
				free(left);
				free(right);
				
				return NULL;
			}

			char* result = new_temporary();
			
			IROp opcode;
			switch (node->token.type) {
				case TOKEN_EQUALS:
					opcode = IR_JEQ;
					break;
				case TOKEN_NOT_EQUALS:
					opcode = IR_JNE;
					break;
				case TOKEN_LESS:
					opcode = IR_LT;
					break;
				case TOKEN_GREATER:
					opcode = IR_GT;
					break;
				case TOKEN_LESS_EQUALS:
					opcode = IR_LTE;
					break;
				case TOKEN_GREATER_EQUALS:
					opcode = IR_GTE;
					break;
				default:
					opcode = IR_JEQ;
					break;
			}

			IRInstruction* new_instruction = create_instruction(opcode, left, right, result);
			emit(new_instruction);

			free(left);
			free(right);

			return result;
		}

		case AST_BLOCK: {
			char* last_result = NULL;
			for (int i = 0; i < node->child_count; i++) {
				if (last_result) free(last_result);
				last_result = generate_ir(node->children[i]);
			}

			return last_result;
		}

		case AST_IF: {
			if (node->child_count < 2) return NULL;
			
			char* cond_temp = generate_ir(node->children[0]);
			if (!cond_temp) return NULL;
			
			char* l_else = new_temporary();
			char* l_end = new_temporary();
			
			IRInstruction* jump_else = create_instruction(IR_JEQ, cond_temp, "0", l_else);
			emit(jump_else);
			
			generate_ir(node->children[1]);
			
			IRInstruction* jump_end = create_instruction(IR_JMP, l_end, NULL, NULL);
			emit(jump_end);
			
			IRInstruction* else_label = create_instruction(IR_LABEL, l_else, NULL, NULL);
			emit(else_label);
			
			if (node->child_count > 2) {
				generate_ir(node->children[2]);
			}
			
			IRInstruction* end_label = create_instruction(IR_LABEL, l_end, NULL, NULL);
			emit(end_label);
			
			free(cond_temp);
			free(l_else);
			free(l_end);
			
			return NULL;
		}

		case AST_WHILE: {
			if (node->child_count < 2) return NULL;

			char* loop_label = new_temporary();
			char* end_label = new_temporary();

			IRInstruction* label_inst = create_instruction(IR_LABEL, loop_label, NULL, NULL);
			emit(label_inst);

			char* condition = generate_ir(node->children[0]);
			if (!condition) {
				free(loop_label);
				free(end_label);
				return NULL;
			}

			IRInstruction* jump_inst = create_instruction(IR_JEQ, condition, "0", end_label);
			emit(jump_inst);

			generate_ir(node->children[1]);

			IRInstruction* loop_jump = create_instruction(IR_JMP, loop_label, NULL, NULL);
			emit(loop_jump);

			IRInstruction* end_label_inst = create_instruction(IR_LABEL, end_label, NULL, NULL);
			emit(end_label_inst);

			free(loop_label);
			free(end_label);
			free(condition);

			return NULL;
		}

		case AST_FOR: {
			if (node->child_count < 3) return NULL;

			char* init_result = generate_ir(node->children[0]);
			free(init_result);

			char* loop_label = new_temporary();
			char* end_label = new_temporary();

			IRInstruction* label_inst = create_instruction(IR_LABEL, loop_label, NULL, NULL);
			emit(label_inst);

			char* condition = generate_ir(node->children[1]);
			if (!condition) {
				free(loop_label);
				free(end_label);
				return NULL;
			}

			IRInstruction* jump_inst = create_instruction(IR_JEQ, condition, "0", end_label);
			emit(jump_inst);

			if (node->child_count > 3) {
				generate_ir(node->children[3]);
			}

			char* incr_result = generate_ir(node->children[2]);
			free(incr_result);

			IRInstruction* loop_jump = create_instruction(IR_JMP, loop_label, NULL, NULL);
			emit(loop_jump);

			IRInstruction* end_label_inst = create_instruction(IR_LABEL, end_label, NULL, NULL);
			emit(end_label_inst);

			free(loop_label);
			free(end_label);
			free(condition);

			return NULL;
		}

		case AST_RETURN: {
			if (node->child_count > 0) {
				char* return_value = generate_ir(node->children[0]);
				if (return_value) {
					free(return_value);
				}
			}

			return NULL;
		}

		case AST_ARRAY_LITERAL: {
			for (int i = 0; i < node->child_count; i++) {
				char* element = generate_ir(node->children[i]);
				if (element) {
					IRInstruction* param_inst = create_instruction(IR_PARAM, element, NULL, NULL);
					emit(param_inst);
					free(element);
				}
			}
			return NULL;
		}

		case AST_ARRAY_ACCESS: {
			if (node->child_count < 2) return NULL;
			char* array = generate_ir(node->children[0]);
			char* index = generate_ir(node->children[1]);
			if (!array || !index) {
				free(array);
				free(index);

				return NULL;
			}

			char* result = new_temporary();
			
			IRInstruction* access_inst = create_instruction(IR_ASSIGN, array, index, result);
			emit(access_inst);
			free(array);
			free(index);

			return result;
		}

		case AST_PROGRAM: {
			char* last_result = NULL;
			for (int i = 0; i < node->child_count; i++) {
				if (last_result) free(last_result);
				last_result = generate_ir(node->children[i]);
			}
			return last_result;
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
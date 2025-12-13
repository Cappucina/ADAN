#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ir.h"
#include "ast.h"

static IRInstruction* ir_head = NULL;
static IRInstruction* ir_tail = NULL;

static int temp_counter = 0;
static int string_counter = 0;
static StringLiteral* string_literals = NULL;
static char* current_function = NULL;

void init_ir() {
	ir_head = NULL;
	ir_tail = NULL;
	temp_counter = 0;
}

void init_ir_full() {
	ir_head = NULL;
	ir_tail = NULL;
	temp_counter = 0;
	string_counter = 0;
	string_literals = NULL;
	current_function = NULL;
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
			
			case IR_RETURN:
				printf("RETURN %s\n", current->arg1);
				break;
		}
		current = current->next;
	}
}

IRInstruction* get_ir_head() {
	return ir_head;
}

StringLiteral* get_string_literals() {
	return string_literals;
}

char* add_string_literal(const char* value) {
	char buffer[32];
	snprintf(buffer, 32, ".STR%d", string_counter);
	string_counter++;

	StringLiteral* lit = malloc(sizeof(StringLiteral));
	lit->label = strdup(buffer);
	lit->value = strdup(value);
	lit->next = string_literals;
	string_literals = lit;

	return strdup(buffer);
}

char* generate_ir(ASTNode* node) {
	if (node == NULL) return NULL;
	switch (node->type) {
		case AST_LITERAL: {
			if (node->token.type == TOKEN_STRING) {
				return add_string_literal(node->token.text);
			}
			if (node->token.type == TOKEN_TRUE) return strdup("1");
			if (node->token.type == TOKEN_FALSE) return strdup("0");
			if (node->token.text) return strdup(node->token.text);
			return NULL;
		}

		case AST_IDENTIFIER: {
			if (node->token.text) {
				// Scope variable to current function
				char buffer[128];
				if (current_function) {
					snprintf(buffer, sizeof(buffer), "%s.%s", current_function, node->token.text);
				} else {
					snprintf(buffer, sizeof(buffer), "%s", node->token.text);
				}
				return strdup(buffer);
			}
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

			// If either operand has been annotated as a string, generate
			// a runtime call to `concat` (and cast numeric args) instead
			// of a plain arithmetic add.
			Type left_type = node->children[0] ? node->children[0]->annotated_type : TYPE_UNKNOWN;
			Type right_type = node->children[1] ? node->children[1]->annotated_type : TYPE_UNKNOWN;

			if ((left_type == TYPE_STRING) || (right_type == TYPE_STRING)) {
				// Convert non-string operands with a cast() call.
				if (left_type != TYPE_STRING) {
					char* tmp_cast = new_temporary();
					IRInstruction* param = create_instruction(IR_PARAM, left, NULL, NULL);
					IRInstruction* call = create_instruction(IR_CALL, "cast", NULL, tmp_cast);
					emit(param);
					emit(call);
					free(left);
					left = tmp_cast;
				}

				if (right_type != TYPE_STRING) {
					char* tmp_cast = new_temporary();
					IRInstruction* param = create_instruction(IR_PARAM, right, NULL, NULL);
					IRInstruction* call = create_instruction(IR_CALL, "cast", NULL, tmp_cast);
					emit(param);
					emit(call);
					free(right);
					right = tmp_cast;
				}

				char* tmp_concat = new_temporary();
				IRInstruction* param1 = create_instruction(IR_PARAM, left, NULL, NULL);
				IRInstruction* param2 = create_instruction(IR_PARAM, right, NULL, NULL);
				IRInstruction* call_concat = create_instruction(IR_CALL, "concat", NULL, tmp_concat);
				emit(param1);
				emit(param2);
				emit(call_concat);

				free(left);
				free(right);
				return tmp_concat;
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

			// Scope variable to current function
			char dest_buffer[128];
			if (current_function) {
				snprintf(dest_buffer, sizeof(dest_buffer), "%s.%s", current_function, name);
			} else {
				snprintf(dest_buffer, sizeof(dest_buffer), "%s", name);
			}
			char* dest = strdup(dest_buffer);
			IRInstruction* new_instruction = create_instruction(IR_ASSIGN, val, NULL, dest);

			emit(new_instruction);

			free(val);
			free(dest);

			return strdup(dest_buffer);
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

				// Scope variable to current function
				char dest_buffer[128];
				if (current_function) {
					snprintf(dest_buffer, sizeof(dest_buffer), "%s.%s", current_function, name);
				} else {
					snprintf(dest_buffer, sizeof(dest_buffer), "%s", name);
				}
				char* dest = strdup(dest_buffer);
				IRInstruction* new_instruction = create_instruction(IR_ASSIGN, val, NULL, dest);

				emit(new_instruction);

				free(val);
				free(dest);

				return strdup(dest_buffer);
			}

			if (node->child_count >= 1 && node->children[0]->token.text) return strdup(node->children[0]->token.text);
			return NULL;
		}

		case AST_PROGRAM: {
			// Save function name for variable scoping
			// AST_PROGRAM structure: [return_type, name, params, body]
			const char* func_name = NULL;
			if (node->child_count > 1 && node->children[1]) {
				func_name = node->children[1]->token.text;
			}
		
			if (current_function) free(current_function);
			current_function = func_name ? strdup(func_name) : NULL;
		
			IRInstruction* label = create_instruction(IR_LABEL, (char*)func_name, NULL, NULL);
			emit(label);

			// Process function body (child[3])
			if (node->child_count > 3 && node->children[3]) {
				generate_ir(node->children[3]);
			}
			// Do not emit an implicit RETURN; rely on explicit AST_RETURN.

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

			if (node->token.type == TOKEN_MINUS) {
				char* temp = new_temporary();
				IRInstruction* new_instruction = create_instruction(IR_SUB, "0", operand, temp);
				emit(new_instruction);
				free(operand);
				return temp;
			}

			if (node->token.type == TOKEN_NOT) {
				char* result = new_temporary();
				char* l_true = new_temporary();
				char* l_end = new_temporary();

				IRInstruction* jump_true = create_instruction(IR_JEQ, operand, "0", l_true);
				emit(jump_true);

				IRInstruction* assign_false = create_instruction(IR_ASSIGN, "0", NULL, result);
				emit(assign_false);

				IRInstruction* jump_end = create_instruction(IR_JMP, l_end, NULL, NULL);
				emit(jump_end);

				IRInstruction* true_label = create_instruction(IR_LABEL, l_true, NULL, NULL);
				emit(true_label);

				IRInstruction* assign_true = create_instruction(IR_ASSIGN, "1", NULL, result);
				emit(assign_true);

				IRInstruction* end_label = create_instruction(IR_LABEL, l_end, NULL, NULL);
				emit(end_label);

				free(operand);
				free(l_true);
				free(l_end);
				return result;
			}

			free(operand);
			return NULL;
		}

		case AST_INCREMENT_EXPR: {
			if (node->child_count < 2) return NULL;
			char* var = generate_ir(node->children[0]);
			if (!var) return NULL;
			TokenType op = node->children[1]->token.type;
			char* temp = new_temporary();

			if (op == TOKEN_INCREMENT) {
				IRInstruction* add_inst = create_instruction(IR_ADD, var, "1", temp);
				emit(add_inst);
			} else {
				IRInstruction* sub_inst = create_instruction(IR_SUB, var, "1", temp);
				emit(sub_inst);
			}

			IRInstruction* assign_inst = create_instruction(IR_ASSIGN, temp, NULL, var);
			emit(assign_inst);

			free(temp);

			return var;
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
			char* l_true = new_temporary();
			char* l_end = new_temporary();

			IROp opcode;
			switch (node->token.type) {
				case TOKEN_EQUALS: opcode = IR_JEQ; break;
				case TOKEN_NOT_EQUALS: opcode = IR_JNE; break;
				case TOKEN_LESS: opcode = IR_LT; break;
				case TOKEN_GREATER: opcode = IR_GT; break;
				case TOKEN_LESS_EQUALS: opcode = IR_LTE; break;
				case TOKEN_GREATER_EQUALS: opcode = IR_GTE; break;
				default: opcode = IR_JEQ; break;
			}

			IRInstruction* jump_true = create_instruction(opcode, left, right, l_true);
			emit(jump_true);

			IRInstruction* assign_false = create_instruction(IR_ASSIGN, "0", NULL, result);
			emit(assign_false);

			IRInstruction* jump_end = create_instruction(IR_JMP, l_end, NULL, NULL);
			emit(jump_end);

			IRInstruction* true_label = create_instruction(IR_LABEL, l_true, NULL, NULL);
			emit(true_label);

			IRInstruction* assign_true = create_instruction(IR_ASSIGN, "1", NULL, result);
			emit(assign_true);

			IRInstruction* end_label = create_instruction(IR_LABEL, l_end, NULL, NULL);
			emit(end_label);

			free(left);
			free(right);
			free(l_true);
			free(l_end);

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
					IRInstruction* return_inst = create_instruction(IR_RETURN, return_value, NULL, NULL);
                    emit(return_inst);
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

	while (string_literals != NULL) {
		StringLiteral* temp = string_literals;
		string_literals = string_literals->next;
		
		free(temp->label);
		free(temp->value);
		free(temp);
	}
}
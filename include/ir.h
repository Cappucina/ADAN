#ifndef IR_H
#define IR_H

#include "ast.h"

// 
//  Opcodes for the IR language.
// 
typedef enum {
	IR_ADD,       // +
	IR_SUB,       // -
	IR_MUL,       // *
	IR_DIV,       // /
	IR_ASSIGN,    // =
	IR_LABEL,     // L1:
	IR_JMP,       // GOTO L1
	IR_JEQ,       // IF a == b GOTO L1
	IR_JNE,       // IF a != b GOTO L1
	IR_LT,        // IF a < b GOTO L1
	IR_GT,        // IF a > b GOTO L1
	IR_LTE,       // IF a <= b GOTO L1
	IR_GTE,       // IF a >= b GOTO L1
	IR_PARAM,     // Handle function params
	IR_CALL,      // Handle function calls
	IR_RETURN,    // Handle function returns
} IROp;

// 
//  Instructions; everything will fit inside this container.
// 
typedef struct IRInstruction {
	IROp op;                       // The verb (ADD, SUB, etc.)
	char* arg1;                    // Input 1 (variable or number)
	char* arg2;                    // Input 2 (can be NULL)
	char* result;                  // Where the answer goes (can be NULL)
	struct IRInstruction* next;    // Linked List pointer
	int index;
} IRInstruction;

typedef struct StringLiteral {
	char* label;
	char* value;
	struct StringLiteral* next;
} StringLiteral;

void init_ir();

void init_ir_full();

char* new_temporary();

IRInstruction* create_instruction(IROp op, char* arg1, char* arg2, char* result);

void emit(IRInstruction* instruction);

void print_ir();

char* generate_ir(ASTNode* node);

IRInstruction* get_ir_head();

StringLiteral* get_string_literals();

void free_ir();

#endif
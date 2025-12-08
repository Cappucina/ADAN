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
	IR_LABEL,	  // L1:
	IR_JMP,       // GOTO L1
	IR_JEQ,       // IF a == b GOTO L1
	IR_PRINT      // PRINT a
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
} IRInstruction;

void init_ir();

char* new_temporary();

IRInstruction* create_instruction(IROp op, char* arg1, char* arg2, char* result);

void emit(IRInstruction* instruction);

void print_ir();

char* generate_ir(ASTNode* node);

void free_ir();

#endif
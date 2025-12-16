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
	IR_MOD,       // %
	IR_POW,       // **
	IR_LABEL,     // L1:
	IR_JMP,       // GOTO L1
	IR_JEQ,       // IF a == b GOTO L1
	IR_JNE,       // IF a != b GOTO L1
	IR_LT,        // IF a < b GOTO L1
	IR_GT,        // IF a > b GOTO L1
	IR_LTE,       // IF a <= b GOTO L1
	IR_GTE,       // IF a >= b GOTO L1
	IR_AND,       // a && b (logical AND)
	IR_OR,        // a || b (logical OR)
	IR_PARAM,     // Handle function params
	IR_CALL,      // Handle function calls
	IR_RETURN,    // Handle function returns
	IR_ADDR_OF,   // &a
	IR_DEREF,     // *a
	IR_LOAD_IDX,  // a[i] load
	IR_STORE_IDX, // a[i] = x store
	IR_CONTINUE,  // Handle continue
	IR_NEG,       // -a (unary minus)
	IR_NOT,       // !a (logical NOT)
	IR_BIT_AND,   // a & b (bitwise AND)
	IR_BIT_OR,    // a | b (bitwise OR)
	IR_BIT_XOR,   // a ^ b (bitwise XOR)
	IR_BIT_ZERO_FILL_LEFT_SHIFT,
	IR_BIT_ZERO_FILL_RIGHT_SHIFT,
	IR_BIT_SIGNED_RIGHT_SHIFT,
	IR_SHL,       // a << b (left shift)
	IR_SHR,       // a >> b (right shift)
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

typedef struct GlobalVariable {
	char* label;       // Assembly label (e.g. G_name)
	char* name;        // original variable name
	char* initial;     // initial value (immediate or .STR label)
	int is_string;     // 1 if the initial value is a string pointer
	struct GlobalVariable* next;
} GlobalVariable;

void init_ir();

void init_ir_full();

char* new_temporary();

IRInstruction* create_instruction(IROp op, char* arg1, char* arg2, char* result);

void emit(IRInstruction* instruction);

void print_ir();

char* generate_ir(ASTNode* node);

IRInstruction* get_ir_head();

StringLiteral* get_string_literals();
GlobalVariable* get_global_variables();
char* add_global_variable(const char* name, const char* initial_value, int is_string);

void free_ir();

#endif
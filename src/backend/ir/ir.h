#ifndef BACKEND_IR_IR_H
#define BACKEND_IR_IR_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef enum
{
	IR_CONST,
	IR_ALLOCA,
	IR_LOAD,
	IR_STORE,
	IR_BINOP,
	IR_PHI,
	IR_CALL,
	IR_RET,
	IR_BR,
	IR_CBR,
	IR_NOP
} IrInstrKind;

typedef enum
{
	IR_T_VOID,
	IR_T_I64,
	IR_T_F64,
	IR_T_PTR,
} IRTypeKind;

typedef struct IRType
{
	IRTypeKind kind;
	struct IRType* pointee;  // for IR_T_PTR
} IRType;

typedef struct IRValue
{
	int kind;
	union
	{
		int temp_id;
		int64_t i64;
		double f64;
	} u;
	struct IRType* type;
} IRValue;

typedef enum
{
	IRV_TEMP,
	IRV_CONST,
	IRV_PARAM,
	IRV_GLOBAL
} IRValueKind;

typedef struct IRInstruction
{
	IrInstrKind kind;
	IRValue* dest;
	IRValue* operands[3];
	int opcode;
	struct IRInstruction* next;
} IRInstruction;

typedef struct IRBlock
{
	char* name;
	IRInstruction *first, *last;
	struct IRBlock* next;
} IRBlock;

typedef struct IRFunction
{
	char* name;
	IRType* return_type;
	IRBlock* blocks;
	struct IRFunction* next;
} IRFunction;

typedef struct IRModule
{
	IRFunction* functions;
} IRModule;

// Builder functions

IRModule* ir_module_create();

void ir_module_destroy(IRModule* m);

void ir_module_add_function(IRModule* m, IRFunction* f);

void ir_function_add_block(IRFunction* fn, IRBlock* b);

void ir_print_module(IRModule* m, FILE* out);

IRType* ir_type_i64(void);

IRType* ir_type_f64(void);

IRType* ir_type_void(void);

IRType* ir_type_ptr(IRType* pointee);

IRFunction* ir_function_create(const char* name, IRType* return_type);

IRBlock* ir_block_create(const char* name);

IRFunction* ir_function_create_in_module(IRModule* m, const char* name, IRType* return_type);

IRBlock* ir_block_create_in_function(IRFunction* f, const char* name);

IRValue* ir_param_create(IRFunction* f, const char* name, IRType* type);

IRValue* ir_global_create(IRModule* m, const char* name, IRType* type, IRValue* initial);

IRValue* ir_emit_alloca(IRBlock* b, IRType* type);

IRValue* ir_emit_load(IRBlock* b, IRValue* ptr);

void ir_emit_store(IRBlock* b, IRValue* ptr, IRValue* val);

IRValue* ir_emit_call(IRBlock* b, IRFunction* callee, IRValue** args, size_t nargs);

void ir_emit_br(IRBlock* b, IRBlock* target);

void ir_emit_cbr(IRBlock* b, IRValue* cond, IRBlock* true_b, IRBlock* false_b);

IRValue* ir_emit_phi(IRBlock* b, IRType* t, IRValue** values, IRBlock** blocks, size_t n);

int ir_validate_module(IRModule* m);

void ir_replace_value(IRModule* m, IRValue* oldv, IRValue* newv);

void ir_remove_instruction(IRBlock* b, IRInstruction* i);

void ir_instr_append(IRBlock* b, IRInstruction* i);

char* ir_strdup(IRModule* m, const char* s);

int ir_dump_module_to_file(IRModule* m, const char* path);

void ir_walk_module(IRModule* m, void (*fn)(IRFunction*, void*), void* user);

void ir_walk_function(IRFunction* f, void (*fn)(IRBlock*, void*), void* user);

IRValue* ir_emit_binop(IRBlock* block, const char* op, IRValue* lhs, IRValue* rhs);

void ir_emit_ret(IRBlock* block, IRValue* value);

IRValue* ir_const_i64(int64_t value);

IRValue* ir_const_string(IRModule* m, const char* str);

IRValue* ir_temp(IRBlock* block, IRType* type);

#endif

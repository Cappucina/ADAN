#ifndef BACKEND_LOWER_H
#define BACKEND_LOWER_H

#include "../frontend/ast/tree.h"
#include "../stm.h"
#include "ir/ir.h"

typedef struct Program
{
	ASTNode* ast_root;
	IRModule* ir;
} Program;

typedef struct SymEntry
{
	char* name;
	IRValue* value;
	int is_address;
	struct SymEntry* next;
} SymEntry;

void lower_program(Program* program);

#endif

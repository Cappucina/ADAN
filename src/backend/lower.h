#ifndef BACKEND_LOWER_H
#define BACKEND_LOWER_H

#include "../frontend/ast/tree.h"
#include "ir/ir.h"

typedef struct Program
{
    ASTNode *ast_root;
    IRModule *ir;
} Program;

void lower_program(Program *program);

#endif

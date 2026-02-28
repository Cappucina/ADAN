#ifndef BACKEND_BACKEND_H
#define BACKEND_BACKEND_H

#include "../frontend/ast/tree.h"

int backend_compile_ast_to_lltext(ASTNode* ast, FILE* out);

int backend_compile_ast_to_llvm_file(ASTNode* ast, const char* path);

#endif

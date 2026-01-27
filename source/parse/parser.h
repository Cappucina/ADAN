#pragma once

#include "../../include/ast.h"
#include "../../include/diagnostics.h"

ASTNode* parse(const char* source, ErrorList* errors, const char* file);
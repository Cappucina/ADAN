#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "../ast/tree.h"
#include "semantic.h"

void validate_node(SemanticAnalyzer* analyzer, ASTNode* node);

void validator_cleanup();

#endif
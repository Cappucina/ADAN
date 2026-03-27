#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "../ast/tree.h"
#include "semantic.h"

void validate_node(SemanticAnalyzer* analyzer, ASTNode* node);

void validator_cleanup();

const char* validator_get_embedded_modules();

const char* validator_get_bundle_paths();

#endif
#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "../ast/tree.h"
#include "semantic.h"

void validate_node(SemanticAnalyzer* analyzer, ASTNode* node);

void validator_cleanup();

// Returns a comma-separated list of embedded module import paths (e.g.
// "adan/io") that were resolved from the compiler's built-in library store
// during the last analysis pass, or NULL if none were used.
// The returned pointer is valid until the next validator_cleanup() call.
const char* validator_get_embedded_modules();

#endif
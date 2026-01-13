#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "diagnostic.h"
#include "common.h"

void semantic_analysis(Analyzer* analyzer);

Analyzer* create_semantic(Buffer* token_buffer, ErrorList* errors);
void semantic_analysis(Analyzer* analyzer);
void free_semantic(Analyzer* analyzer);

#endif


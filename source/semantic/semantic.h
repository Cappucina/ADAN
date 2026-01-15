#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "common.h"
#include "diagnostic.h"

Parser* create_semantic(Buffer* token_buffer, ErrorList* errors);

void semantic_analysis(Parser* parser);

void free_semantic(Parser* parser);

#endif

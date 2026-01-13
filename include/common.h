#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "diagnostic.h"
#include "lex/lexer.h"

typedef struct
{
    Token* tokens;
    size_t current;
    size_t count;
    ErrorList* errors;
    bool panic;
} Analyzer;

char* strdup(const char* s);

#endif

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    LEXER,
    PARSER,
    SEMANTIC,
    CODEGEN,
    GENERIC
} Category;

typedef enum
{
    INFO,
    WARNING,
    ERROR,
    CRITICAL
} Severity;

typedef struct
{
    const char* message;
    const char* file;
    size_t line;
    size_t column;
    uint32_t length;
    Severity severity;
    Category category;
} Error;

typedef struct
{
    Error* errors;
    uint32_t size;
    uint32_t capacity;
} ErrorList;

ErrorList* create_errors(void);

ErrorList* append_error(ErrorList* error_list, const char* file, const char* error_message, uint32_t line, uint32_t column, Severity severity, Category category);

void free_errors(ErrorList* error_list);

void set_warnings_as_errors(bool enabled);

void set_verbose_mode(bool enabled);  // Provides details on what's currently going on during compilation.

void set_silent_errors(bool enabled);  // Suppresses immediate error printing (useful for tests)

void warn(ErrorList* error_list, const char* file, uint32_t line, uint32_t column, Category category, const char* format, ...);

void error(ErrorList* error_list, const char* file, uint32_t line, uint32_t column, Category category, const char* format, ...);

void verbose(ErrorList* error_list, const char* file, uint32_t line, uint32_t column, Category category, const char* format, ...);

void print_diagnostics(const Error* error);

extern ErrorList* __global_error_list;

#endif
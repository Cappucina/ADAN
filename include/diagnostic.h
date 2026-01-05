#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
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
    size_t length;
    Severity severity;
    Category category;
} Error;

typedef struct
{
    Error* errors;
    size_t size;
    size_t capacity;
} ErrorList;

ErrorList* create_errors();

ErrorList* push_error(ErrorList* error_list, const char* file, const char* message, size_t line,
                      size_t column, Severity severity, Category category);

void free_errors(ErrorList* error_list);

void set_warnings_as_errors(bool enabled);

void set_suppress_warnings(bool enabled);

void set_verbose_mode(bool enabled);

void warn(ErrorList* error_list, const char* file, size_t line, size_t column, Category category,
          const char* format, ...);

void error(ErrorList* error_list, const char* file, size_t line, size_t column, Category category,
           const char* format, ...);

void verbose(ErrorList* error_list, const char* file, size_t line, size_t column, Category category,
             const char* format, ...);

void print_diagnostic(const Error* error);

extern ErrorList* g_error_list;

#endif

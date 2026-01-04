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

ErrorList* push_error(ErrorList* error_list, const char* message, size_t line, size_t column,
		      Severity severity, Category category);

void free_errors(ErrorList* error_list);
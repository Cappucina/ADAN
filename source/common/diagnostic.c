#include "diagnostic.h"

#include <stdarg.h>
#include <stdio.h>

#include "error.h"

static bool g_warnings_as_errors = false;
static bool g_suppress_warnings = false;
static bool g_verbose_mode = false;

ErrorList* g_error_list = NULL;

void set_warnings_as_errors(bool enabled)
{
    g_warnings_as_errors = enabled;
}

void set_suppress_warnings(bool enabled)
{
    g_suppress_warnings = enabled;
}

void set_verbose_mode(bool enabled)
{
    g_verbose_mode = enabled;
}

static const char* severity_to_string(Severity severity)
{
    switch (severity)
    {
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        case CRITICAL:
            return "CRITICAL";
        default:
            return "unknown error";
    }
}

static const char* get_severity_color(Severity severity)
{
    switch (severity)
    {
        case INFO:
            return "\033[0;36m";
        case WARNING:
            return "\033[0;33m";
        case ERROR:
            return "\033[0;31m";
        case CRITICAL:
            return "\033[0;35m";
        default:
            return "\033[0m";
    }
}

static const char* get_color_reset(void)
{
    return "\033[0m";
}

void print_diagnostic(const Error* error)
{
    if (!error)
    {
        return;
    }

    const char* color = get_severity_color(error->severity);
    const char* reset = get_color_reset();

    fprintf(stderr, "%s:%zu:%zu:%s%s%s: %s\n", error->file, error->line, error->column, color,
            severity_to_string(error->severity), reset, error->message);
}

ErrorList* create_errors(void)
{
    ErrorList* error_list = (ErrorList*)malloc(sizeof(ErrorList));
    if (!error_list)
    {
        return NULL;
    }

    error_list->size = 0;
    error_list->capacity = 10;
    error_list->errors = (Error*)malloc(sizeof(Error) * error_list->capacity);
    if (!error_list->errors)
    {
        free(error_list);
        return NULL;
    }

    return error_list;
}

ErrorList* push_error(ErrorList* error_list, const char* file, const char* message, size_t line,
                      size_t column, Severity severity, Category category)
{
    if (!error_list || !message)
    {
        return error_list;
    }

    if (error_list->size >= error_list->capacity)
    {
        size_t new_capacity = error_list->capacity * 2;
        Error* tmp = (Error*)realloc(error_list->errors, sizeof(Error) * new_capacity);
        if (!tmp)
        {
            return error_list;
        }

        error_list->errors = tmp;
        error_list->capacity = new_capacity;
    }

    Error error = {message, file, line, column, strlen(message), severity, category};
    error_list->errors[error_list->size] = error;
    error_list->size++;

    return error_list;
}

void free_errors(ErrorList* error_list)
{
    if (!error_list)
    {
        return;
    }

    free(error_list->errors);
    free(error_list);
}

void warn(ErrorList* error_list, const char* file, size_t line, size_t column, Category category,
          const char* format, ...)
{
    if (!error_list || !format)
    {
        return;
    }

    if (g_suppress_warnings && !g_warnings_as_errors)
    {
        return;
    }

    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Severity severity = g_warnings_as_errors ? ERROR : WARNING;
    push_error(error_list, file, buffer, line, column, severity, category);

    Error error = {buffer, file, line, column, strlen(buffer), severity, category};
    print_diagnostic(&error);
}

void error(ErrorList* error_list, const char* file, size_t line, size_t column, Category category,
           const char* format, ...)
{
    if (!error_list || !format)
    {
        return;
    }

    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    push_error(error_list, file, buffer, line, column, ERROR, category);

    Error error = {buffer, file, line, column, strlen(buffer), ERROR, category};
    print_diagnostic(&error);
}

void verbose(ErrorList* error_list, const char* file, size_t line, size_t column, Category category,
             const char* format, ...)
{
    if (!g_verbose_mode || !error_list || !format)
    {
        return;
    }

    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    push_error(error_list, buffer, file, line, column, INFO, category);

    Error error = {buffer, file, line, column, strlen(buffer), INFO, category};
    print_diagnostic(&error);
}

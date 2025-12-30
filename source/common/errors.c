#include "errors.h"

ErrorList* create_errors()
{
    ErrorList* error_list = (ErrorList*)malloc(sizeof(ErrorList));
    if (!error_list) {
        return NULL;
    }

    error_list->size = 0;
    error_list->errors = NULL;
    return error_list;
}

ErrorList* push_error(ErrorList* error_list, const char* message, size_t line, size_t column,
                      Severity severity, Category category)
{
    // Make more optimized and utilize `capacity`, don't realloc every single push
    Error error = {message, line, column, strlen(message), severity, category};
    Error* tmp = (Error*)realloc(error_list->errors, sizeof(Error) * (error_list->size + 1));
    if (!tmp) {
        return NULL;
    }

    tmp[error_list->size] = error;
    error_list->errors = tmp;
    error_list->size++;
    return error_list;
}

void free_errors(ErrorList* error_list)
{
    if (!error_list) {
        return;
    }

    free(error_list->errors);
    free(error_list);
}
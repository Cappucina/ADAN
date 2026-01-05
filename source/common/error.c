#include "error.h"

const char* error_to_string(ErrorCode code)
{
    switch (code)
    {
        case ERR_OK:
            return "No error";
        case ERR_MEMORY:
            return "Memory allocation failed";
        case ERR_INVALID:
            return "Invalid argument";
        case ERR_IO:
            return "I/O error";
        case ERR_PARSE:
            return "Parse error";
        case ERR_SEMANTIC:
            return "Semantic error";
        case ERR_CODEGEN:
            return "Code generation error";
        default:
            return "Unknown error";
    }
}

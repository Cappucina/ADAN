#ifndef ERROR_H
#define ERROR_H

typedef enum
{
    ERR_OK = 0,
    ERR_MEMORY = -12,
    ERR_INVALID = -22,
    ERR_IO = -5,
    ERR_PARSE = -74,
    ERR_SEMANTIC = -125,
    ERR_CODEGEN = -71,
} ErrorCode;

const char* error_to_string(ErrorCode code);

#endif

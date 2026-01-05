#ifndef FLAGS_H
#define FLAGS_H

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

typedef enum
{
    EXECUTABLE,
    OBJECT,
    ASM
} OutputType;

typedef struct
{
    Target target;
    bool help;
    bool verbose;
    bool warnings_as_errors;
    bool tests;
    uint8_t optimazation_level;
    OutputType compile_to;
    char* input;
    char* output;
    bool suppress_warnings;
    char** include;
    char* error;
} CompilerFlags;

// Function declarations
CompilerFlags* flags_init(int argc, char* argv[]);
void flags_free(CompilerFlags* flags);
void set_default_flags(CompilerFlags* flags);
const char* flags_get_error(void);

#endif

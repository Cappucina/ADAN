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
} CompilorFlags;

// Function declarations
CompilorFlags* flags_init(int argc, char* argv[]);
void flags_free(CompilorFlags* flags);
void set_default_flags(CompilorFlags* flags);
const char* flags_get_error(void);

#endif

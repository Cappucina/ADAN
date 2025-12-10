#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen_x86_64.h"

int x86_64_code_generation_handler(CompilerFlags* flags) {
    int res = 0;

    if (flags == NULL) {
        fprintf(stderr, "Error: Compiler flags are NULL\n");
        res = -1;
        goto out;
    }

    printf("Starting x86_64 code generation for input file: %s\n", flags->input_file);
    Token** tokens = convertToLogic(flags->input_file);

    if (tokens == NULL) {
        fprintf(stderr, "Error: Failed to convert input to logic representation\n");
        res = -1;
        goto out;
    }

    

    printf("Successfully generated x86_64 code to output file: %s\n", flags->output_file);
    return 0;

out:
    return res;
}
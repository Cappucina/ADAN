#include "compiler_handler.h"
#include "compiler_flags.h"
#include "codegen.h"

int compileHandler(CompilerFlags* flags) {
    int res = 0;

    if (flags == NULL) {
        fprintf(stderr, "Error: flags is NULL\n");
        return -1;
    }

    if (flags_validate(flags) != 0) {
        return -1;
    }

    switch (flags->target_arch) {
        case TARGET_ARM64:
            // ARM64 (Apple Silicon)
            res = arm64_code_generation_handler(flags);
            break;
        case TARGET_X86_64:
            res = x86_64_code_generation_handler(flags);
            break;
        case TARGET_WASM:
            fprintf(stderr, "Error: wasm codegen not implemented\n");
            res = -1;
            break;
        default:
            fprintf(stderr, "Error: Target architecture not specified or unsupported\n");
            res = -1;
            break;
    }

out:
    return res;
error_out:
    res = -1;
    goto out;
}
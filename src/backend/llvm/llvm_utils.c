#include <stdio.h>

#include "llvm_utils.h"

void llvm_utils_destroy_context(LLVMContext* ctx)
{
    if (!ctx)
    {
        fprintf(stderr, "Attempted to destroy a NULL LLVMContext. (Warning)\n");
        return;
    }
    fprintf(stderr, "LLVMContext destroyed successfully. (Info)\n");
    free(ctx);
}

// The purpose of this function is kinda to create a unique
// name so that we don't have to worry about overlapping with stuff later.
char* llvm_utils_mangle_name(const char* name)
{
    if (!name)
    {
        return NULL;
    }
    size_t len = strlen(name);
    char* mangled = (char*)malloc(len * 2 + 1); // Worst case: every char is '_'
    if (!mangled)
    {
        fprintf(stderr, "Failed to allocate memory for mangled name. (Error)\n");
        return NULL;
    }
    char* p = mangled;
    for (size_t i = 0; i < len; i++)
    {
        char c = name[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_')
        {
            *p++ = c;
        }
        else
        {
            *p++ = '_';
        }
    }
    *p = '\0';
    return mangled;
}

char* llvm_utils_unique_label(LLVMContext* ctx, const char* base)
{
    if (!ctx || !base)
    {
        return NULL;
    }
    size_t base_len = strlen(base);
    size_t label_len = base_len + 20;
    char* label = (char*)malloc(label_len);
    if (!label)
    {
        fprintf(stderr, "Failed to allocate memory for unique label. (Error)\n");
        return NULL;
    }
    snprintf(label, label_len, "%s_%lu", base, ctx->label_counter++);
    return label;
}
#include "semantic.h"

#include <stdlib.h>

void semantic_analysis(Analyzer* analyzer __attribute__((unused)))
{
    
}

Analyzer* create_semantic(Buffer* token_buffer, ErrorList* errors)
{
    Analyzer* analyzer = (Analyzer*)malloc(sizeof(Analyzer));
    analyzer->current = 0;
    analyzer->count = token_buffer->count;
    analyzer->tokens = token_buffer->data;
    analyzer->errors = errors;
    analyzer->panic = false;

    return analyzer;
}

void free_semantic(Analyzer* analyzer)
{
    if (analyzer) free(analyzer);
}

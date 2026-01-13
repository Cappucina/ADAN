#include "semantic.h"

#include <stdlib.h>

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

void semantic_analysis(Analyzer* analyzer)
{
    for (size_t i = 0; i < analyzer->count; i++)
    {
        switch (analyzer->tokens[i].type)
        {
            case TOKEN_EOF:
                break;
            case TOKEN_ERROR:
            case TOKEN_IDENTIFIER:
            case TOKEN_INT:
            case TOKEN_FLOAT:
            case TOKEN_STRING:
            case TOKEN_BOOL:
            case TOKEN_CHAR:
            case TOKEN_NULL:
            case TOKEN_VOID:
            case TOKEN_ADD:
            case TOKEN_SUBTRACT:
            case TOKEN_MULTIPLY:
            case TOKEN_DIVIDE:
            case TOKEN_MODULO:
            case TOKEN_CARET:
            case TOKEN_EXPONENT:
            case TOKEN_ADDRESS_OF:
            case TOKEN_REFERENCE:
            case TOKEN_INCREMENT:
            case TOKEN_DECREMENT:
            case TOKEN_BITWISE_AND:
            case TOKEN_BITWISE_OR:
            case TOKEN_BITWISE_NOT:
            case TOKEN_BITWISE_XOR:
            case TOKEN_BITWISE_NAND:
            case TOKEN_BITWISE_NOR:
            case TOKEN_BITWISE_XNOR:
            case TOKEN_BITWISE_ZERO_FILL_LEFT_SHIFT:
            case TOKEN_BITWISE_SIGNED_RIGHT_SHIFT:
            case TOKEN_BITWISE_ZERO_FILL_RIGHT_SHIFT:
            case TOKEN_LEFT_PAREN:
            case TOKEN_RIGHT_PAREN:
            case TOKEN_LEFT_BRACE:
            case TOKEN_RIGHT_BRACE:
            case TOKEN_LEFT_BRACKET:
            case TOKEN_RIGHT_BRACKET:
            case TOKEN_SEMICOLON:
            case TOKEN_COMMA:
            case TOKEN_PERIOD:
            case TOKEN_APOSTROPHE:
            case TOKEN_QUOTATION:
            case TOKEN_NOT:
            case TOKEN_AND:
            case TOKEN_TYPE_DECLARATOR:
            case TOKEN_EQUALS:
            case TOKEN_GREATER:
            case TOKEN_LESS:
            case TOKEN_GREATER_EQUALS:
            case TOKEN_LESS_EQUALS:
            case TOKEN_ASSIGN:
            case TOKEN_NOT_EQUALS:
            case TOKEN_OR:
            case TOKEN_ELLIPSIS:
            case TOKEN_TRUE_LITERAL:
            case TOKEN_INT_LITERAL:
            case TOKEN_FALSE_LITERAL:
            case TOKEN_FLOAT_LITERAL:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_FOR:
            case TOKEN_INCLUDE:
            case TOKEN_CONTINUE:
            case TOKEN_PROGRAM:
            case TOKEN_RETURN:
            case TOKEN_ELSE:
            case TOKEN_STRUCT:
            case TOKEN_BREAK:
                continue;
        }
    }
}

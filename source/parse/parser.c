#include "parser.h"

Parser* create_parser(Token* tokens, uint32_t count, ErrorList* errors)
{
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->current = 0;
    parser->count = count;
    parser->tokens = tokens;
    parser->errors = errors;
    parser->panic = false;

    return parser;
}

void free_parser()
{

}g
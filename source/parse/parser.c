#include "parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"
#include "diagnostic.h"

Token peek(Analyzer* parser)
{
    return parser->tokens[parser->current + 1];
}

Token advance(Analyzer* parser)
{
    return parser->tokens[++parser->current];
}

Token current_token(Analyzer* parser)
{
    return parser->tokens[parser->current];
}

// bool match(Analyzer* parser, uint32_t ct, ...)
// {
//     TokenType current = current_token(parser).type;

//     va_list args;
//     va_start(args, ct);
//     for (uint32_t i = 0; i < ct; i++)
//     {
//         if (current != va_arg(args, TokenType))
//         {
//             va_end(args);
//             return false;
//         }
//     }

//     va_end(args);
//     return true;
// }

bool match(Analyzer* parser, TokenType expected)
{
    return current_token(parser).type == expected;
}

Token* expect(Analyzer* parser, TokenType expected, const char* message)
{
    if (match(parser, expected))
    {
        advance(parser);
        return &parser->tokens[parser->current];
    }

    Error error;
    error.message = message;
    error.file = current_token(parser).file;
    error.line = current_token(parser).line;
    error.column = current_token(parser).column;
    if (parser->errors->size < parser->errors->capacity)
    {
        parser->errors->errors[parser->errors->size++] = error;
    }

    parser->panic = true;
    return NULL;
}

static void synchronize(Analyzer* parser)
{
    advance(parser);
    parser->panic = false;
    while (!match(parser, TOKEN_EOF))
    {
        if (current_token(parser).type == TOKEN_SEMICOLON)
        {
            advance(parser);
            return;
        }
        if (current_token(parser).type == TOKEN_IF ||
            current_token(parser).type == TOKEN_FOR ||
            current_token(parser).type == TOKEN_WHILE ||
            current_token(parser).type == TOKEN_RETURN ||
            current_token(parser).type == TOKEN_STRUCT ||
            current_token(parser).type == TOKEN_INCLUDE)
        {
            return;
        }
        advance(parser);
    }
}

Analyzer* create_parser(Buffer* token_buffer, ErrorList* errors)
{
    Analyzer* parser = (Analyzer*)malloc(sizeof(Analyzer));
    parser->current = 0;
    parser->count = token_buffer->count;
    parser->tokens = token_buffer->data;
    parser->errors = errors;
    parser->panic = false;

    return parser;
}

void parse(Analyzer* parser)
{
    for (size_t i = 0; i < parser->count; i++)
    {
        switch (parser->tokens[i].type)
        {
            case TOKEN_EOF:
                error(parser->errors, parser->tokens[i].file, parser->tokens[i].line, parser->tokens[i].column, LEXER,
                      "Unexpected end of file.");
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
            default:
                error(parser->errors, parser->tokens[i].file, parser->tokens[i].line, parser->tokens[i].column, LEXER,
                      "Unexpected token.");
                break;
        }
    }
}

void free_parser(Analyzer* parser)
{
    if (parser) free(parser);
}

// break;
static ASTNode* parse_break(Analyzer* parser)
{

}

// struct struct_name { ... }
static ASTNode* parse_struct(Analyzer* parser)
{

}

// return expression;
static ASTNode* parse_return(Analyzer* parser)
{

}

// program::type program_name(param_1, ...) { ... }
static ASTNode* parse_program(Parser* parser)
{

}

// continue;
static ASTNode* parse_continue(Analyzer* parser)
{

}

// include org.lib;
static ASTNode* parse_include(Analyzer* parser)
{

}

// for (init; condition; increment) { ... }
static ASTNode* parse_for(Analyzer* parser)
{

}

// while (condition) { ... }
static ASTNode* parse_while(Analyzer* parser)
{

}

// if (condition) { ... } else if (condition) { ... } else { ... }
static ASTNode* parse_if(Analyzer* parser)
{
}

static ASTNode* parse_expression(Analyzer* parser)
{
}

static ASTNode* parse_statement(Analyzer* parser)
{
}

static ASTNode* parse_expression_list(Alayzer* parser)
{
}

static ASTNode* parse_primary(Analyzer* parser)
{
}

static ASTNode* parse_binary(Analyzer* parser)
{
}

static ASTNode* parse_unary(Analyzer* parser)
{
}

static ASTNode* parse_declaration(Analyzer* parser)
{
}

static ASTNode* parse_type_specifier(Analyzer* parser)
{
}

static ASTNode* parse_assignment(Analyzer* parser)
{
}

static ASTNode* parse_or(Analyzer* parser)
{
}

static ASTNode* parse_and(Analyzer* parser)
{
}

static ASTNode* parse_equality(Analyzer* parser)
{
}

static ASTNode* parse_comparison(Analyzer* parser)
{
}

static ASTNode* parse_term(Analyzer* parser)
{
}

static ASTNode* parse_factor(Analyzer* parser)
{
}

void parse(Analyzer* parser)
{
    while (!match(parser, TOKEN_EOF))
    {
        parse_statement(parser);
    }
}

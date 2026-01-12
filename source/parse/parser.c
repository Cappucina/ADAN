#include "parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "buffer.h"
#include "diagnostic.h"

Token peek(Parser* parser)
{
    return parser->tokens[parser->current + 1];
}

Token advance(Parser* parser)
{
    return parser->tokens[++parser->current];
}

Token current_token(Parser* parser)
{
    return parser->tokens[parser->current];
}

bool match(Parser* parser, TokenType expected)
{
    return current_token(parser).type == expected;
}

Token* expect(Parser* parser, TokenType expected, const char* message)
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

static void synchronize(Parser* parser)
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

Parser* create_parser(Buffer* token_buffer, ErrorList* errors)
{
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->current = 0;
    parser->count = token_buffer->count;
    parser->tokens = token_buffer->data;
    parser->errors = errors;
    parser->panic = false;

    return parser;
}

void free_parser(Parser* parser)
{
    if (parser) free(parser);
}

// break;
static ASTNode* parse_break(Parser* parser)
{
}

// struct struct_name { ... }
static ASTNode* parse_struct(Parser* parser)
{
}

// return expression;
static ASTNode* parse_return(Parser* parser)
{
}

// program::type program_name(param_1, ...) { ... }
static ASTNode* parse_program(Parser* parser)
{
}

// continue;
static ASTNode* parse_continue(Parser* parser)
{
}

// include org.lib;
static ASTNode* parse_include(Parser* parser)
{
}

// for (init; condition; increment) { ... }
static ASTNode* parse_for(Parser* parser)
{
}

// while (condition) { ... }
static ASTNode* parse_while(Parser* parser)
{
}

// if (condition) { ... } else if (condition) { ... } else { ... }
static ASTNode* parse_if(Parser* parser)
{
}

static ASTNode* parse_expression(Parser* parser)
{
}

static ASTNode* parse_statement(Parser* parser)
{
}

static ASTNode* parse_expression_list(Parser* parser)
{
}

static ASTNode* parse_primary(Parser* parser)
{
}

static ASTNode* parse_binary(Parser* parser)
{
}

static ASTNode* parse_unary(Parser* parser)
{
}

static ASTNode* parse_declaration(Parser* parser)
{
}

static ASTNode* parse_type_specifier(Parser* parser)
{
}

static ASTNode* parse_assignment(Parser* parser)
{
}

static ASTNode* parse_or(Parser* parser)
{
}

static ASTNode* parse_and(Parser* parser)
{
}

static ASTNode* parse_equality(Parser* parser)
{
}

static ASTNode* parse_comparison(Parser* parser)
{
}

static ASTNode* parse_term(Parser* parser)
{
}

static ASTNode* parse_factor(Parser* parser)
{
}

void parse(Parser* parser)
{
    while (!match(parser, TOKEN_EOF))
    {
        parse_statement(parser);
    }
}
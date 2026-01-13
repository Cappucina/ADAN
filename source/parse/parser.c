#include "parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "ast.h"
#include "buffer.h"
#include "diagnostic.h"

Token peek(Analyzer* parser)
{
    if (parser->current + 1 >= parser->count)
    {
        return parser->tokens[parser->current];
    }
    return parser->tokens[parser->current + 1];
}

Token advance(Analyzer* parser)
{
    if (parser->current + 1 >= parser->count)
    {
        return parser->tokens[parser->current];
    }

    return parser->tokens[++parser->current];
}

Token current_token(Analyzer* parser)
{
    return parser->tokens[parser->current];
}

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

    Error error = {0};
    error.message = message;
    error.file = current_token(parser).file;
    error.line = current_token(parser).line;
    error.column = current_token(parser).column;
    error.length = 0;
    error.severity = ERROR;
    error.category = GENERIC;
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

void parse(Analyzer* analyzer)
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
                continue;
            case TOKEN_ADD:
            case TOKEN_SUBTRACT:
            case TOKEN_MULTIPLY:
            case TOKEN_DIVIDE:
            case TOKEN_MODULO:
            case TOKEN_CARET:
            case TOKEN_EXPONENT:
                continue;
            case TOKEN_ADDRESS_OF:
            case TOKEN_REFERENCE:
                continue;
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
                continue;
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

void free_parser(Analyzer* parser)
{
    if (parser) free(parser);
}

// break;
static ASTNode* parse_break(Analyzer* parser)
{
    ASTNode* node = create_ast_node(AST_BREAK);
    if (!node) return NULL;

    node->line = current_token(parser).line;
    node->column = current_token(parser).column;
    node->file_name = current_token(parser).file;

    advance(parser);
    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';' after 'break'"))
    {
        free_ast(node);
        return NULL;
    }

    return node;
}

// struct struct_name { ... }
static ASTNode* parse_struct(Analyzer* parser)
{
    Token struct_token = current_token(parser);
    advance(parser);
    if (!match(parser, TOKEN_IDENTIFIER))
    {
        expect(parser, TOKEN_IDENTIFIER, "Expected struct name after 'struct'");
        return NULL;
    }

    Token name_token = current_token(parser);
    advance(parser);
    if (!expect(parser, TOKEN_LEFT_BRACE, "Expected '{' after struct name"))
    {
        return NULL;
    }

    size_t capacity = 8;
    size_t count = 0;
    ASTNode** members = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
    if (!members) return NULL;
    while (!match(parser, TOKEN_RIGHT_BRACE) && !match(parser, TOKEN_EOF))
    {
        ASTNode* member = parse_declaration(parser);
        if (!member)
        {
            if (parser->panic) synchronize(parser);
            continue;
        }

        if (count >= capacity)
        {
            capacity *= 2;
            ASTNode** new_members = (ASTNode**)realloc(members, sizeof(ASTNode*) * capacity);
            if (!new_members)
            {
                for (size_t i = 0; i < count; i++) free_ast(members[i]);
                free(members);
                return NULL;
            }
            members = new_members;
        }

        members[count++] = member;
    }

    if (!expect(parser, TOKEN_RIGHT_BRACE, "Expected '}' after struct body"))
    {
        for (size_t i = 0; i < count; i++) free_ast(members[i]);
        free(members);
        return NULL;
    }

    ASTNode* node = create_struct_decl_node(name_token.start, members, count);
    if (!node)
    {
        for (size_t i = 0; i < count; i++) free_ast(members[i]);
        free(members);
        return NULL;
    }

    node->line = struct_token.line;
    node->column = struct_token.column;
    node->file_name = struct_token.file;

    return node;
}

// return expression;
static ASTNode* parse_return(Analyzer* parser)
{
    Token return_token = current_token(parser);
    advance(parser);
    if (match(parser, TOKEN_SEMICOLON))
    {
        ASTNode* node = create_ast_node(AST_RETURN);
        if (!node) return NULL;

        node->line = return_token.line;
        node->column = return_token.column;
        node->file_name = return_token.file;

        advance(parser);
        return node;
    }
    ASTNode* value = parse_expression(parser);
    if (value)
    {
        ASTNode* node = create_ast_node(AST_RETURN);
        if (!node)
        {
            free_ast(value);
            return NULL;
        }

        node->line = return_token.line;
        node->column = return_token.column;
        node->file_name = return_token.file;
        node->data.return_stmt.value = value;
        if (!expect(parser, TOKEN_SEMICOLON, "Expected ';' after return value"))
        {
            free_ast(node);
            return NULL;
        }

        return node;
    }
    return NULL;
}

// program::type program_name(param_1, ...) { ... }
static ASTNode* parse_program(Analyzer* parser)
{
    return NULL;
}

// continue;
static ASTNode* parse_continue(Analyzer* parser)
{
    return NULL;
}

// include org.lib;
static ASTNode* parse_include(Analyzer* parser)
{
    return NULL;
}

// for (init; condition; increment) { ... }
static ASTNode* parse_for(Analyzer* parser)
{
    return NULL;
}

// while (condition) { ... }
static ASTNode* parse_while(Analyzer* parser)
{
    return NULL;
}

// if (condition) { ... } else if (condition) { ... } else { ... }
static ASTNode* parse_if(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_expression(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_statement(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_expression_list(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_primary(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_binary(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_unary(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_declaration(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_type_specifier(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_assignment(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_or(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_and(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_equality(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_comparison(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_term(Analyzer* parser)
{
    return NULL;
}

static ASTNode* parse_factor(Analyzer* parser)
{
    return NULL;
}

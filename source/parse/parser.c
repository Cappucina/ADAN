#include "parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include "ast.h"
#include "buffer.h"
#include "diagnostic.h"

Token peek(Parser* parser)
{
    if (parser->current + 1 >= parser->count)
    {
        return parser->tokens[parser->current];
    }
    return parser->tokens[parser->current + 1];
}

Token advance(Parser* parser)
{
    if (parser->current + 1 >= parser->count)
    {
        return parser->tokens[parser->current];
    }

    return parser->tokens[++parser->current];
}

Token current_token(Parser* parser)
{
    return parser->tokens[parser->current];
}

bool is_at_end(Parser* parser) {
    return parser->tokens[parser->current].type == TOKEN_EOF;
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

// break;
static ASTNode* parse_break(Parser* parser)
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

static ASTNode* parse_declaration(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_statement(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_expression(Parser* parser)
{
    return NULL;
}

// struct struct_name { ... }
static ASTNode* parse_struct(Parser* parser)
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
    
    ASTNode* node = create_struct_decl_node(name_token.lexeme, members, count);
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
static ASTNode* parse_return(Parser* parser)
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

static ASTNode* parse_type(Parser* parser)
{
    return NULL;
}

// program::<type> program_name(param_one::<type>, param_two::<type>, ...);
static ASTNode* parse_program(Parser* parser)
{
    Token program_token = current_token(parser);
    advance(parser);
    if (!expect(parser, TOKEN_TYPE_DECLARATOR, "Expected '::' after 'program'")) {
        return NULL;
    }
    Token* program_type = expect(parser, TOKEN_IDENTIFIER, "Expected program type after '::'");
    if (!program_type) {
        synchronize(parser);
        return NULL;
    }
    Token* program_name = expect(parser, TOKEN_IDENTIFIER, "Expected program name after program type");
    if (!program_name) {
        synchronize(parser);
        return NULL;
    }
    ASTNode* node = create_ast_node(AST_PROGRAM);
    if (!node) {
        return NULL;
    }
    node->line = program_token.line;
    node->column = program_token.column;
    node->file_name = program_token.file;
    node->data.program_def.type = create_ast_node(AST_TYPE);
    if (!node->data.program_def.type) {
        free_ast(node);
        return NULL;
    }
    node->data.program_def.type->data.ident.name = strdup(program_type->lexeme);
    if (!node->data.program_def.type->data.ident.name) {
        free_ast(node);
        return NULL;
    }
    node->data.program_def.name = strdup(program_name->lexeme);
    if (!node->data.program_def.name) {
        free_ast(node);
        return NULL;
    }
    if (!expect(parser, TOKEN_LEFT_PAREN, "Expected '(' after program name")) {
        free_ast(node);
        return NULL;
    }
    // Written like: (param_one::<type>, param_two::<type>, ...)
    Buffer* arguments_buffer = buffer_create(sizeof(ASTNode*));
    if (!arguments_buffer) {
        free_ast(node);
        return NULL;
    }
    while (!match(parser, TOKEN_RIGHT_PAREN) && !match(parser, TOKEN_EOF)) {
        Token* param_name = expect(parser, TOKEN_IDENTIFIER, "Expected parameter name");
        if (!param_name) {
            buffer_free(arguments_buffer);
            free_ast(node);
            synchronize(parser);
            return NULL;
        }
        if (!expect(parser, TOKEN_TYPE_DECLARATOR, "Expected '::' after parameter name")) {
            buffer_free(arguments_buffer);
            free_ast(node);
            synchronize(parser);
            return NULL;
        }
        Token* param_type = expect(parser, TOKEN_IDENTIFIER, "Expected parameter type after '::'");
        if (!param_type) {
            buffer_free(arguments_buffer);
            free_ast(node);
            synchronize(parser);
            return NULL;
        }
        ASTNode* parameter = create_ast_node(AST_PARAM);
        if (!parameter) {
            buffer_free(arguments_buffer);
            free_ast(node);
            return NULL;
        }
        parameter->data.ident.name = strdup(param_name->lexeme);
        if (!parameter->data.ident.name) {
            free_ast(parameter);
            buffer_free(arguments_buffer);
            free_ast(node);
            return NULL;
        }
        parameter->data.ident.type = create_ast_node(AST_TYPE);
        if (!parameter->data.ident.type) {
            free_ast(parameter);
            buffer_free(arguments_buffer);
            free_ast(node);
            return NULL;
        }
        parameter->data.ident.type->data.ident.name = strdup(param_type->lexeme);
        if (!parameter->data.ident.type->data.ident.name) {
            free_ast(parameter);
            buffer_free(arguments_buffer);
            free_ast(node);
            return NULL;
        }
        parameter->line = param_name->line;
        parameter->column = param_name->column;
        parameter->file_name = param_name->file;
        buffer_push(arguments_buffer, &parameter);    
        if (match(parser, TOKEN_COMMA)) {
            advance(parser);
        } else {
            break;
        }
    }
    if (!expect(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters")) {
        buffer_free(arguments_buffer);
        free_ast(node);
        return NULL;
    }
    if (!expect(parser, TOKEN_LEFT_BRACE, "Expected opening brace, '{' after program declaration")) {
        buffer_free(arguments_buffer);
        free_ast(node);
        return NULL;
    }
    
    Buffer* body_buffer = buffer_create(sizeof(ASTNode*));
    if (!body_buffer) {
        buffer_free(arguments_buffer);
        free_ast(node);
        return NULL;
    }
    
    while (!match(parser, TOKEN_RIGHT_BRACE) && !match(parser, TOKEN_EOF)) {
        ASTNode* stmt = parse_statement(parser);
        if (!stmt) {
            if (parser->panic) synchronize(parser);
            continue;
        }
        buffer_push(body_buffer, &stmt);
    }
    
    if (!expect(parser, TOKEN_RIGHT_BRACE, "Expected closing brace, '}' after program body")) {
        buffer_free(arguments_buffer);
        buffer_free(body_buffer);
        free_ast(node);
        return NULL;
    }
    
    node->data.program_def.arguments = (ASTNode**)malloc(sizeof(ASTNode*) * arguments_buffer->count);
    if (!node->data.program_def.arguments) {
        buffer_free(arguments_buffer);
        buffer_free(body_buffer);
        free_ast(node);
        return NULL;
    }
    for (size_t i = 0; i < arguments_buffer->count; i++) {
        node->data.program_def.arguments[i] = *(ASTNode**)buffer_get(arguments_buffer, i);
    }
    node->data.program_def.count = arguments_buffer->count;
    
    node->data.program_def.body = create_block_node(
        (ASTNode**)body_buffer->data,
        body_buffer->count
    );
    if (!node->data.program_def.body) {
        buffer_free(arguments_buffer);
        buffer_free(body_buffer);
        free_ast(node);
        return NULL;
    }
    
    buffer_free(arguments_buffer);
    buffer_free(body_buffer);
    return node;
}

// continue;
static ASTNode* parse_continue(Parser* parser)
{
    ASTNode* node = create_ast_node(AST_CONTINUE);
    if (!node) return NULL;
    node->line = current_token(parser).line;
    node->column = current_token(parser).column;
    node->file_name = current_token(parser).file;
    return node;
}

static ASTNode* parse_include(Parser* parser)
{
    // <include> ::= "include" <identifier> {"." <identifier>} ";"
    if (!expect(parser, TOKEN_INCLUDE, "Expected include")) {
        return NULL;
    }
    ASTNode* node = create_ast_node(AST_INCLUDE);
    
    Token* org_node = expect(parser, TOKEN_IDENTIFIER, "Expected identifier after include");
    if (!org_node) {
        synchronize(parser);
        return NULL;
    }

    Buffer* libs_buffer = buffer_create(sizeof(char*));

    while (match(parser, TOKEN_PERIOD)) {
        advance(parser);
        
        Token* current_lib = expect(parser, TOKEN_IDENTIFIER, "Expected identifier after period");
        if (!current_lib) {
            synchronize(parser);
            buffer_free(libs_buffer);
            free_ast(node);
            return NULL;
        }
        buffer_push(libs_buffer, &current_lib->lexeme);
    }
    
    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';' after include statement")) {
        synchronize(parser);
        buffer_free(libs_buffer);
        free_ast(node);
        return NULL;
    }

    char** libs = (char**)malloc(sizeof(char*) * libs_buffer->count);
    if (!libs) {
        buffer_free(libs_buffer);
        free_ast(node);
        return NULL;
    }
    memcpy(libs, libs_buffer->data, sizeof(char*) * libs_buffer->count);

    const char* tmp_org = strdup(org_node->lexeme);
    if (!tmp_org) {
        free(libs);
        buffer_free(libs_buffer);
        free_ast(node);
        return NULL;
    }
    node->data.include.org = tmp_org;
    node->data.include.libs = libs;
    node->data.include.libs_count = libs_buffer->count;

    buffer_free(libs_buffer);
    return node;
}

// for (init; condition; increment) { ... }
static ASTNode* parse_for(Parser* parser)
{
    return NULL;
}

// while (condition) { ... }
static ASTNode* parse_while(Parser* parser)
{
    return NULL;
}

// if (condition) { ... } else if (condition) { ... } else { ... }
static ASTNode* parse_if(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_expression_list(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_primary(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_binary(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_unary(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_type_specifier(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_assignment(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_or(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_and(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_equality(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_comparison(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_term(Parser* parser)
{
    return NULL;
}

static ASTNode* parse_factor(Parser* parser)
{
    return NULL;
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

ASTNode* parse(Parser* parser)
{
    // <program> ::= {<include>} {<top_decl>}
    Token current = current_token(parser);
    ASTNode* root_node = create_ast_node(AST_ROOT);
    if (!root_node) {
        return NULL;
    }

    Buffer* includes = buffer_create(sizeof(ASTNode*));
    while (match(parser, TOKEN_INCLUDE) && !is_at_end(parser)) {
        ASTNode* tmp = parse_include(parser);
        if (!tmp) {
            synchronize(parser);
            continue;
        }
        buffer_push(includes, tmp);
    }

    Buffer* top_decls = buffer_create(sizeof(ASTNode*));
    while (!is_at_end(parser)) {
        ASTNode* tmp = parse_declaration(parser);
        if (!tmp) {
            synchronize(parser);
            continue;
        }
        buffer_push(top_decls, tmp);
    }

    ASTNode** includes_ast = NULL;
    if (includes->count > 0) {
        includes_ast = (ASTNode**)malloc(sizeof(ASTNode*) * includes->count);
        if (!includes_ast) {
            buffer_free(includes);
            buffer_free(top_decls);
            return NULL;
        }
        memcpy(includes_ast, includes->data, includes->count * sizeof(ASTNode*));
    }
    ASTNode** top_decls_ast = NULL;
    if (top_decls->count > 0) {
        top_decls_ast = (ASTNode**)malloc(sizeof(ASTNode*) * top_decls->count);
        if (!top_decls_ast) {
            free(includes_ast);
            buffer_free(includes);
            buffer_free(top_decls);
            return NULL;
        }
        memcpy(top_decls_ast, top_decls->data, top_decls->count * sizeof(ASTNode*));
    }

    root_node->data.root.includes = includes_ast;
    root_node->data.root.includes_count = includes->count;
    root_node->data.root.decls = top_decls_ast;
    root_node->data.root.decls_count = top_decls->count;

    root_node->line = current.line;
    root_node->column = current.column;
    buffer_free(includes);
    buffer_free(top_decls);
    return root_node;
}

void free_parser(Parser* parser)
{
    if (parser) free(parser);
}
#include <stdio.h>
#include <stdarg.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->error = false;
    parser->error_message = NULL;

    Token* current = next_token(lexer);
    Token* peek = next_token(lexer);

    if (!current || !peek)
    {
        if (current) free_token(current);
        if (peek) free_token(peek);
        set_error(parser, "Failed to get initial tokens");
        return;
    }

    parser->current_token = *current;
    parser->peek_token = *peek;

    free(current);
    free(peek);
}

void free_parser(Parser* parser) {
    if (parser->error_message) {
        free(parser->error_message);
        parser->error_message = NULL;
    }
}

bool match(Parser* parser, TokenType type) {
    TokenType current_type = parser->current_token.type;
    if (current_type == type) {
        parser->current_token = parser->peek_token;
        parser->peek_token = *next_token(parser->lexer);
        return true;
    }
    return false;
}

bool expect(Parser* parser, TokenType type, const char* error_msg) {
    TokenType current_type = parser->current_token.type;
    if (current_type == type) {
        parser->current_token = parser->peek_token;
        parser->peek_token = *next_token(parser->lexer);
        return true;
    }

    set_error(parser, error_msg);
    return false;
}

bool peek_is(Parser* parser, TokenType type) {
    TokenType current_type = parser->peek_token.type;
    if (current_type == type) {
        return true;
    }
    return false;
}

void set_error(Parser* parser, const char* fmt, ...) {
    if (parser->error) return;

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);

    int f = vsnprintf(NULL, 0, fmt, args);
    char* formatted = malloc(f + 1);
    if (!formatted) return;
    vsnprintf(formatted, f + 1, fmt, args_copy);

    va_end(args);
    va_end(args_copy);

    parser->error_message = formatted;

    parser->error = true;
}


ASTNode* create_ast_node(ASTNodeType type, Token token) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (node == NULL) {
        return NULL;
    }
    node->token = token;
    node->type = type;
    node->child_count = 0;
    node->children = NULL;
    return node;
}

ASTNode* parse_statement(Parser* parser) { // WiP
    Token current_token = parser->current_token;

    switch(parser->current_token.type)
    {
        case TOKEN_IDENTIFIER:
            switch(parser->peek_token.type) {
                case TOKEN_TYPE_DECL:
                    return parse_assignment(parser);
                    break;
                case TOKEN_LPAREN:
                    return parse_function_call(parser);
                    break;
                default:
                    return parse_expression(parser);
                    break;
            }
            break;

        case TOKEN_IF:
            return parse_if_statement(parser);
            break;
        case TOKEN_WHILE:
            return parse_while_statement(parser);
            break;
        case TOKEN_FOR:
            return parse_for_statement(parser);
            break;
        
        case TOKEN_PROGRAM:
            return parse_program(parser);
            break;
        case TOKEN_INCLUDE:
            return parse_include_statement(parser);
            break;
        
        case TOKEN_INT_LITERAL:
        case TOKEN_FLOAT_LITERAL:
        case TOKEN_STRING:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_NULL:
        case TOKEN_LPAREN:
            return parse_expression(parser);
            break;

        default:
            set_error(parser, "Unexpected token '%s'", current_token.text);
            return NULL;
    }
}

// program::void my_function() { }
ASTNode* parse_program(Parser* parser) {
    ASTNode* parse_node = create_ast_node(AST_PROGRAM, parser->current_token);
    if (!match(parser, TOKEN_PROGRAM)) {
        set_error(parser, "Expected keyword 'program', got '%s'", parser->current_token.text);
        return NULL;
    }

    // TOKEN_TYPE_DECL = ::
    if (!match(parser, TOKEN_TYPE_DECL)) {
        set_error(parser, "Expected '::' after program, got '%s'", parser->current_token.text);
        return NULL;
    }

    ASTNode* type = create_ast_node(AST_TYPE, parser->current_token);
    if (!match(parser, TOKEN_INT) && !match(parser, TOKEN_VOID) &&
        !match(parser, TOKEN_CHAR) && !match(parser, TOKEN_FLOAT) &&
        !match(parser, TOKEN_BOOLEAN) && !match(parser, TOKEN_STRING)) {
            set_error(parser, "Expected a type after program, got '%s'", parser->current_token.text);
            return;
    }

    ASTNode* identifier = parse_identifier(parser);
    if (!identifier) {
        set_error(parser, "Expected identifier, got '%s'", parser->current_token.text);
        return NULL;
    }

    ASTNode* params = parse_params(parser);
    if (!params) {
        set_error(parser, "Expected params, got '%s'", parser->current_token.text);
        return NULL;
    }

    ASTNode* block = parse_block(parser);
    if (!block) {
        set_error(parser, "Expected block, got '%s'", parser->current_token.text);
        return NULL;
    }

    parse_node->child_count = 4;
    parse_node->children = malloc(sizeof(ASTNode*) * parse_node->child_count);
    parse_node->children[0] = type;
    parse_node->children[1] = identifier;
    parse_node->children[2] = params;
    parse_node->children[3] = block;


    return parse_node;
}

ASTNode* parse_params(Parser* parser) {
    ASTNode* node = create_ast_node(AST_PARAMS, parser->current_token);
    if (!node) {
        set_error(parser, "Expected keyword 'program', got '%s'", parser->current_token.text);
        return NULL;
    }
    
    if (!match(parser, TOKEN_LPAREN)) {
        set_error(parser, "Expected '(', got '%s'", parser->current_token.text);
        return NULL;
    }
    int count = 0;
    Parser clone = *parser;
    while (clone.current_token.type != TOKEN_RPAREN) {
        ASTNode* id = parse_assignment(&clone);
        count++;
    }

    ASTNode** assignments = malloc(sizeof(ASTNode*) * count);
    for (int i = 0; i < count; i++) {
        assignments[i] = parse_assignment(parser);
    }
    if (!match(parser, TOKEN_RPAREN)) {
        set_error(parser, "Expected ')', got '%s'", parser->current_token.text);
        return NULL;
    }

    node->child_count = count;
    node->children = assignments;

    return node;
}

ASTNode* parse_assignment(Parser* parser) {

}

ASTNode* parse_expression(Parser* parser) {

}

ASTNode* parse_binary(Parser* parser) {

}

ASTNode* parse_unary(Parser* parser) {

}

ASTNode* parse_primary(Parser* parser) {

}

ASTNode* parse_if_statement(Parser* parser) {
    ASTNode* if_node = create_ast_node(AST_IF, parser->current_token);
    if (!match(parser, TOKEN_IF)) {
        set_error(parser, "Expected keyword 'if', got '%s'", parser->current_token.text);
        return NULL;
    }

    ASTNode* left = parse_expression(parser);

    ASTNode* operator = create_ast_node(AST_OPERATORS, parser->current_token);
    if (!match(parser, TOKEN_EQUALS) && !match(parser, TOKEN_GREATER) &&
        !match(parser, TOKEN_LESS) && !match(parser, TOKEN_GREATER_EQUALS) &&
        !match(parser, TOKEN_LESS_EQUALS) &&
        !match(parser, TOKEN_NOT_EQUALS) && !match(parser, TOKEN_AND)) {
            set_error(parser, "Expected comparison operator, got '%s'", parser->current_token.text);
            return;
    }

    ASTNode* right = parse_expression(parser);

    ASTNode* condition = create_ast_node(AST_COMPARISON, operator->token);

    condition->child_count = 2;
    condition->children = malloc(sizeof(ASTNode*) * condition->child_count);
    condition->children[0] = left;
    condition->children[1] = right;

    ASTNode* block = parse_block(parser);

    if_node->child_count = 2;
    if_node->children = malloc(sizeof(ASTNode*) * if_node->child_count);
    if_node->children[0] = condition;
    if_node->children[1] = block;

    return if_node;
}

ASTNode* parse_while_statement(Parser* parser) {

}

ASTNode* parse_for_statement(Parser* parser) {

}

ASTNode* parse_function_call(Parser* parser) {

}

ASTNode* parse_include_statement(Parser* parser) {

}

ASTNode* parse_block(Parser* parser) {
    ASTNode* node = create_ast_node(AST_BLOCK, parser->current_token);
    if (!node) {
        set_error(parser, "Expected keyword 'program', got '%s'", parser->current_token.text);
        return NULL;
    }

    match(parser, TOKEN_LBRACE);

    int count = 0;
    Parser clone = *parser;
    while(clone.current_token.type != TOKEN_RBRACE 
            && clone.current_token.type != TOKEN_EOF) {
        parse_statement(&clone);
        count++;
    }

    ASTNode** children = malloc(sizeof(ASTNode*) * count);
    if (!children) {
        set_error(parser, "Failed to allocate memory for child nodes");
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        children[i] = parse_statement(parser);
    }

    node->children = children;
    node->child_count = count;

    match(parser, TOKEN_RBRACE);
    return node;
}

ASTNode* parse_identifier(Parser* parser) {

}

ASTNode* parse_literal(Parser* parser) {

}

ASTNode* parse_statement_list(Parser* parser) {

}

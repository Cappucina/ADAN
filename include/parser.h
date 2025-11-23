#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;

    Token current_token;   // Store tokens by value, not pointer
    Token peek_token;

    bool error;
    char *error_message;
} Parser;

void init_parser(Parser *parser, Lexer *lexer);

void free_parser(Parser *parser);

bool match(Parser *parser, TokenType type);

bool expect(Parser *parser, TokenType type, const char *error_msg);

bool peek_is(Parser *parser, TokenType type);

void set_error(Parser *parser, const char *fmt, ...);

static inline bool has_error(Parser *parser) {
    return parser->error;
}

ASTNode* create_ast_node(ASTNodeType type, Token token);

static inline void add_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return;
    ASTNode **new_children = (ASTNode**) realloc(parent->children, sizeof(ASTNode*) * (parent->child_count + 1));

    if (!new_children) return;
    parent->children = new_children;
    parent->children[parent->child_count] = child;
    parent->child_count++;
}

ASTNode* parse_program(Parser *parser);

ASTNode* parse_statement(Parser *parser);

ASTNode* parse_assignment(Parser *parser);

ASTNode* parse_expression(Parser *parser);

ASTNode* parse_binary(Parser *parser);

ASTNode* parse_unary(Parser *parser);

ASTNode* parse_primary(Parser *parser);

ASTNode* parse_if_statement(Parser *parser);

ASTNode* parse_while_statement(Parser *parser);

ASTNode* parse_function_call(Parser *parser);

ASTNode* parse_include_statement(Parser *parser);

ASTNode* parse_block(Parser *parser);

ASTNode* parse_identifier(Parser *parser);

ASTNode* parse_literal(Parser *parser);

ASTNode* parse_statement_list(Parser *parser);

#endif

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "ast.h"
#include "diagnostic.h"
#include "lexer.h"

typedef struct Parser
{
    Lexer* lexer;
    ErrorList* errors;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

Parser* create_parser(Lexer* lexer, ErrorList* errors);

Parser* create_parser_from_source(const char* source, ErrorList* errors);

void free_parser(Parser* parser);

/**
 *
 * Navigational helpers.
 */
bool parser_at_end(Parser* parser);

Token parser_peek(Parser* parser);

Token parser_peek_next(Parser* parser);

Token parser_next(Parser* parser);

bool parser_check(Parser* parser, TokenType type);

bool parser_match(Parser* parser, TokenType type);

Token parser_consume(Parser* parser, TokenType type, const char* error_message);

/**
 *
 * Expression parsing helpers. (Precedence climbing)
 */
ASTNode* parse_expression(Parser* parser);

ASTNode* parse_assignment(Parser* parser);

ASTNode* parse_logical_or(Parser* parser);

ASTNode* parse_logical_and(Parser* parser);

ASTNode* parse_bitwise_or(Parser* parser);

ASTNode* parse_bitwise_xor(Parser* parser);

ASTNode* parse_bitwise_and(Parser* parser);

ASTNode* parse_equality(Parser* parser);

ASTNode* parse_comparison(Parser* parser);

ASTNode* parse_shift(Parser* parser);

ASTNode* parse_term(Parser* parser);

ASTNode* parse_factor(Parser* parser);

ASTNode* parse_unary(Parser* parser);

ASTNode* parse_postfix(Parser* parser);

ASTNode* parse_call(Parser* parser);

ASTNode* parse_primary(Parser* parser);

/**
 *
 * Statement and declaration helpers.
 *
 * The difference between parameters and arguments is that
 * parameters are used in function declarations, while
 * arguments are used in function calls.
 */
ASTNode* parse_program(Parser* parser);

ASTNode* parse_parameter_list(Parser* parser);

ASTNode* parse_declaration(Parser* parser);

ASTNode* parse_variable_declaration(Parser* parser);

ASTNode* parse_argument_list(Parser* parser);

ASTNode* parse_struct_declaration(Parser* parser);

ASTNode* parse_include_directive(Parser* parser);

ASTNode* parse_statement(Parser* parser);

ASTNode* parse_block(Parser* parser);

ASTNode* parse_if_statement(Parser* parser);

ASTNode* parse_while_statement(Parser* parser);

ASTNode* parse_for_statement(Parser* parser);

ASTNode* parse_return_statement(Parser* parser);

ASTNode* parse_break_statement(Parser* parser);

ASTNode* parse_continue_statement(Parser* parser);

/**
 *
 * Type helpers.
 */
ASTNode* parse_type(Parser* parser);

ASTNode* parse_type_annotation(Parser* parser);

#endif
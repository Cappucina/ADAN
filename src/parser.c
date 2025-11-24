#include "lexer.h"
#include "ast.h"
#include "parser.h"

void init_parser(Parser *parser, Lexer *lexer) {
    parser->lexer = lexer;
    parser->error = false;
    parser->error_message = NULL;
    parser->current_token = *next_token(lexer);
    parser->peek_token = *next_token(lexer);
}

void free_parser(Parser *parser) {
    if (parser->error_message) parser->error_message = NULL;
}

// 
//  Helper functions, designed to make parsing a whole lot simpler and
//   easier to manage.
// 

// 
//  Match two tokens
// 
bool match(Parser *parser, TokenType type) {

}

bool expect(Parser *parser, TokenType type, const char *error_msg) {

}

bool peek_is(Parser *parser, TokenType type) {

}

void set_error(Parser *parser, const char *fmt, ...) {

}

// 
//  General parsing functions (For Statements, Literals, etc.)
//
ASTNode* parse_program(Parser *parser) {

}

ASTNode* parse_statement(Parser *parser) {

}

ASTNode* parse_assignment(Parser *parser) {

}

ASTNode* parse_expression(Parser *parser) {

}

ASTNode* parse_binary_op(Parser *parser) {

}

ASTNode* parse_if_statement(Parser *parser) {

}

ASTNode* parse_while_statement(Parser *parser) {

}

ASTNode* parse_function_call(Parser *parser) {

}

ASTNode* parse_include_statement(Parser *parser) {

}

ASTNode* parse_block(Parser *parser) {

}

ASTNode* parse_identifier(Parser *parser) {

}

ASTNode* parse_literal(Parser *parser) {

}

ASTNode* parse_statement_list(Parser *parser) {

}
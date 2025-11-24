#include <stdarg.h>
#include <stdio.h>
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
//  Matches the current token with the expected type. If it matches,
//   advances to the next token and returns true. Otherwise, returns false.
// 
bool match(Parser *parser, TokenType type) {
    if (parser->current_token.type == type) {
        parser->current_token = parser->peek_token;
        Token *next = next_token(parser->lexer);
        if (next) {
            parser->peek_token = *next;
            free_token(next);
        } else {
            parser->peek_token.type = TOKEN_EOF;
            parser->peek_token.text = "";
            parser->peek_token.line = parser->lexer->line;
        }
        return true;
    } else {
        return false;
    }
}

//
//  Expects the current token to be of a specific type. If it is, advances
//   to the next token and returns true. If not, sets an error with the
//   provided error message and returns false.
//
bool expect(Parser *parser, TokenType type, const char *error_msg) {
    if (parser->current_token.type == type) {
        parser->current_token = parser->peek_token;
        Token *next = next_token(parser->lexer);
        if (next) {
            parser->peek_token = *next;
            free_token(next);
        } else {
            parser->peek_token.type = TOKEN_EOF;
            parser->peek_token.text = "";
            parser->peek_token.line = parser->lexer->line;
        }
        return true;
    } else {
        set_error(parser, "%s", error_msg);
        return false;
    }
}

//
//  Peeks at the next token to see if it matches the expected type.
//   Does not advance the parser's current token.
//
bool peek_is(Parser *parser, TokenType type) {
    return parser->peek_token.type == type;
}

void set_error(Parser *parser, const char *fmt, ...) {
    if (parser->error) return;
    parser->error = true;
    va_list args;
    va_start(args, fmt);
    size_t size = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);
    parser->error_message = malloc(size);
    va_start(args, fmt);
    vsnprintf(parser->error_message, size, fmt, args);
    va_end(args);
}

// 
//  General parsing functions (For Statements, Literals, etc.)
//
ASTNode* parse_statement(Parser *parser) {
    ASTNode *node = NULL;
    if (parser->current_token.type == TOKEN_IF) {
        node = parse_if_statement(parser);
    } else if (parser->current_token.type == TOKEN_WHILE) {
        node = parse_while_statement(parser);
    } else if (parser->current_token.type == TOKEN_IDENTIFIER) {
        node = parse_assignment(parser);
    } else if (parser->current_token.type == TOKEN_INCLUDE) {
        node = parse_include_statement(parser);
    } else if (parser->current_token.type == TOKEN_PROGRAM) {
        node = parse_program(parser);
    } else {
        set_error(parser, "Unexpected token '%s' on line %d", parser->current_token.text, parser->current_token.line);
    }
    return node;
}

ASTNode* parse_program(Parser *parser) {
    ASTNode *program_node = create_ast_node(AST_PROGRAM, parser->current_token);
    while (parser->current_token.type != TOKEN_EOF && !has_error(parser)) {
        ASTNode *statement = parse_statement(parser);
        if (statement) {
            add_child(program_node, statement);
        } else {
            break;
        }
    }
    return program_node;
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
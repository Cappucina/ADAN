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
    ASTNode *assignment_node = create_ast_node(AST_ASSIGNMENT, parser->current_token);
    ASTNode *identifier_node = parse_identifier(parser);
    if (!identifier_node) {
        set_error(parser, "Expected identifier in assignment on line %d", parser->current_token.line);
        free_ast(assignment_node);
        return NULL;
    }
    
    add_child(assignment_node, identifier_node);
    if (!expect(parser, TOKEN_ASSIGN, "Expected '=' in assignment")) {
        free_ast(assignment_node);
        return NULL;
    }

    // 
    //  Handle expression operations on the right side
    //   for things like: a = 5 + 3; or a = "hello";
    // 
    ASTNode *expression_node = parse_expression(parser);
    if (!expression_node) {
        set_error(parser, "Expected expression in assignment on line %d", parser->current_token.line);
        free_ast(assignment_node);
        return NULL;
    }

    add_child(assignment_node, expression_node);
    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';' at end of assignment")) {
        free_ast(assignment_node);
        return NULL;
    }
    return assignment_node;
}

// 
//  Parse expressions, handling binary operations and literals such as
//   integers, floats, string literals, booleans, etc.
// 
ASTNode* parse_expression(Parser *parser) {
    
}

ASTNode* parse_binary_op(Parser *parser) {
    ASTNode *binary_node = create_ast_node(AST_BINARY_OP, parser->current_token);
    ASTNode *left = parse_primary(parser);
    if (!left) {
        set_error(parser, "Expected left operand in binary operation on line %d", parser->current_token.line);
        free_ast(binary_node);
        return NULL;
    }
    add_child(binary_node, left);

    Token operator_token = parser->current_token;
    if (!match(parser, operator_token.type)) {
        set_error(parser, "Expected operator in binary operation on line %d", parser->current_token.line);
        free_ast(binary_node);
        return NULL;
    }

    ASTNode *right = parse_primary(parser);
    if (!right) {
        set_error(parser, "Expected right operand in binary operation on line %d", parser->current_token.line);
        free_ast(binary_node);
        return NULL;
    }

    add_child(binary_node, right);
    return binary_node;
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
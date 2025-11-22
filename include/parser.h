#include "lexer.h"
#include "ast.h"

#ifndef PARSER_H
#define PARSER_H

//
//  Turn a list of tokens into a proper Abstract Syntax Tree. (AST)
// 
/*

                                +--------------------+
                                |    AST_PROGRAM     |
                                +----------+---------+
                                           |
                                           v
                                +--------------------+
                                |   AST_STATEMENT*   |
                                +----------+---------+
                                           |
                --------------------------------------------------------------------------
                |                |                     |                      |          |
                v                v                     v                      v          v

        +----------------+   +----------------+   +----------------+   +------------------+ +-------------+
        | AST_ASSIGNMENT |   |    AST_IF      |   |   AST_WHILE    |   | AST_FUNCTION_CALL| | AST_INCLUDE |
        +-------+--------+   +--------+-------+   +--------+-------+   +---------+--------+ +------+------+
                |                     |                    |                     |                 |
                |                     |                    |                     |                 |
                v                     v                    v                     v                 v

        +----------------+   +----------------+   +----------------+   +----------------+   +-------------+
        | AST_IDENTIFIER |   | AST_EXPRESSION |   | AST_EXPRESSION |   | AST_IDENTIFIER |   | AST_BREAK   |
        +----------------+   +--------+-------+   +--------+-------+   +----------------+   +-------------+
                                      |                    |
                                      v                    v

                            +----------------+   +----------------+
                            | AST_BLOCK      |   |  AST_BLOCK     |
                            +-------+--------+   +--------+-------+
                                    |                     |
                                    v                     v
                            +----------------+   +----------------+
                            | AST_STATEMENT* |   | AST_STATEMENT* |
                            +----------------+   +----------------+

                (Expressions)
                ---------------------------------------------
                |                   |                      |
                v                   v                      v
        +---------------+   +--------------- +     +----------------+
        | AST_LITERAL   |   | AST_IDENTIFIER |     | AST_BINARY_OP  |
        +---------------+   +----------------+     +--------+-------+
                                                            |   
                                                            v
                                            +--------------------------------+
                                            |  (left/right expression nodes) |
                                            +--------------------------------+

*/

ASTNode* parse_program(Lexer *lexer);

ASTNode* parse_statement(Lexer *lexer);

ASTNode* parse_assignment(Lexer *lexer);

ASTNode* parse_expression(Lexer *lexer);

ASTNode* parse_binary_op(Lexer *lexer);

ASTNode* parse_if_statement(Lexer *lexer);

ASTNode* parse_while_statement(Lexer *lexer);

ASTNode* parse_function_call(Lexer *lexer);

ASTNode* parse_include_statement(Lexer *lexer);

ASTNode* parse_block(Lexer *lexer);

ASTNode* parse_identifier(Lexer *lexer);

ASTNode* parse_literal(Lexer *lexer);

#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"

void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->error = false;
    parser->error_message = NULL;

    parser->current_token.text = NULL;
    parser->peek_token.text = NULL;

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
    if (current->text) {
       parser->current_token.text = strdup(current->text);
    } else {
        parser->current_token.text = NULL;
    }

    parser->peek_token = *peek;
    if (peek->text) {
        parser->peek_token.text = strdup(peek->text);
    } else {
        parser->peek_token.text = NULL;
    }

    free_token(current);
    free_token(peek);
}

void free_parser(Parser* parser) {
    if (parser->error_message) {
        free(parser->error_message);
        parser->error_message = NULL;
    }

    if (parser->current_token.text) {
        free(parser->current_token.text);
        parser->current_token.text = NULL;
    }
    if (parser->peek_token.text) {
        free(parser->peek_token.text);
        parser->peek_token.text = NULL;
    }
}

bool match(Parser* parser, TokenType type) {
    if (parser->current_token.type == type) {
        if (parser->current_token.text) {
            free(parser->current_token.text);
            parser->current_token.text = NULL;
        }
        parser->current_token = parser->peek_token;
        parser->peek_token.text = NULL;
        
        Token* t = next_token(parser->lexer);
        if (t == NULL) {
            parser->peek_token.type = TOKEN_EOF;
            parser->peek_token.text = NULL;
        } else {
            parser->peek_token = *t;
            if (t->text) {
                parser->peek_token.text = strdup(t->text);
            } else {
                parser->peek_token.text = NULL;
            }
            free_token(t);
        }
        return true;
    }
    return false;
}

bool expect(Parser* parser, TokenType type, const char* error_msg, ...) {
    va_list args;
    va_start(args, error_msg);
    if (parser->current_token.type == type) {
        if (parser->current_token.text) {
            free(parser->current_token.text);
            parser->current_token.text = NULL;
        }
        parser->current_token = parser->peek_token;
        parser->peek_token.text = NULL;

        Token* t = next_token(parser->lexer);
        if (t == NULL) {
            parser->peek_token.type = TOKEN_EOF;
            parser->peek_token.text = NULL;
            va_end(args);
            return false;
        }
        parser->peek_token = *t;
        if (t->text) {
            parser->peek_token.text = strdup(t->text);
            t->text = NULL;
        }
        free_token(t);
        va_end(args);
        return true;
    }

    va_list args_copy;
    va_copy(args_copy, args);
    int f = vsnprintf(NULL, 0, error_msg, args);
    char* formatted = malloc(f + 1);
    if (formatted == NULL) {
        parser->error_message = NULL;
        parser->error = true;
        va_end(args_copy);
        va_end(args);
        return false;
    }
    vsnprintf(formatted, f + 1, error_msg, args_copy);
    va_end(args_copy);
    va_end(args);

    parser->error_message = formatted;
    parser->error = true;
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
    
    free(parser->error_message);
    
    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);

    int f = vsnprintf(NULL, 0, fmt, args);
    char* formatted = malloc(f + 1);
    if (formatted == NULL) {
        parser->error_message = NULL;
        parser->error = true;
        va_end(args_copy);
        va_end(args);
        return;
    }
    vsnprintf(formatted, f + 1, fmt, args_copy);

    va_end(args_copy);
    va_end(args);

    parser->error_message = formatted;

    parser->error = true;
}

ASTNode* create_ast_node(ASTNodeType type, Token token) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (node == NULL) {
        return NULL;
    }
    node->type = type;
    node->token.type = token.type;
    node->token.line = token.line;
    if (token.text) {
        node->token.text = strdup(token.text);
    } else {
        node->token.text = NULL;
    }
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


// 
//  program::void my_function() { }
// 
ASTNode* parse_program(Parser* parser) {
    ASTNode* parse_node = create_ast_node(AST_PROGRAM, parser->current_token);

    if (!expect(parser, TOKEN_PROGRAM, "Expected keyword 'program', got '%s'", parser->current_token.text)) return NULL;
    if (!expect(parser, TOKEN_TYPE_DECL, "Expected '::' after program, got '%s'", parser->current_token.text)) return NULL;

    ASTNode* type = create_ast_node(AST_TYPE, parser->current_token);
    if (!match(parser, TOKEN_INT) && !match(parser, TOKEN_VOID) &&
        !match(parser, TOKEN_CHAR) && !match(parser, TOKEN_FLOAT) &&
        !match(parser, TOKEN_BOOLEAN) && !match(parser, TOKEN_STRING)) {
            set_error(parser, "Expected a type after program, got '%s'", parser->current_token.text);
            return NULL;
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
    if (!expect(parser, TOKEN_LPAREN, "Expected '(', got '%s'", parser->current_token.text)) return NULL;

    ASTNode** params = NULL;
    int count = 0;

    if (parser->current_token.type != TOKEN_RPAREN) {
        while (1) {
            Token id = parser->current_token;
            if (!expect(parser, TOKEN_IDENTIFIER, "Expected parameter name, got '%s'", parser->current_token.text)) {
                return NULL;
            }
            if (!expect(parser, TOKEN_TYPE_DECL, "Expected '::' after parameter name, got '%s'", parser->current_token.text)) {
                return NULL;
            }

            Token type = parser->current_token;
            if (!match(parser, TOKEN_INT) && !match(parser, TOKEN_FLOAT) &&
                !match(parser, TOKEN_STRING) && !match(parser, TOKEN_BOOLEAN) &&
                !match(parser, TOKEN_CHAR) && !match(parser, TOKEN_NULL) &&
                !match(parser, TOKEN_VOID)) {
                set_error(parser, "Expected type declaration for parameter, got '%s'", parser->current_token.text);
                return NULL;
            }

            ASTNode* id_node = create_ast_node(AST_IDENTIFIER, id);
            ASTNode* param_node = create_ast_node(AST_TYPE, type);
            if (!id_node || !param_node) {
                set_error(parser, "Failed to create AST nodes for parameter");
                return NULL;
            }

            ASTNode* assign = create_ast_node(AST_ASSIGNMENT, id);
            assign->child_count = 2;
            assign->children = malloc(sizeof(ASTNode*) * 2);
            assign->children[0] = id_node;
            assign->children[1] = param_node;

            ASTNode** tmp = realloc(params, sizeof(ASTNode*) * (count + 1));
            if (!tmp) {
                set_error(parser, "Failed to allocate memory for parameters");
                return NULL;
            }

            params = tmp;
            params[count] = assign;
            count++;
            
            if (parser->current_token.type == TOKEN_COMMA) {
                if (!match(parser, TOKEN_COMMA)) {
                    set_error(parser, "Expected ',', got '%s'", parser->current_token.text);
                    return NULL;
                }
                if (parser->current_token.type == TOKEN_RPAREN) {
                    set_error(parser, "Trailing comma in parameter list");
                    return NULL;
                }
                continue;
            }
            break;
        }
    }
    if (!expect(parser, TOKEN_RPAREN, "Expected ')', got '%s'", parser->current_token.text)) return NULL;

    node->child_count = count;
    node->children = params;
    return node;
}

ASTNode* parse_assignment(Parser* parser) {
    Token identifier_token = parser->current_token;
    if (identifier_token.text) {
        identifier_token.text = strdup(identifier_token.text);
    }

    if (!expect(parser, TOKEN_IDENTIFIER, "Expected identifier, got '%s'", parser->current_token.text)) {
        if (identifier_token.text) free(identifier_token.text);
        return NULL;
    }
    if (!expect(parser, TOKEN_TYPE_DECL, "Expected '::', got '%s'", parser->current_token.text)) {
        if (identifier_token.text) free(identifier_token.text);
        return NULL;
    }

    Token type_token = parser->current_token;
    if (type_token.text) {
        type_token.text = strdup(type_token.text);
    }
    
    if (!match(parser, TOKEN_INT) && !match(parser, TOKEN_FLOAT) &&
        !match(parser, TOKEN_STRING) && !match(parser, TOKEN_BOOLEAN) &&
        !match(parser, TOKEN_CHAR) && !match(parser, TOKEN_NULL) &&
        !match(parser, TOKEN_VOID)) {
            if (identifier_token.text) free(identifier_token.text);
            if (type_token.text) free(type_token.text);
            set_error(parser, "Expected type declaration, got '%s'", parser->current_token.text);
            return NULL;
    }

    ASTNode* identifier = create_ast_node(AST_IDENTIFIER, identifier_token);
    ASTNode* type = create_ast_node(AST_TYPE, type_token);

    if (!identifier || !type) 
    {
        set_error(parser, "Failed to create AST nodes for assignment");
        return NULL;
    }

    ASTNode* assignment_node = create_ast_node(AST_ASSIGNMENT, identifier_token);

    if (parser->current_token.type == TOKEN_ASSIGN) {
        match(parser, TOKEN_ASSIGN);

        ASTNode* expression = parse_expression(parser);
        if (!expression) {
            set_error(parser, "Expected expression, got '%s'", parser->current_token.text);
            free_ast(identifier);
            free_ast(type);
            free_ast(assignment_node);
            return NULL;
        }

        if (!expect(parser, TOKEN_SEMICOLON, "Expected ';', got '%s'", parser->current_token.text)) {
            free_ast(identifier);
            free_ast(type);
            free_ast(expression);
            free_ast(assignment_node);
            return NULL;
        }

        assignment_node->child_count = 3;
        assignment_node->children = malloc(sizeof(ASTNode*) * 3);
        assignment_node->children[0] = identifier;
        assignment_node->children[1] = type;
        assignment_node->children[2] = expression;
    } else {
        if (!expect(parser, TOKEN_SEMICOLON, "Expected ';', got '%s'", parser->current_token.text)) {
            free_ast(identifier);
            free_ast(type);
            free_ast(assignment_node);
            return NULL;
        }

        assignment_node->child_count = 2;
        assignment_node->children = malloc(sizeof(ASTNode*) * 2);
        assignment_node->children[0] = identifier;
        assignment_node->children[1] = type;
    }

    return assignment_node;
}

ASTNode* parse_expression(Parser* parser) {
    ASTNode* expression_node = NULL;

    // 
    //  Literals (Strings, Integers, Booleans, etc.)
    // 
    TokenType type = parser->current_token.type;
    if (type == TOKEN_INT_LITERAL || type == TOKEN_FLOAT_LITERAL ||
        type == TOKEN_STRING || type == TOKEN_CHAR ||
        type == TOKEN_TRUE || type == TOKEN_FALSE ||
        type == TOKEN_NULL) {
            Token tok = parser->current_token;
            if (tok.text) tok.text = strdup(tok.text);
            match(parser, type);
            expression_node = create_ast_node(AST_LITERAL, tok);
            return expression_node;
    }
    
    // 
    //  Identifiers (such as variable names)
    // 
    if (type == TOKEN_IDENTIFIER) {
        return parse_identifier(parser);
    }

    // 
    //  Unary operands
    // 
    if (parser->current_token.type == TOKEN_MINUS || parser->current_token.type == TOKEN_PLUS ||
        parser->current_token.type == TOKEN_NOT || parser->current_token.type == TOKEN_INCREMENT ||
        parser->current_token.type == TOKEN_DECREMENT) {
            Token unary_token = parser->current_token;
            if (unary_token.text) unary_token.text = strdup(unary_token.text);
            match(parser, unary_token.type);
            ASTNode* operand = parse_expression(parser);
            if (!operand) {
                if (unary_token.text) free(unary_token.text);
                return NULL;
            }

            expression_node = create_ast_node(AST_UNARY_EXPR, unary_token);
            expression_node->child_count = 1;
            expression_node->children = malloc(sizeof(ASTNode*));
            expression_node->children[0] = operand;

            return expression_node;
    }

    if (parser->current_token.type == TOKEN_LPAREN) {
        match(parser, TOKEN_LPAREN);
        expression_node = parse_expression(parser);

        if (!expression_node) return NULL;
        if (!match(parser, TOKEN_RPAREN)) {
            set_error(parser, "Expected ')', got '%s'", parser->current_token.text);
            return NULL;
        }

        return expression_node;
    }

    set_error(parser, "Expected expression starter, got '%s'", parser->current_token.text);
    return NULL;
}

int get_node_precedence(ASTNode* node) {
    if (!node) return -1;

    switch (node->token.type) {
        case TOKEN_CAROT: // ^
            return 4;

        case TOKEN_ASTERISK: // *
        case TOKEN_SLASH: // /
        case TOKEN_PERCENT: // %
            return 3;
        
        case TOKEN_PLUS: // +
        case TOKEN_MINUS: // -
            return 2;
        
        default:
            return -1;
    }
}

// 
//  Supported operators:
//   *, +, -, /, ^, %
// 
ASTNode* parse_binary(Parser* parser) {
    ASTNode* left = parse_primary(parser);
    
    if (!left) return NULL;
    while (1) {
        TokenType op_type = parser->current_token.type;

        int precedence;
        int right_assoc = 0;

        switch (op_type) {
            case TOKEN_CAROT:
                precedence = 4;
                right_assoc = 1;
                break;
            
            case TOKEN_ASTERISK:
            case TOKEN_SLASH:
            case TOKEN_PERCENT:
                precedence = 3;
                break;
            
            case TOKEN_PLUS:
            case TOKEN_MINUS:
                precedence = 2;
                break;
            
            default:
                return left;
        }

        if (!right_assoc && precedence < get_node_precedence(left)) break;
        if (right_assoc && precedence <= get_node_precedence(left)) break;

        Token op_token = parser->current_token;
        match(parser, op_type);

        ASTNode* right = parse_primary(parser);
        if (!right) {
            set_error(parser, "Expected expression after '%s'", op_token.text);
            return left;
        }

        ASTNode* new_node = create_ast_node(AST_BINARY_OP, op_token);
        
        new_node->child_count = 2;
        new_node->children = malloc(sizeof(ASTNode*) * 2);
        new_node->children[0] = left;
        new_node->children[1] = right;

        left = new_node;
    }

    return left;
}

// 
//  Supported operators:
//   -, !
// 
ASTNode* parse_unary(Parser* parser) {
    TokenType op_type = parser->current_token.type;

    if (op_type == TOKEN_MINUS || op_type == TOKEN_NOT) {
        Token op_token = parser->current_token;
        match(parser, op_type);

        ASTNode* operand = parse_unary(parser);
        if (!operand) {
            set_error(parser, "Expected expression after unary operator '%s'", op_token.text);
            return NULL;
        }

        ASTNode* node = create_ast_node(AST_UNARY_OP, op_token);
        
        node->child_count = 1;
        node->children = malloc(sizeof(ASTNode*));
        node->children[0] = operand;

        return node;
    }

    return parse_primary(parser);
}

ASTNode* parse_if_statement(Parser* parser) {
    ASTNode* if_node = create_ast_node(AST_IF, parser->current_token);

    if (!expect(parser, TOKEN_IF, "Expected keyword 'if', got '%s'", parser->current_token.text)) return NULL;
    if (!expect(parser, TOKEN_LPAREN, "Expected '(', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* left = parse_statement(parser);
    if (!left) {
        set_error(parser, "Expected expression in if condition, got '%s'", parser->current_token.text);
        return NULL;
    }

    ASTNode* condition = NULL;

    if (parser->current_token.type == TOKEN_RPAREN) {
        condition = left;
    } else {
        Token op = parser->current_token;
        TokenType t = op.type;

        if (t != TOKEN_EQUALS && t != TOKEN_GREATER &&
            t != TOKEN_LESS && t != TOKEN_GREATER_EQUALS &&
            t != TOKEN_LESS_EQUALS && t != TOKEN_NOT_EQUALS &&
            t != TOKEN_AND) {
                set_error(parser, "Expected comparison operator, got '%s'", parser->current_token.text);
                return NULL;
        }
        match(parser, t);
        

        ASTNode* right = parse_statement(parser);
        if (!right) {
            set_error(parser, "Expected expression in if condition, got '%s'", parser->current_token.text);
            return NULL;
        }

        condition = create_ast_node(AST_COMPARISON, op);
        condition->child_count = 2;
        condition->children = malloc(sizeof(ASTNode*) * condition->child_count);
        condition->children[0] = left;
        condition->children[1] = right;
    }

    if (!expect(parser, TOKEN_RPAREN, "Expected ')', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* block = parse_block(parser);

    if_node->child_count = 2;
    if_node->children = malloc(sizeof(ASTNode*) * if_node->child_count);
    if_node->children[0] = condition;
    if_node->children[1] = block;

    return if_node;
}

// 
//  while (condition) { }
// 
ASTNode* parse_while_statement(Parser* parser) {
    ASTNode* while_node = create_ast_node(AST_WHILE, parser->current_token);

    if (!expect(parser, TOKEN_WHILE, "Expected keyword 'while', got '%s'", parser->current_token.text)) return NULL;
    if (!expect(parser, TOKEN_LPAREN, "Expected '(', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* left = parse_statement(parser);
    if (!left) {
        set_error(parser, "Expected expression in while condition, got '%s'", parser->current_token.text);
        return NULL;
    }

    ASTNode* condition = NULL;

    if (parser->current_token.type == TOKEN_RPAREN) {
        condition = left;
    } else {
        ASTNode* operator = create_ast_node(AST_OPERATORS, parser->current_token);
        Token op = parser->current_token;
        TokenType t = op.type;
        if (t != TOKEN_EQUALS && t != TOKEN_GREATER &&
            t != TOKEN_LESS && t != TOKEN_GREATER_EQUALS &&
            t != TOKEN_LESS_EQUALS && t != TOKEN_NOT_EQUALS &&
            t != TOKEN_AND) {
                set_error(parser, "Expected comparison operator, got '%s'", parser->current_token.text);
                return NULL;
        }

        ASTNode* right = parse_statement(parser);
        if (!right) {
            set_error(parser, "Expected expression in while condition, got '%s'", parser->current_token.text);
            return NULL;
        }

        condition = create_ast_node(AST_COMPARISON, operator->token);
        condition->child_count = 2;
        condition->children = malloc(sizeof(ASTNode*) * condition->child_count);
        condition->children[0] = left;
        condition->children[1] = operator;
        condition->children[2] = right;
    }

    if (!expect(parser, TOKEN_RPAREN, "Expected ')', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* block = parse_block(parser);

    while_node->child_count = 2;
    while_node->children = malloc(sizeof(ASTNode*) * while_node->child_count);
    while_node->children[0] = condition;
    while_node->children[1] = block;

    return while_node;
}

// 
//  for (var; condition; increment) { }
// 
ASTNode* parse_for_statement(Parser* parser) {
    ASTNode* for_node = create_ast_node(AST_FOR, parser->current_token);

    if (!expect(parser, TOKEN_FOR, "Expected keyword 'for', got '%s'", parser->current_token.text)) return NULL;
    if (!expect(parser, TOKEN_LPAREN, "Expected '(', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* variable = create_ast_node(AST_ASSIGNMENT, parser->current_token);

    if (!expect(parser, TOKEN_IDENTIFIER, "Expected identifier, got '%s'", parser->current_token.text)) return NULL;
    if (!expect(parser, TOKEN_TYPE_DECL, "Expected '::', got '%s'", parser->current_token.text)) return NULL;
    if (!expect(parser, TOKEN_ASSIGN, "Expected '=', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* value = parse_statement(parser);

    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* left = parse_statement(parser);

    ASTNode* operator = create_ast_node(AST_OPERATORS, parser->current_token);
    if (!match(parser, TOKEN_EQUALS) && !match(parser, TOKEN_GREATER) &&
        !match(parser, TOKEN_LESS) && !match(parser, TOKEN_GREATER_EQUALS) &&
        !match(parser, TOKEN_LESS_EQUALS) &&
        !match(parser, TOKEN_NOT_EQUALS) && !match(parser, TOKEN_AND)) {
            set_error(parser, "Expected comparison operator, got '%s'", parser->current_token.text);
            return NULL;
    }

    ASTNode* right = parse_statement(parser);

    ASTNode* condition = create_ast_node(AST_COMPARISON, operator->token);

    condition->child_count = 2;
    condition->children = malloc(sizeof(ASTNode*) * condition->child_count);
    condition->children[0] = left;
    condition->children[1] = right;

    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* variable_to_modify = create_ast_node(AST_IDENTIFIER, parser->current_token);

    if (!expect(parser, TOKEN_IDENTIFIER, "Expected identifier, got '%s'", parser->current_token.text)) return NULL;

    Token operator_token;;
    if (match(parser, TOKEN_INCREMENT)) operator_token = parser->current_token;
    else if (match(parser, TOKEN_DECREMENT)) operator_token = parser->current_token;
    else if (match(parser, TOKEN_MUL_MUL)) operator_token = parser->current_token;
    else if (match(parser, TOKEN_DIV_DIV)) operator_token = parser->current_token;
    else {
        set_error(parser, "Expected increment/decrement operator, got '%s'", parser->current_token.text);
        return NULL;
    }

    ASTNode* increment_op = create_ast_node(AST_OPERATORS, operator_token);
    ASTNode* increment_node = create_ast_node(AST_INCREMENT_EXPR, parser->current_token);
    
    increment_node->child_count = 2;
    increment_node->children = malloc(sizeof(ASTNode*) * 2);
    increment_node->children[0] = variable_to_modify;
    increment_node->children[1] = increment_op;

    ASTNode* block = parse_block(parser);

    for_node->child_count = 4;
    for_node->children = malloc(sizeof(ASTNode*) * for_node->child_count);
    for_node->children[0] = variable;
    for_node->children[1] = condition;
    for_node->children[2] = variable_to_modify;
    for_node->children[3] = block;

    return for_node;
}

ASTNode* parse_function_call(Parser* parser) {
    ASTNode* node = create_ast_node(AST_FUNCTION_CALL, parser->current_token);
    
    ASTNode* identifier = parse_primary(parser);
    ASTNode* params = parse_primary(parser);

    expect(parser, TOKEN_SEMICOLON, "Expected ';', got '%s'", parser->current_token.text);
    
    node->child_count = 2; // Identifier - Params
    node->children = malloc(sizeof(ASTNode*) * node->child_count);
    node->children[0] = identifier;
    node->children[1] = params;
    
    return node;
}

ASTNode* parse_include_statement(Parser* parser) {
    ASTNode* include_node = create_ast_node(AST_INCLUDE, parser->current_token);

    if (!expect(parser, TOKEN_INCLUDE, "Expected keyword 'include', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* publisher = parse_identifier(parser); // `include **adan**.io`, the publisher part being `adan`.

    if (!expect(parser, TOKEN_PERIOD, "Expected '.', got '%s'", parser->current_token.text)) return NULL;

    ASTNode* package = parse_identifier(parser); // `include adan.**io**`, the package part being `io`.

    if (!expect(parser, TOKEN_SEMICOLON, "Expected ';', got '%s'", parser->current_token.text)) return NULL;

    include_node->child_count = 2;
    include_node->children = malloc(sizeof(ASTNode*) * include_node->child_count);
    include_node->children[0] = publisher;
    include_node->children[1] = package;

    return include_node;
}

ASTNode* parse_block(Parser* parser) {
    ASTNode* node = create_ast_node(AST_BLOCK, parser->current_token);
    if (!node) {
        set_error(parser, "Expected keyword 'program', got '%s'", parser->current_token.text);
        return NULL;
    }

    if (!expect(parser, TOKEN_LBRACE, "Expected '{', got '%s'", parser->current_token.text)) return NULL;

    int count = 0;
    ASTNode** children = NULL;
    while(parser->current_token.type != TOKEN_RBRACE 
            && parser->current_token.type != TOKEN_EOF) {
        ASTNode* st = parse_statement(parser);
        if (!st) {
            set_error(parser, "Failed to parse statement in block");
            return NULL;
        }

        ASTNode** tmp = realloc(children, sizeof(ASTNode*) * (count + 1));
        if (!tmp) {
            set_error(parser, "Failed to allocate memory for child nodes");
            return NULL;
        }
        children = tmp;
        children[count] = st;
        count++;
    }

    node->children = children;
    node->child_count = count;

    if (!expect(parser, TOKEN_RBRACE, "Expected '}', got '%s'", parser->current_token.text)) return NULL;

    return node;
}

ASTNode* parse_identifier(Parser* parser) {
    Token id_token = parser->current_token;
    
    if (id_token.text) id_token.text = strdup(id_token.text);
    if (!expect(parser, TOKEN_IDENTIFIER, "Expected an identifier, got '%s'", parser->current_token.text)) {
        if (id_token.text) free(id_token.text);
        return NULL;
    }

    ASTNode* identifier_node = create_ast_node(AST_IDENTIFIER, id_token);

    return identifier_node;
}

ASTNode* parse_primary(Parser* parser) {
    TokenType type = parser->current_token.type;

    switch (type) {
        case TOKEN_INT_LITERAL:
        case TOKEN_FLOAT_LITERAL: {
            Token tok = parser->current_token;
            if (tok.text) tok.text = strdup(tok.text);
            match(parser, type);
            ASTNode* node = create_ast_node(AST_LITERAL, tok);

            return node;
        }

        case TOKEN_TRUE:
        case TOKEN_FALSE: {
            Token tok = parser->current_token;
            if (tok.text) tok.text = strdup(tok.text);
            match(parser, type);
            ASTNode* node = create_ast_node(AST_LITERAL, tok);

            return node;
        }

        case TOKEN_STRING: {
            Token tok = parser->current_token;
            if (tok.text) tok.text = strdup(tok.text);
            match(parser, TOKEN_STRING);
            ASTNode* node = create_ast_node(AST_LITERAL, tok);

            return node;
        }

        case TOKEN_LPAREN: {
            match(parser, TOKEN_LPAREN);
            ASTNode* expr = parse_binary(parser);
            if (!expr) {
                set_error(parser, "Expected expression inside parentheses");
                return NULL;
            }

            if (parser->current_token.type != TOKEN_RPAREN) {
                set_error(parser, "Expected ')'");
                return NULL;
            }

            match(parser, TOKEN_RPAREN);
            return expr;
        }

        case TOKEN_ARRAY: {
            match(parser, TOKEN_ARRAY);
            ASTNode* array_node = create_ast_node(AST_ARRAY_LITERAL, parser->current_token);

            array_node->child_count = 0;
            array_node->children = NULL;

            while (parser->current_token.type != TOKEN_RBRACE) {
                ASTNode* element = parse_binary(parser);
                if (!element) {
                    set_error(parser, "Expected array element");
                    return NULL;
                }

                array_node->child_count++;
                array_node->children = realloc(array_node->children, sizeof(ASTNode*) * array_node->child_count);
                array_node->children[array_node->child_count - 1] = element;

                if (parser->current_token.type == TOKEN_COMMA) {
                    match(parser, TOKEN_COMMA);
                } else {
                    break;
                }
            }

            if (parser->current_token.type != TOKEN_RBRACE) {
                set_error(parser, "Expected '}' at end of array");
                return NULL;
            }
 
            match(parser, TOKEN_RBRACE);
            return array_node;
        }

        case TOKEN_MINUS:
        case TOKEN_PLUS: {
            Token op_token = parser->current_token;
            if (op_token.text) op_token.text = strdup(op_token.text);
            match(parser, type);

            ASTNode* operand = parse_primary(parser);
            if (!operand) {
                set_error(parser, "Expected expression after unary operator '%s'", op_token.text);
                return NULL;
            }

            ASTNode* node = create_ast_node(AST_UNARY_OP, op_token);
 
            node->child_count = 1;
            node->children = malloc(sizeof(ASTNode*));
            node->children[0] = operand;

            return node;
        }

        default:
            set_error(parser, "Unexpected token '%s'", parser->current_token.text);
            return NULL;
    }
}
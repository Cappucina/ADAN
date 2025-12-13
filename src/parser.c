#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#define UNUSED(x) (void)(x)

// Forward declarations for locally defined helpers
static ASTNode* parse_single_comment(Parser* parser);
#include "semantic.h"
#include "logs.h"

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
		set_error(parser, PARSER_INITIALIZATION_FAILURE);
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

const char* get_error_message(int error_code) {
	if (error_code >= 0 && error_code < sizeof(ParserErrorMessages) / sizeof(ParserErrorMessages[0])) {
		return ParserErrorMessages[error_code];
	}
	return "Unknown error";
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

bool expect(Parser* parser, TokenType type, int error_msg, ...) {
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

	const char* error_format = get_error_message(error_msg);
	
	va_list args_copy;
	va_copy(args_copy, args);
	int f = vsnprintf(NULL, 0, error_format, args);
	char* formatted = malloc(f + 1);
	if (formatted == NULL) {
		parser->error_message = NULL;
		parser->error = true;
		va_end(args_copy);
		va_end(args);
		return false;
	}
	vsnprintf(formatted, f + 1, error_format, args_copy);
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

void set_error(Parser* parser, int error_code, ...) {
	if (parser->error) return;
	
	free(parser->error_message);
	
	va_list args;
	va_start(args, error_code);

	const char* fmt = get_error_message(error_code);

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
		set_error(NULL, PARSER_ALLOCATION_FAILURE, "AST node");
		return NULL;
	}
	node->type = type;
	node->token.type = token.type;
	node->token.line = token.line;
	node->token.column = token.column;
	if (token.text) {
		node->token.text = strdup(token.text);
	} else {
		node->token.text = NULL;
	}
	node->child_count = 0;
	node->children = NULL;
	node->annotated_type = TYPE_UNKNOWN;
	return node;
}

ASTNode* parse_statement(Parser* parser) { 
	Token current_token = parser->current_token;

	switch(parser->current_token.type)
	{
		case TOKEN_IDENTIFIER:
			switch(parser->peek_token.type) {
				case TOKEN_TYPE_DECL:
					return parse_declaration(parser);
					break;
				case TOKEN_ASSIGN:
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
		case TOKEN_BREAK:
			return parse_break_statement(parser);
			break;
		case TOKEN_RETURN:
			return parse_return_statement(parser);
			break;
		case TOKEN_PROGRAM:
			return parse_program(parser);
			break;
		case TOKEN_INCLUDE:
			return parse_include_statement(parser);
			break;

		case TOKEN_SINGLE_COMMENT:
			return parse_single_comment(parser);
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
			set_error(parser, PARSER_UNEXPECTED_TOKEN, current_token.text);
			return NULL;
	}
}

static ASTNode* parse_single_comment(Parser* parser) {
	ASTNode* node = create_ast_node(AST_SINGLE_COMMENT, parser->current_token);
	if (!match(parser, TOKEN_SINGLE_COMMENT)) {
		set_error(parser, PARSER_UNEXPECTED_TOKEN, parser->current_token.text);
		return NULL;
	}
	return node;
}

ASTNode* parse_break_statement(Parser* parser) {
	ASTNode* node = create_ast_node(AST_BREAK, parser->current_token);
	if (!expect(parser, TOKEN_BREAK, PARSER_EXPECTED, "'break'", parser->current_token.text)) {
		return NULL;
	}
	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'' after 'break'", parser->current_token.text)) {
		return NULL;
	}
	return node;
}

ASTNode* parse_return_statement(Parser* parser) {
	ASTNode* node = create_ast_node(AST_RETURN, parser->current_token);
	if (!expect(parser, TOKEN_RETURN, PARSER_EXPECTED, "'return'", parser->current_token.text)) {
		return NULL;
	}

	if (parser->current_token.type != TOKEN_SEMICOLON) {
		ASTNode* expr = parse_expression(parser);
		if (!expr) {
			set_error(parser, PARSER_EXPECTED, "'return'", parser->current_token.text);
			return NULL;
		}
		add_child(node, expr);
	}

	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'' after 'return'", parser->current_token.text)) {
		return NULL;
	}

	return node;
}

ASTNode* parse_file(Parser* parser) {
	ASTNode* file_node = create_ast_node(AST_FILE, parser->current_token);
	int include_count = 0;
	ASTNode** includes = NULL;
	
	while (parser->current_token.type == TOKEN_INCLUDE) {
		ASTNode* include = parse_include_statement(parser);
		if (!include) {
			for (int i = 0; i < include_count; i++) {
				free_ast(includes[i]);
			}
			free(includes);
			free(file_node);
			return NULL;
		}
		
		ASTNode** temp = realloc(includes, sizeof(ASTNode*) * (include_count + 1));
		if (!temp) {
			free_ast(include);
			for (int i = 0; i < include_count; i++) {
				free_ast(includes[i]);
			}
			free(includes);
			free(file_node);
			return NULL;
		}
		includes = temp;
		includes[include_count++] = include;
	}
	
	int program_count = 0;
	ASTNode** programs = NULL;
	
	while (parser->current_token.type == TOKEN_PROGRAM) {
		ASTNode* program = parse_program(parser);
		if (!program) {
			for (int i = 0; i < include_count; i++) {
				free_ast(includes[i]);
			}
			for (int i = 0; i < program_count; i++) {
				free_ast(programs[i]);
			}
			free(includes);
			free(programs);
			free(file_node);
			return NULL;
		}
		
		ASTNode** temp = realloc(programs, sizeof(ASTNode*) * (program_count + 1));
		if (!temp) {
			free_ast(program);
			for (int i = 0; i < include_count; i++) {
				free_ast(includes[i]);
			}
			for (int i = 0; i < program_count; i++) {
				free_ast(programs[i]);
			}
			free(includes);
			free(programs);
			free(file_node);
			return NULL;
		}
		programs = temp;
		programs[program_count++] = program;
	}
	
	if (program_count == 0) {
		for (int i = 0; i < include_count; i++) {
			free_ast(includes[i]);
		}
		free(includes);
		free(file_node);
		set_error(parser, PARSER_EXPECTED, "at least one program function", parser->current_token.text);
		return NULL;
	}
	
	file_node->child_count = include_count + program_count;
	file_node->children = malloc(sizeof(ASTNode*) * file_node->child_count);
	if (!file_node->children) {
		for (int i = 0; i < include_count; i++) {
			free_ast(includes[i]);
		}
		for (int i = 0; i < program_count; i++) {
			free_ast(programs[i]);
		}
		free(includes);
		free(programs);
		free(file_node);
		return NULL;
	}
	
	for (int i = 0; i < include_count; i++) {
		file_node->children[i] = includes[i];
	}
	for (int i = 0; i < program_count; i++) {
		file_node->children[include_count + i] = programs[i];
	}
	
	free(includes);
	free(programs);
	return file_node;
}


ASTNode* parse_program(Parser* parser) {
	ASTNode* parse_node = create_ast_node(AST_PROGRAM, parser->current_token);

	if (!expect(parser, TOKEN_PROGRAM, PARSER_EXPECTED, "keyword 'program'", parser->current_token.text)) return NULL;
	if (!expect(parser, TOKEN_TYPE_DECL, PARSER_EXPECTED, "type declarator '::' when defining a type", parser->current_token.text)) return NULL;

	ASTNode* type = create_ast_node(AST_TYPE, parser->current_token);
	if (!match(parser, TOKEN_INT) && !match(parser, TOKEN_VOID) &&
		!match(parser, TOKEN_CHAR) && !match(parser, TOKEN_FLOAT) &&
		!match(parser, TOKEN_BOOLEAN) && !match(parser, TOKEN_STRING)) {
			set_error(parser, PARSER_EXPECTED, "type", parser->current_token.text);
			return NULL;
	}

	ASTNode* identifier = parse_identifier(parser);
	if (!identifier) {
		set_error(parser, PARSER_EXPECTED, "identifier", parser->current_token.text);
		return NULL;
	}

	ASTNode* params = parse_params(parser);
	if (!params) {
		set_error(parser, PARSER_EXPECTED, "parameters or at least an empty set of parenthesis ('( )')", parser->current_token.text);
		return NULL;
	}

	ASTNode* block = parse_block(parser);
	if (!block) {
		set_error(parser, PARSER_EXPECTED, "code block or at least an empty set of braces ('{ }')", parser->current_token.text);
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
	if (!expect(parser, TOKEN_LPAREN, PARSER_EXPECTED, "'('", parser->current_token.text)) return NULL;

	ASTNode** params = NULL;
	int count = 0;

	if (parser->current_token.type != TOKEN_RPAREN) {
		while (1) {
			Token id = parser->current_token;
			if (!expect(parser, TOKEN_IDENTIFIER, PARSER_EXPECTED, "parameter name", parser->current_token.text)) {
				return NULL;
			}
			if (!expect(parser, TOKEN_TYPE_DECL, PARSER_EXPECTED, "'::' after parameter name", parser->current_token.text)) {
				return NULL;
			}

			Token type = parser->current_token;
			if (!match(parser, TOKEN_INT) && !match(parser, TOKEN_FLOAT) &&
				!match(parser, TOKEN_STRING) && !match(parser, TOKEN_BOOLEAN) &&
				!match(parser, TOKEN_CHAR) && !match(parser, TOKEN_NULL) &&
				!match(parser, TOKEN_VOID)) {
				set_error(parser, PARSER_EXPECTED, "type declaration for parameter", parser->current_token.text);
				return NULL;
			}

			ASTNode* id_node = create_ast_node(AST_IDENTIFIER, id);
			ASTNode* param_node = create_ast_node(AST_TYPE, type);
			if (!id_node || !param_node) {
				set_error(parser, PARSER_FAILED_AST, "parameter");
				return NULL;
			}

			ASTNode* assign = create_ast_node(AST_ASSIGNMENT, id);
			assign->child_count = 2;
			assign->children = malloc(sizeof(ASTNode*) * 2);
			assign->children[0] = id_node;
			assign->children[1] = param_node;

			ASTNode** tmp = realloc(params, sizeof(ASTNode*) * (count + 1));
			if (!tmp) {
				set_error(parser, PARSER_ALLOCATION_FAILURE, "parameters");
				return NULL;
			}

			params = tmp;
			params[count] = assign;
			count++;
			
			if (parser->current_token.type == TOKEN_COMMA) {
				if (!match(parser, TOKEN_COMMA)) {
					set_error(parser, PARSER_EXPECTED, "','", parser->current_token.text);
					return NULL;
				}
				if (parser->current_token.type == TOKEN_RPAREN) {
					set_error(parser, PARSER_TRAILING_COMMA);
					return NULL;
				}
				continue;
			}
			break;
		}
	}
	if (!expect(parser, TOKEN_RPAREN, PARSER_EXPECTED, "')'", parser->current_token.text)) return NULL;

	node->child_count = count;
	node->children = params;
	return node;
}

ASTNode* parse_declaration(Parser* parser) {
	Token identifier_token = parser->current_token;
	if (identifier_token.text) {
		identifier_token.text = strdup(identifier_token.text);
	}

	if (!expect(parser, TOKEN_IDENTIFIER, PARSER_EXPECTED, "identifier", parser->current_token.text)) {
		if (identifier_token.text) free(identifier_token.text);
		return NULL;
	}
	if (!expect(parser, TOKEN_TYPE_DECL, PARSER_EXPECTED, "'::'", parser->current_token.text)) {
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
			set_error(parser, PARSER_EXPECTED, "type declaration", parser->current_token.text);
			return NULL;
	}

	ASTNode* identifier = create_ast_node(AST_IDENTIFIER, identifier_token);
	ASTNode* type = create_ast_node(AST_TYPE, type_token);

	if (!identifier || !type) 
	{
		set_error(parser, PARSER_FAILED_AST, "assignment AST nodes");
		return NULL;
	}

	ASTNode* assignment_node = create_ast_node(AST_DECLARATION, identifier_token);

	if (parser->current_token.type == TOKEN_ASSIGN) {
		match(parser, TOKEN_ASSIGN);

		ASTNode* expression = parse_expression(parser);
		if (!expression) {
			set_error(parser, PARSER_EXPECTED, "expression", parser->current_token.text);
			free_ast(identifier);
			free_ast(type);
			free_ast(assignment_node);
			return NULL;
		}

		if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) {
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
		if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) {
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

ASTNode* parse_assignment(Parser* parser) {
	Token identifier_token = parser->current_token;

	if (!expect(parser, TOKEN_IDENTIFIER, PARSER_EXPECTED, "identifier", parser->current_token.text)) {
		return NULL;
	}

	ASTNode* identifier = create_ast_node(AST_IDENTIFIER, identifier_token);
	ASTNode* assignment_node = create_ast_node(AST_ASSIGNMENT, identifier_token);

	if (!identifier || !assignment_node) {
		set_error(parser, PARSER_FAILED_AST, "assignment AST nodes");
		return NULL;
	}

	if (!expect(parser, TOKEN_ASSIGN, PARSER_EXPECTED, "'='", parser->current_token.text)) {
		free_ast(identifier);
		free_ast(assignment_node);
	
		return NULL;
	}

	ASTNode* expression = parse_expression(parser);
	if (!expression) {
		set_error(parser, PARSER_EXPECTED, "expression", parser->current_token.text);
		free_ast(identifier);
		free_ast(assignment_node);
		return NULL;
	}

	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) {
		free_ast(identifier);
		free_ast(expression);
		free_ast(assignment_node);
	
		return NULL;
	}

	assignment_node->child_count = 2;
	assignment_node->children = malloc(sizeof(ASTNode*) * 2);
	
	if (!assignment_node->children) {
		free_ast(identifier);
		free_ast(expression);
		free_ast(assignment_node);

		return NULL;
	}

	assignment_node->children[0] = identifier;
	assignment_node->children[1] = expression;

	return assignment_node;
}

ASTNode* parse_expression(Parser* parser) {
	return parse_binary(parser);
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
			set_error(parser, PARSER_EXPECTED, "expression after '%s'", op_token.text);
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
			set_error(parser, PARSER_EXPECTED, "expression after unary operator '%s'", op_token.text);
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

	if (!expect(parser, TOKEN_IF, PARSER_EXPECTED, "'if'", parser->current_token.text)) return NULL;
	if (!expect(parser, TOKEN_LPAREN, PARSER_EXPECTED, "'('", parser->current_token.text)) return NULL;

	ASTNode* left = parse_statement(parser);
	if (!left) {
		set_error(parser, PARSER_EXPECTED, "expression in if condition", parser->current_token.text);
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
				set_error(parser, PARSER_EXPECTED, "comparison operator", parser->current_token.text);
				return NULL;
		}
		match(parser, t);

		ASTNode* right = parse_statement(parser);
		if (!right) {
			set_error(parser, PARSER_EXPECTED, "expression in if condition", parser->current_token.text);
			return NULL;
		}

		condition = create_ast_node(AST_COMPARISON, op);
		condition->child_count = 2;
		condition->children = malloc(sizeof(ASTNode*) * condition->child_count);
		condition->children[0] = left;
		condition->children[1] = right;
	}

	if (!expect(parser, TOKEN_RPAREN, PARSER_EXPECTED, "')'", parser->current_token.text)) return NULL;

	ASTNode* then_block = parse_block(parser);

	if (match(parser, TOKEN_ELSE)) {
		if_node->child_count = 3;
		if_node->children = malloc(sizeof(ASTNode*) * 3);
		if_node->children[0] = condition;
		if_node->children[1] = then_block;

		if (parser->current_token.type == TOKEN_IF) {
			if_node->children[2] = parse_if_statement(parser);
		} else {
			ASTNode* else_block = parse_block(parser);
			if_node->children[2] = else_block;
		}
	} else {
		if_node->child_count = 2;
		if_node->children = malloc(sizeof(ASTNode*) * 2);
		if_node->children[0] = condition;
		if_node->children[1] = then_block;
	}

	return if_node;
}


ASTNode* parse_while_statement(Parser* parser) {
	ASTNode* while_node = create_ast_node(AST_WHILE, parser->current_token);

	if (!expect(parser, TOKEN_WHILE, PARSER_EXPECTED, "'while'", parser->current_token.text)) return NULL;
	if (!expect(parser, TOKEN_LPAREN, PARSER_EXPECTED, "'('", parser->current_token.text)) return NULL;

	ASTNode* left = parse_statement(parser);
	if (!left) {
		set_error(parser, PARSER_EXPECTED, "expression in while condition", parser->current_token.text);
		return NULL;
	}

	ASTNode* condition = NULL;

	if (parser->current_token.type == TOKEN_RPAREN) {
		condition = left;
	} else {
		Token op = parser->current_token;
		if (op.type != TOKEN_EQUALS && op.type != TOKEN_GREATER &&
			op.type != TOKEN_LESS && op.type != TOKEN_GREATER_EQUALS &&
			op.type != TOKEN_LESS_EQUALS && op.type != TOKEN_NOT_EQUALS &&
			op.type != TOKEN_AND) {
				set_error(parser, PARSER_EXPECTED, "comparison operator", parser->current_token.text);
				return NULL;
		}

		match(parser, op.type);

		ASTNode* right = parse_statement(parser);
		if (!right) {
			set_error(parser, PARSER_EXPECTED, "expression in while condition", parser->current_token.text);
			return NULL;
		}

		condition = create_ast_node(AST_COMPARISON, op);
		condition->child_count = 2;
		condition->children = malloc(sizeof(ASTNode*) * 2);
		condition->children[0] = left;
		condition->children[1] = right;
	}

	if (!expect(parser, TOKEN_RPAREN, PARSER_EXPECTED, "')'", parser->current_token.text)) return NULL;

	ASTNode* block = parse_block(parser);

	while_node->child_count = 2;
	while_node->children = malloc(sizeof(ASTNode*) * while_node->child_count);
	while_node->children[0] = condition;
	while_node->children[1] = block;

	return while_node;
}


ASTNode* parse_for_statement(Parser* parser) {
	if (!expect(parser, TOKEN_FOR, PARSER_EXPECTED, "'for'", parser->current_token.text)) return NULL;
	if (!expect(parser, TOKEN_LPAREN, PARSER_EXPECTED, "'('", parser->current_token.text)) return NULL;

	Token id_token = parser->current_token;
	if (id_token.text) id_token.text = strdup(id_token.text);

	if (!expect(parser, TOKEN_IDENTIFIER, PARSER_EXPECTED, "identifier", parser->current_token.text)) {
		if (id_token.text) free(id_token.text);
		return NULL;
	}
	
	if (!expect(parser, TOKEN_TYPE_DECL, PARSER_EXPECTED, "'::'", parser->current_token.text)) return NULL;

	TokenType t = parser->current_token.type;
	if (t != TOKEN_INT && t != TOKEN_FLOAT && t != TOKEN_STRING &&
		t != TOKEN_BOOLEAN && t != TOKEN_CHAR && t != TOKEN_NULL &&
		t != TOKEN_VOID) {
			if (id_token.text) free(id_token.text);
			set_error(parser, PARSER_EXPECTED, "type for for-loop variable", parser->current_token.text);
			return NULL;
	}

	Token type_token = parser->current_token;
	if (type_token.text) type_token.text = strdup(type_token.text);
	match(parser, t);
   
	if (!expect(parser, TOKEN_ASSIGN, PARSER_EXPECTED, "'='", parser->current_token.text)) {
		if (id_token.text) free(id_token.text);
		if (type_token.text) free(type_token.text);
		return NULL;
	}

	ASTNode* initial_value = parse_expression(parser);
	if (!initial_value) {
		return NULL;
	}

	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) {
		free_ast(initial_value);
		return NULL;
	}

	ASTNode* left_cond = parse_expression(parser);
	Token op_token = parser->current_token;
	Token cmp_token = (Token){ .type = op_token.type, .text = NULL };
	if (!match(parser, TOKEN_LESS) && !match(parser, TOKEN_GREATER) &&
		!match(parser, TOKEN_EQUALS) && !match(parser, TOKEN_NOT_EQUALS) &&
		!match(parser, TOKEN_LESS_EQUALS) && !match(parser, TOKEN_GREATER_EQUALS)) {
			set_error(parser, PARSER_EXPECTED, "comparison operator", parser->current_token.text);
			free_ast(left_cond);
			free_ast(initial_value);
			return NULL;
		}

	ASTNode* right_cond = parse_expression(parser);
	if (!right_cond) {
		free_ast(left_cond);
		free_ast(initial_value);
		return NULL;
	}

	ASTNode* condition_node = create_ast_node(AST_COMPARISON, cmp_token);
	condition_node->child_count = 2;
	condition_node->children = malloc(sizeof(ASTNode*) * 2);
	condition_node->children[0] = left_cond;
	condition_node->children[1] = right_cond;

	ASTNode* identifier_node = create_ast_node(AST_IDENTIFIER, id_token);
	ASTNode* type_node = create_ast_node(AST_TYPE, type_token);
	ASTNode* assignment_node = create_ast_node(AST_DECLARATION, (Token){0});
	assignment_node->child_count = 3;
	assignment_node->children = malloc(sizeof(ASTNode*) * 3);
	assignment_node->children[0] = identifier_node;
	assignment_node->children[1] = type_node;
	assignment_node->children[2] = initial_value;

	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) {
		free_ast(assignment_node);
		free_ast(condition_node);
		return NULL;
	}

	Token inc_id_token = parser->current_token;
	if (inc_id_token.text) inc_id_token.text = strdup(inc_id_token.text);
	if (!expect(parser, TOKEN_IDENTIFIER, PARSER_EXPECTED, "identifier for increment", parser->current_token.text)) {
		free_ast(assignment_node);
		free_ast(condition_node);
		return NULL;
	}

	Token inc_op_token = parser->current_token;
	if (inc_op_token.text) inc_op_token.text = strdup(inc_op_token.text);
	if (!match(parser, TOKEN_INCREMENT) && !match(parser, TOKEN_DECREMENT)) {
		set_error(parser, PARSER_EXPECTED, "increment/decrement operator", parser->current_token.text);
		free_ast(assignment_node);
		free_ast(condition_node);
		return NULL;
	}

	ASTNode* inc_var_node = create_ast_node(AST_IDENTIFIER, inc_id_token);
	ASTNode* inc_op_node = create_ast_node(AST_OPERATORS, inc_op_token);
	ASTNode* increment_node = create_ast_node(AST_INCREMENT_EXPR, (Token){0});
	increment_node->child_count = 2;
	increment_node->children = malloc(sizeof(ASTNode*) * 2);
	increment_node->children[0] = inc_var_node;
	increment_node->children[1] = inc_op_node;

	if (!expect(parser, TOKEN_RPAREN, PARSER_EXPECTED, "')'", parser->current_token.text)) {
		free_ast(assignment_node);
		free_ast(condition_node);
		free_ast(increment_node);
		return NULL;
	}

	ASTNode* block_node = parse_block(parser);
	if (!block_node) {
		free_ast(assignment_node);
		free_ast(condition_node);
		free_ast(increment_node);
		return NULL;
	}

	ASTNode* for_node = create_ast_node(AST_FOR, (Token){0});
	for_node->child_count = 4;
	for_node->children = malloc(sizeof(ASTNode*) * 4);
	for_node->children[0] = assignment_node;
	for_node->children[1] = condition_node;
	for_node->children[2] = increment_node;
	for_node->children[3] = block_node;

	return for_node;
}

ASTNode* parse_function_call(Parser* parser) {
	ASTNode* node = create_ast_node(AST_FUNCTION_CALL, parser->current_token);
	ASTNode* identifier = parse_identifier(parser);
	if (!identifier) {
		free(node);
		return NULL;
	}

	if (!expect(parser, TOKEN_LPAREN, PARSER_EXPECTED, "'('", parser->current_token.text)) {
		free_ast(identifier);
		free(node);
		return NULL;
	}

	ASTNode* args = create_ast_node(AST_PARAMS, parser->current_token);
	args->child_count = 0;
	args->children = NULL;

	if (parser->current_token.type != TOKEN_RPAREN) {
		while (1) {
			ASTNode* expr = parse_expression(parser);
			if (!expr) {
				free_ast(identifier);
				free_ast(args);
				free(node);
				return NULL;
			}

			ASTNode** tmp = realloc(args->children, sizeof(ASTNode*) * (args->child_count + 1));
			if (!tmp) {
				free_ast(expr);
				free_ast(identifier);
				free_ast(args);
				free(node);
				return NULL;
			}
			args->children = tmp;
			args->children[args->child_count++] = expr;

			if (parser->current_token.type == TOKEN_COMMA) {
				match(parser, TOKEN_COMMA);
				continue;
			}
			break;
		}
	}

	if (!expect(parser, TOKEN_RPAREN, PARSER_EXPECTED, "')'", parser->current_token.text)) {
		free_ast(identifier);
		free_ast(args);
		free(node);
		return NULL;
	}

	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) {
		free_ast(identifier);
		free_ast(args);
		free(node);
		return NULL;
	}

	node->child_count = 2;
	node->children = malloc(sizeof(ASTNode*) * 2);
	node->children[0] = identifier;
	node->children[1] = args;

	return node;
}

ASTNode* parse_include_statement(Parser* parser) {
	ASTNode* include_node = create_ast_node(AST_INCLUDE, parser->current_token);
	if (!expect(parser, TOKEN_INCLUDE, PARSER_EXPECTED, "'include'", parser->current_token.text)) return NULL;
	ASTNode* publisher = parse_identifier(parser);
	if (!expect(parser, TOKEN_PERIOD, PARSER_EXPECTED, "'.'", parser->current_token.text)) return NULL;
	ASTNode* package = parse_identifier(parser);
	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) return NULL;
	include_node->child_count = 2;
	include_node->children = malloc(sizeof(ASTNode*) * 2);
	include_node->children[0] = publisher;
	include_node->children[1] = package;
	return include_node;
}

ASTNode* parse_block(Parser* parser) {
	ASTNode* node = create_ast_node(AST_BLOCK, (Token){0});
	if (!node) {
		set_error(parser, PARSER_FAILED_AST, "block node");
		return NULL;
	}

	if (!expect(parser, TOKEN_LBRACE, PARSER_EXPECTED, "'{'", parser->current_token.text)) return NULL;

	int count = 0;
	ASTNode** children = NULL;
	while(parser->current_token.type != TOKEN_RBRACE && parser->current_token.type != TOKEN_EOF) {
		ASTNode* st = parse_statement(parser);
		if (!st) {
			set_error(parser, PARSER_EXPECTED, "statement in block", parser->current_token.text);
			return NULL;
		}

		ASTNode** tmp = realloc(children, sizeof(ASTNode*) * (count + 1));
		if (!tmp) {
			set_error(parser, PARSER_ALLOCATION_FAILURE, "child nodes in block");
			return NULL;
		}
		children = tmp;
		children[count] = st;
		count++;
	}

	node->children = children;
	node->child_count = count;

	if (!expect(parser, TOKEN_RBRACE, PARSER_EXPECTED, "'}'", parser->current_token.text)) return NULL;

	return node;
}

ASTNode* parse_identifier(Parser* parser) {
	Token id_token = parser->current_token;
	if (id_token.text) id_token.text = strdup(id_token.text);
	if (!expect(parser, TOKEN_IDENTIFIER, PARSER_EXPECTED, "identifier", parser->current_token.text)) {
		if (id_token.text) free(id_token.text);
		return NULL;
	}
	ASTNode* identifier_node = create_ast_node(AST_IDENTIFIER, id_token);
	return identifier_node;
}

ASTNode* parse_primary(Parser* parser) {
	TokenType type = parser->current_token.type;

	switch (type) {
		case TOKEN_IDENTIFIER: {
			Token id_token = parser->current_token;
			if (id_token.text) id_token.text = strdup(id_token.text);
			match(parser, TOKEN_IDENTIFIER);
			
			if (parser->current_token.type == TOKEN_LPAREN) {
				match(parser, TOKEN_LPAREN);
				ASTNode* call_node = create_ast_node(AST_FUNCTION_CALL, id_token);
				ASTNode* identifier_node = create_ast_node(AST_IDENTIFIER, id_token);
				
				ASTNode* args = create_ast_node(AST_PARAMS, parser->current_token);
				args->child_count = 0;
				args->children = NULL;

				if (parser->current_token.type != TOKEN_RPAREN) {
					while (1) {
						ASTNode* expr = parse_expression(parser);
						if (!expr) {
							free_ast(identifier_node);
							free_ast(args);
							free(call_node);
							return NULL;
						}

						ASTNode** tmp = realloc(args->children, sizeof(ASTNode*) * (args->child_count + 1));
						if (!tmp) {
							free_ast(expr);
							free_ast(identifier_node);
							free_ast(args);
							free(call_node);
							return NULL;
						}
						args->children = tmp;
						args->children[args->child_count++] = expr;

						if (parser->current_token.type == TOKEN_COMMA) {
							match(parser, TOKEN_COMMA);
							continue;
						}
						break;
					}
				}

				if (!expect(parser, TOKEN_RPAREN, PARSER_EXPECTED, "')'", parser->current_token.text)) {
					free_ast(identifier_node);
					free_ast(args);
					free(call_node);
					return NULL;
				}

				call_node->child_count = 2;
				call_node->children = malloc(sizeof(ASTNode*) * 2);
				call_node->children[0] = identifier_node;
				call_node->children[1] = args;

				return call_node;
			}
			
			ASTNode* identifier_node = create_ast_node(AST_IDENTIFIER, id_token);
			return identifier_node;
		}
		case TOKEN_INT_LITERAL:
		case TOKEN_FLOAT_LITERAL: {
			Token tok = parser->current_token;
			if (tok.text) tok.text = strdup(tok.text);
			match(parser, type);
			return create_ast_node(AST_LITERAL, tok);
		}

		case TOKEN_TRUE:
		case TOKEN_FALSE: {
			Token tok = parser->current_token;
			if (tok.text) tok.text = strdup(tok.text);
			match(parser, type);
			return create_ast_node(AST_LITERAL, tok);
		}

		case TOKEN_STRING: {
			Token tok = parser->current_token;
			if (tok.text) tok.text = strdup(tok.text);
			match(parser, TOKEN_STRING);

			// If the string contains interpolation markers `${...}` then
			// split into pieces and construct a concatenation AST node.
			char* s = tok.text ? tok.text : "";
			int len = strlen(s);
			int has_interp = 0;
			for (int i = 0; i < len - 1; i++) {
				if (s[i] == '$' && s[i+1] == '{') { has_interp = 1; break; }
			}

			if (!has_interp) {
				return create_ast_node(AST_LITERAL, tok);
			}

			// Build parts: alternating string literals and expression AST nodes
			ASTNode** parts = NULL;
			int parts_count = 0;

			int i = 0;
			while (i < len) {
				if (s[i] == '$' && i + 1 < len && s[i+1] == '{') {
					// Find matching '}'
					int j = i + 2;
					while (j < len && s[j] != '}') j++;
					if (j >= len) {
						// Unterminated interpolation; treat literally
						// Append remaining as a literal
						int rem_len = len - i;
						char* lit = malloc(rem_len + 1);
						strncpy(lit, s + i, rem_len);
						lit[rem_len] = '\0';
						Token literal_token = { .type = TOKEN_STRING, .text = lit, .line = tok.line, .column = tok.column };
						ASTNode* lit_node = create_ast_node(AST_LITERAL, literal_token);
						parts = realloc(parts, sizeof(ASTNode*) * (parts_count + 1));
						parts[parts_count++] = lit_node;
						break;
					}

					int expr_len = j - (i + 2);
					char* expr_txt = malloc(expr_len + 1);
					strncpy(expr_txt, s + i + 2, expr_len);
					expr_txt[expr_len] = '\0';

					// Parse the expression into an AST using a temporary lexer & parser
					Lexer* sublexer = create_lexer(expr_txt);
					Parser subparser;
					init_parser(&subparser, sublexer);
					ASTNode* expr_node = parse_expression(&subparser);
					free_parser(&subparser);
					free(sublexer);
					free(expr_txt);

					if (!expr_node) {
						// If expression parse failed, treat literal
						int part_len = j - i + 1;
						char* lit = malloc(part_len + 1);
						strncpy(lit, s + i, part_len);
						lit[part_len] = '\0';
						Token literal_token = { .type = TOKEN_STRING, .text = lit, .line = tok.line, .column = tok.column };
						ASTNode* lit_node = create_ast_node(AST_LITERAL, literal_token);
						parts = realloc(parts, sizeof(ASTNode*) * (parts_count + 1));
						parts[parts_count++] = lit_node;
					} else {
						parts = realloc(parts, sizeof(ASTNode*) * (parts_count + 1));
						parts[parts_count++] = expr_node;
					}

					i = j + 1;
					continue;
				}

				// accumulate literal until next '${'
				int start = i;
				while (i < len) {
					if (s[i] == '$' && i + 1 < len && s[i+1] == '{') break;
					i++;
				}
				int seg_len = i - start;
				if (seg_len > 0) {
					char* lit = malloc(seg_len + 1);
					strncpy(lit, s + start, seg_len);
					lit[seg_len] = '\0';
					Token literal_token = { .type = TOKEN_STRING, .text = lit, .line = tok.line, .column = tok.column };
					ASTNode* lit_node = create_ast_node(AST_LITERAL, literal_token);
					parts = realloc(parts, sizeof(ASTNode*) * (parts_count + 1));
					parts[parts_count++] = lit_node;
				}
			}

			free(tok.text);

			if (parts_count == 0) return NULL;
			if (parts_count == 1) return parts[0];

			// Build left-to-right concatenation binary plus nodes: left + right + ...
			ASTNode* left = parts[0];
			for (int p = 1; p < parts_count; p++) {
				Token plus_tok = { .type = TOKEN_PLUS, .text = strdup("+"), .line = tok.line, .column = tok.column };
				ASTNode* plus_node = create_ast_node(AST_BINARY_OP, plus_tok);
				plus_node->child_count = 2;
				plus_node->children = malloc(sizeof(ASTNode*) * 2);
				plus_node->children[0] = left;
				plus_node->children[1] = parts[p];
				left = plus_node;
			}

			free(parts);
			return left;
		}

		case TOKEN_LPAREN: {
			match(parser, TOKEN_LPAREN);
			ASTNode* expr = parse_binary(parser);
			if (!expr) {
				set_error(parser, PARSER_EXPECTED, "expression inside parentheses", parser->current_token.text);
				return NULL;
			}
			if (parser->current_token.type != TOKEN_RPAREN) {
				set_error(parser, PARSER_EXPECTED, "')'", parser->current_token.text);
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
					set_error(parser, PARSER_EXPECTED, "array element", parser->current_token.text);
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
				set_error(parser, PARSER_EXPECTED, "'}' at end of array", parser->current_token.text);
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
				set_error(parser, PARSER_EXPECTED, "expression after unary operator", op_token.text);
				return NULL;
			}
			ASTNode* node = create_ast_node(AST_UNARY_OP, op_token);
			node->child_count = 1;
			node->children = malloc(sizeof(ASTNode*));
			node->children[0] = operand;
			return node;
		}

		default:
			set_error(parser, PARSER_UNEXPECTED_TOKEN, parser->current_token.text);
			return NULL;
	}
}
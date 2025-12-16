#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#define UNUSED(x) (void)(x)
#include "semantic.h"
#include "logs.h"

static ASTNode* parse_single_comment(Parser* parser);

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

	if (current->type == TOKEN_ERROR) {
		char error_msg[256];
		snprintf(error_msg, sizeof(error_msg), "Lexer error at initialization: %s (line %d, column %d)", 
			current->text ? current->text : "unknown error", current->line, current->column);
		free_token(current);
		if (peek) free_token(peek);
		parser->error_message = strdup(error_msg);
		parser->error = true;
		return;
	}
	if (peek->type == TOKEN_ERROR) {
		char error_msg[256];
		snprintf(error_msg, sizeof(error_msg), "Lexer error at initialization: %s (line %d, column %d)", 
			peek->text ? peek->text : "unknown error", peek->line, peek->column);
		free_token(current);
		free_token(peek);
		parser->error_message = strdup(error_msg);
		parser->error = true;
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
					case TOKEN_ADD_IMMEDIATE:
					case TOKEN_SUB_IMMEDIATE:
					case TOKEN_MUL_IMMEDIATE:
					case TOKEN_DIV_IMMEDIATE:
					case TOKEN_MOD_IMMEDIATE:
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
		case TOKEN_CONTINUE:
			return parse_continue_statement(parser);
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
	if (!(parser->current_token.type == TOKEN_SINGLE_COMMENT || parser->current_token.type == TOKEN_BLOCK_COMMENT)) {
		set_error(parser, PARSER_UNEXPECTED_TOKEN, parser->current_token.text);
		return NULL;
	}
	match(parser, parser->current_token.type);
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

ASTNode* parse_continue_statement(Parser* parser) {
	ASTNode* node = create_ast_node(AST_CONTINUE, parser->current_token);
	if (!expect(parser, TOKEN_CONTINUE, PARSER_EXPECTED, "'continue'", parser->current_token.text)) {
		return NULL;
	}
	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';' after 'continue'", parser->current_token.text)) {
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
	
	int declaration_count = 0;
	ASTNode** declarations = NULL;

	int program_count = 0;
	ASTNode** programs = NULL;
	
	while (parser->current_token.type == TOKEN_IDENTIFIER && parser->peek_token.type == TOKEN_TYPE_DECL) {
		ASTNode* decl = parse_declaration(parser);
		if (!decl) {
			for (int i = 0; i < include_count; i++) free_ast(includes[i]);
			for (int i = 0; i < declaration_count; i++) free_ast(declarations[i]);
			free(includes);
			free(declarations);
			free(file_node);
			return NULL;
		}

		ASTNode** temp = realloc(declarations, sizeof(ASTNode*) * (declaration_count + 1));
		if (!temp) {
			free_ast(decl);
			for (int i = 0; i < include_count; i++) free_ast(includes[i]);
			for (int i = 0; i < declaration_count; i++) free_ast(declarations[i]);
			free(includes);
			free(declarations);
			free(file_node);
			return NULL;
		}
		declarations = temp;
		declarations[declaration_count++] = decl;
	}

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
		for (int i = 0; i < declaration_count; i++) {
			free_ast(declarations[i]);
		}
		free(includes);
		free(file_node);
		set_error(parser, PARSER_EXPECTED, "at least one program function", parser->current_token.text);
		return NULL;
	}
	
	file_node->child_count = include_count + declaration_count + program_count;
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
	
	int idx = 0;
	for (int i = 0; i < include_count; i++) {
		file_node->children[i] = includes[i];
		idx++;
	}
	for (int i = 0; i < declaration_count; i++) {
		file_node->children[idx++] = declarations[i];
	}
	for (int i = 0; i < program_count; i++) {
		file_node->children[idx++] = programs[i];
	}
	
	free(includes);
	free(declarations);
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

	int is_pointer = 0;
	int is_array = 0;

	if (parser->current_token.type == TOKEN_ASTERISK) {
		is_pointer = 1;
		match(parser, TOKEN_ASTERISK);
	} else if (parser->current_token.type == TOKEN_LBRACKET) {
		is_array = 1;
		match(parser, TOKEN_LBRACKET);
		if (!expect(parser, TOKEN_RBRACKET, PARSER_EXPECTED, "']'", parser->current_token.text)) {
			if (identifier_token.text) free(identifier_token.text);
			if (type_token.text) free(type_token.text);
			return NULL;
		}
	}

	ASTNode* identifier = create_ast_node(AST_IDENTIFIER, identifier_token);
	ASTNode* type = create_ast_node(AST_TYPE, type_token);

	if (is_pointer && type->token.text) {
		char* new_text = malloc(strlen(type->token.text) + 2);
		sprintf(new_text, "%s*", type->token.text);
		free(type->token.text);
		type->token.text = new_text;
	} else if (is_array && type->token.text) {
		char* new_text = malloc(strlen(type->token.text) + 3);
		sprintf(new_text, "%s[]", type->token.text);
		free(type->token.text);
		type->token.text = new_text;
	}

	if (!identifier || !type) 
	{
		set_error(parser, PARSER_FAILED_AST, "assignment AST nodes");
		return NULL;
	}

	ASTNode* assignment_node = create_ast_node(AST_DECLARATION, identifier_token);

	if (parser->current_token.type == TOKEN_ASSIGN) {
		match(parser, TOKEN_ASSIGN);

		ASTNode* expression = NULL;

		if (is_array && parser->current_token.type == TOKEN_LBRACE) {
			match(parser, TOKEN_LBRACE);
			expression = create_ast_node(AST_ARRAY_LITERAL, parser->current_token);
			expression->child_count = 0;
			expression->children = NULL;

			while (parser->current_token.type != TOKEN_RBRACE && parser->current_token.type != TOKEN_EOF) {
				ASTNode* element = parse_expression(parser);
				if (!element) {
					set_error(parser, PARSER_EXPECTED, "array element", parser->current_token.text);
					free_ast(identifier);
					free_ast(type);
					free_ast(expression);
					free_ast(assignment_node);
					return NULL;
				}
				expression->child_count++;
				expression->children = realloc(expression->children, sizeof(ASTNode*) * expression->child_count);
				expression->children[expression->child_count - 1] = element;

				if (parser->current_token.type == TOKEN_COMMA) {
					match(parser, TOKEN_COMMA);
				} else {
					break;
				}
			}

			if (!expect(parser, TOKEN_RBRACE, PARSER_EXPECTED, "'}'", parser->current_token.text)) {
				free_ast(identifier);
				free_ast(type);
				free_ast(expression);
				free_ast(assignment_node);
				return NULL;
			}
		} else {
			expression = parse_expression(parser);
		}

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
	if (identifier_token.text) {
		identifier_token.text = strdup(identifier_token.text);
	}

	if (!expect(parser, TOKEN_IDENTIFIER, PARSER_EXPECTED, "identifier", parser->current_token.text)) {
		if (identifier_token.text) free(identifier_token.text);
		return NULL;
	}

	ASTNode* identifier = create_ast_node(AST_IDENTIFIER, identifier_token);
	ASTNode* assignment_node = create_ast_node(AST_ASSIGNMENT, identifier_token);

	if (!identifier || !assignment_node) {
		set_error(parser, PARSER_FAILED_AST, "assignment AST nodes");
		return NULL;
	}

	TokenType next = parser->current_token.type;

	if (next == TOKEN_ASSIGN) {
		match(parser, TOKEN_ASSIGN);

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

	if (next == TOKEN_ADD_IMMEDIATE || next == TOKEN_SUB_IMMEDIATE || next == TOKEN_MUL_IMMEDIATE || next == TOKEN_DIV_IMMEDIATE || next == TOKEN_MOD_IMMEDIATE) {
		Token op_token = parser->current_token;
		match(parser, next);

		ASTNode* rhs = parse_expression(parser);
		if (!rhs) {
			set_error(parser, PARSER_EXPECTED, "expression after compound assignment", parser->current_token.text);
			free_ast(identifier);
			free_ast(assignment_node);
			return NULL;
		}

		if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) {
			free_ast(identifier);
			free_ast(rhs);
			free_ast(assignment_node);
			return NULL;
		}

		Token bin_token = {0};
		switch (op_token.type) {
			case TOKEN_ADD_IMMEDIATE: bin_token.type = TOKEN_PLUS; bin_token.text = "+"; break;
			case TOKEN_SUB_IMMEDIATE: bin_token.type = TOKEN_MINUS; bin_token.text = "-"; break;
			case TOKEN_MUL_IMMEDIATE: bin_token.type = TOKEN_ASTERISK; bin_token.text = "*"; break;
			case TOKEN_DIV_IMMEDIATE: bin_token.type = TOKEN_SLASH; bin_token.text = "/"; break;
			case TOKEN_MOD_IMMEDIATE: bin_token.type = TOKEN_PERCENT; bin_token.text = "%"; break;
			default: free_ast(identifier); free_ast(rhs); free_ast(assignment_node); return NULL;
		}

		ASTNode* bin = create_ast_node(AST_BINARY_OP, bin_token);
		bin->child_count = 2;
		bin->children = malloc(sizeof(ASTNode*) * 2);
		bin->children[0] = identifier;
		bin->children[1] = rhs;

		assignment_node->child_count = 2;
		assignment_node->children = malloc(sizeof(ASTNode*) * 2);
		assignment_node->children[0] = create_ast_node(AST_IDENTIFIER, identifier->token);
		assignment_node->children[1] = bin;

		return assignment_node;
	}

	set_error(parser, PARSER_EXPECTED, "'=' or compound assignment", parser->current_token.text);
	free_ast(identifier);
	free_ast(assignment_node);
	return NULL;
}

ASTNode* parse_expression(Parser* parser) {
	return parse_binary(parser);
}

int get_node_precedence(ASTNode* node) {
	if (!node) return -1;

	switch (node->token.type) {
		case TOKEN_EXPONENT: // **
			return 5;
		
		case TOKEN_CAROT: // ^
			return 6;

		case TOKEN_ASTERISK: // *
		case TOKEN_SLASH: // /
		case TOKEN_PERCENT: // %
			return 5;
		
		case TOKEN_PLUS: // +
		case TOKEN_MINUS: // -
			return 4;

		case TOKEN_GREATER:
		case TOKEN_LESS:
		case TOKEN_GREATER_EQUALS:
		case TOKEN_LESS_EQUALS:
		case TOKEN_EQUALS:
		case TOKEN_NOT_EQUALS:
			return 3;

		case TOKEN_AND:
			return 2;

		case TOKEN_OR:
			return 1;
		
		case TOKEN_AMPERSAND:
			return 1;
		
		case TOKEN_PIPE:
			return 0;
		
		default:
			return -1;
	}
}

static int precedence_of(TokenType t) {
	switch (t) {
		case TOKEN_CAROT: return 6;
		case TOKEN_ASTERISK:
		case TOKEN_SLASH:
		case TOKEN_PERCENT: return 5;
		case TOKEN_PLUS:
		case TOKEN_MINUS: return 4;
		case TOKEN_GREATER:
		case TOKEN_LESS:
		case TOKEN_GREATER_EQUALS:
		case TOKEN_LESS_EQUALS:
		case TOKEN_EQUALS:
		case TOKEN_NOT_EQUALS: return 3;
		case TOKEN_AND: return 2;
		case TOKEN_OR: return 1;
		default: return -1;
	}
}

static int is_right_assoc(TokenType t) {
	return t == TOKEN_CAROT;
}

static ASTNode* parse_binary_prec(Parser* parser, int min_prec) {
	ASTNode* left = parse_unary(parser);
	if (!left) return NULL;

	while (1) {
		TokenType op_type = parser->current_token.type;
		int prec = precedence_of(op_type);
		if (prec < min_prec) break;

		Token op_token = parser->current_token;
		char* op_text_copy = NULL;
		if (op_token.text) {
			op_text_copy = strdup(op_token.text);
			op_token.text = op_text_copy;
		}
		match(parser, op_type);

		int next_min = is_right_assoc(op_type) ? prec : prec + 1;
		ASTNode* right = parse_binary_prec(parser, next_min);
		if (!right) {
			set_error(parser, PARSER_EXPECTED, "expression after '%s'", op_token.text);
			if (op_text_copy) free(op_text_copy);
			return left;
		}

		ASTNodeType node_type = AST_BINARY_OP;
		switch (op_type) {
			case TOKEN_GREATER:
			case TOKEN_LESS:
			case TOKEN_GREATER_EQUALS:
			case TOKEN_LESS_EQUALS:
			case TOKEN_EQUALS:
			case TOKEN_NOT_EQUALS:
				node_type = AST_COMPARISON; break;
			case TOKEN_AND:
			case TOKEN_OR:
				node_type = AST_LOGICAL_OP; break;
			default:
				node_type = AST_BINARY_OP; break;
		}

		ASTNode* node = create_ast_node(node_type, op_token);
		if (op_text_copy) free(op_text_copy);
		node->child_count = 2;
		node->children = malloc(sizeof(ASTNode*) * 2);
		node->children[0] = left;
		node->children[1] = right;
		left = node;
	}

	return left;
}

ASTNode* parse_binary(Parser* parser) {
	return parse_binary_prec(parser, 1);
}

ASTNode* parse_unary(Parser* parser) {
	TokenType op_type = parser->current_token.type;

	if (op_type == TOKEN_AT) {
		Token op_token = parser->current_token;
		match(parser, TOKEN_AT);

		ASTNode* operand = parse_unary(parser);
		if (!operand) {
			set_error(parser, PARSER_EXPECTED, "expression after '@'", parser->current_token.text);
			return NULL;
		}

		ASTNode* node = create_ast_node(AST_ADDRESS_OF, op_token);
		node->child_count = 1;
		node->children = malloc(sizeof(ASTNode*));
		node->children[0] = operand;

		return node;
	}

	if (op_type == TOKEN_MINUS || op_type == TOKEN_NOT) {
		Token op_token = parser->current_token;
		char* op_text_copy = NULL;
		if (op_token.text) {
			op_text_copy = strdup(op_token.text);
			op_token.text = op_text_copy;
		}
		match(parser, op_type);

		ASTNode* operand = parse_unary(parser);
		if (!operand) {
			set_error(parser, PARSER_EXPECTED, "expression after unary operator '%s'", op_token.text);
			if (op_text_copy) free(op_text_copy);
			return NULL;
		}

		ASTNode* node = create_ast_node(AST_UNARY_OP, op_token);
		if (op_text_copy) free(op_text_copy);
		
		node->child_count = 1;
		node->children = malloc(sizeof(ASTNode*));
		node->children[0] = operand;

		return node;
	}



	if (op_type == TOKEN_ASTERISK) {
		Token op_token = parser->current_token;
		match(parser, TOKEN_ASTERISK);

		ASTNode* operand = parse_unary(parser);
		if (!operand) {
			set_error(parser, PARSER_EXPECTED, "expression after '*'", parser->current_token.text);
			return NULL;
		}

		ASTNode* node = create_ast_node(AST_DEREFERENCE, op_token);
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

	ASTNode* condition = parse_expression(parser);
	if (!condition) {
		set_error(parser, PARSER_EXPECTED, "expression in if condition", parser->current_token.text);
		return NULL;
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

	ASTNode* condition = parse_expression(parser);
	if (!condition) {
		set_error(parser, PARSER_EXPECTED, "expression in while condition", parser->current_token.text);
		return NULL;
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

	ASTNode* cond = NULL;
	if (parser->current_token.type != TOKEN_SEMICOLON) {
		cond = parse_expression(parser);
		if (!cond) {
			set_error(parser, PARSER_EXPECTED, "expression in for condition", parser->current_token.text);
			if (id_token.text) free(id_token.text);
			if (type_token.text) free(type_token.text);
			return NULL;
		}
	}

	ASTNode* condition_node = cond;
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

static ASTNode* parse_include_name(Parser* parser) {
	Token id_token = parser->current_token;
	if (id_token.type == TOKEN_IDENTIFIER || id_token.type == TOKEN_STRING ||
		id_token.type == TOKEN_INT || id_token.type == TOKEN_FLOAT ||
		id_token.type == TOKEN_CHAR || id_token.type == TOKEN_BOOLEAN ||
		id_token.type == TOKEN_VOID) {
		if (id_token.text) id_token.text = strdup(id_token.text);
		match(parser, parser->current_token.type);
		return create_ast_node(AST_IDENTIFIER, id_token);
	}
	set_error(parser, PARSER_EXPECTED, "identifier or type name", parser->current_token.text);
	return NULL;
}

ASTNode* parse_include_statement(Parser* parser) {
	ASTNode* include_node = create_ast_node(AST_INCLUDE, parser->current_token);
	if (!expect(parser, TOKEN_INCLUDE, PARSER_EXPECTED, "'include'", parser->current_token.text)) return NULL;
	ASTNode* publisher = parse_include_name(parser);
	if (!publisher) return NULL;
	if (!expect(parser, TOKEN_PERIOD, PARSER_EXPECTED, "'.'", parser->current_token.text)) return NULL;
	ASTNode* package = parse_include_name(parser);
	if (!package) return NULL;
	if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';'", parser->current_token.text)) return NULL;
	include_node->child_count = 2;
	include_node->children = malloc(sizeof(ASTNode*) * 2);
	include_node->children[0] = publisher;
	include_node->children[1] = package;
	return include_node;
}

static int is_expression_like_node(ASTNode* node) {
	if (!node) return 0;
	switch (node->type) {
		case AST_LITERAL:
		case AST_IDENTIFIER:
		case AST_INCREMENT_EXPR:
		case AST_BINARY_EXPR:
		case AST_BINARY_OP:
		case AST_UNARY_EXPR:
		case AST_UNARY_OP:
		case AST_COMPARISON:
		case AST_GROUPED_EXPR:
		case AST_TERNARY_EXPR:
		case AST_CAST_EXPR:
		case AST_EXPRESSION:
		case AST_LOGICAL_OP:
		case AST_ARRAY_LITERAL:
			return 1;
		default:
			return 0;
	}
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

		if (parser->current_token.type == TOKEN_SEMICOLON) {
			if (!expect(parser, TOKEN_SEMICOLON, PARSER_EXPECTED, "';' after expression", parser->current_token.text)) {
				free_ast(st);
				return NULL;
			}
		} else if (is_expression_like_node(st)) {
			set_error(parser, PARSER_EXPECTED, "';' after expression", parser->current_token.text);
			free_ast(st);
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
	if (parser->current_token.type == TOKEN_INCREMENT || parser->current_token.type == TOKEN_DECREMENT) {
		Token inc_op_token = parser->current_token;
		if (inc_op_token.text) inc_op_token.text = strdup(inc_op_token.text);
		match(parser, parser->current_token.type);
		ASTNode* inc_op_node = create_ast_node(AST_OPERATORS, inc_op_token);
		ASTNode* increment_node = create_ast_node(AST_INCREMENT_EXPR, (Token){0});
		increment_node->child_count = 2;
		increment_node->children = malloc(sizeof(ASTNode*) * 2);
		increment_node->children[0] = identifier_node;
		increment_node->children[1] = inc_op_node;
		return increment_node;
	}

	if (parser->current_token.type == TOKEN_ASSIGN ||
		parser->current_token.type == TOKEN_ADD_IMMEDIATE ||
		parser->current_token.type == TOKEN_SUB_IMMEDIATE ||
		parser->current_token.type == TOKEN_MUL_IMMEDIATE ||
		parser->current_token.type == TOKEN_DIV_IMMEDIATE ||
		parser->current_token.type == TOKEN_MOD_IMMEDIATE) {
		Token op = parser->current_token;
		match(parser, op.type);

		if (op.type == TOKEN_ASSIGN) {
			ASTNode* expr = parse_expression(parser);
			if (!expr) { free_ast(identifier_node); return NULL; }
			ASTNode* assign = create_ast_node(AST_ASSIGNMENT, (Token){0});
			assign->child_count = 2;
			assign->children = malloc(sizeof(ASTNode*) * 2);
			assign->children[0] = identifier_node;
			assign->children[1] = expr;
			return assign;
		} else {
			ASTNode* rhs = parse_expression(parser);
			if (!rhs) { free_ast(identifier_node); return NULL; }

			Token bin_token = {0};
			switch (op.type) {
				case TOKEN_ADD_IMMEDIATE: bin_token.type = TOKEN_PLUS; bin_token.text = "+"; break;
				case TOKEN_SUB_IMMEDIATE: bin_token.type = TOKEN_MINUS; bin_token.text = "-"; break;
				case TOKEN_MUL_IMMEDIATE: bin_token.type = TOKEN_ASTERISK; bin_token.text = "*"; break;
				case TOKEN_DIV_IMMEDIATE: bin_token.type = TOKEN_SLASH; bin_token.text = "/"; break;
				case TOKEN_MOD_IMMEDIATE: bin_token.type = TOKEN_PERCENT; bin_token.text = "%"; break;
				default: free_ast(identifier_node); free_ast(rhs); return NULL;
			}

			ASTNode* bin = create_ast_node(AST_BINARY_OP, bin_token);
			bin->child_count = 2;
			bin->children = malloc(sizeof(ASTNode*) * 2);
			bin->children[0] = identifier_node;
			bin->children[1] = rhs;

			ASTNode* assign = create_ast_node(AST_ASSIGNMENT, (Token){0});
			assign->child_count = 2;
			assign->children = malloc(sizeof(ASTNode*) * 2);
			assign->children[0] = create_ast_node(AST_IDENTIFIER, identifier_node->token);
			assign->children[1] = bin;
			return assign;
		}
	}
	return identifier_node;
}

ASTNode* parse_primary(Parser* parser) {
	TokenType type = parser->current_token.type;
	ASTNode* primary_node = NULL;

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

			while (parser->current_token.type == TOKEN_LBRACKET) {
				match(parser, TOKEN_LBRACKET);
				ASTNode* index_expr = parse_expression(parser);
				if (!index_expr) {
					set_error(parser, PARSER_EXPECTED, "expression in array index", parser->current_token.text);
					free_ast(identifier_node);
					return NULL;
				}
				if (!expect(parser, TOKEN_RBRACKET, PARSER_EXPECTED, "']'", parser->current_token.text)) {
					free_ast(index_expr);
					free_ast(identifier_node);
					return NULL;
				}
				ASTNode* array_index_node = create_ast_node(AST_ARRAY_INDEX, id_token);
				array_index_node->child_count = 2;
				array_index_node->children = malloc(sizeof(ASTNode*) * 2);
				array_index_node->children[0] = identifier_node;
				array_index_node->children[1] = index_expr;
				identifier_node = array_index_node;
			}

			if (parser->current_token.type == TOKEN_INCREMENT || parser->current_token.type == TOKEN_DECREMENT) {
				Token inc_op_token = parser->current_token;
				if (inc_op_token.text) inc_op_token.text = strdup(inc_op_token.text);
				match(parser, parser->current_token.type);

				ASTNode* inc_op_node = create_ast_node(AST_OPERATORS, inc_op_token);
				ASTNode* increment_node = create_ast_node(AST_INCREMENT_EXPR, (Token){0});
				increment_node->child_count = 2;
				increment_node->children = malloc(sizeof(ASTNode*) * 2);
				increment_node->children[0] = identifier_node;
				increment_node->children[1] = inc_op_node;
				return increment_node;
			}
			return identifier_node;
		}
		case TOKEN_INT_LITERAL:
		case TOKEN_FLOAT_LITERAL: {
			Token tok = parser->current_token;
			if (tok.text) tok.text = strdup(tok.text);
			match(parser, type);
			primary_node = create_ast_node(AST_LITERAL, tok);
			break;
		}

		case TOKEN_TRUE:
		case TOKEN_FALSE: {
			Token tok = parser->current_token;
			if (tok.text) tok.text = strdup(tok.text);
			match(parser, type);
			primary_node = create_ast_node(AST_LITERAL, tok);
			break;
		}

		case TOKEN_STRING: {
			Token tok = parser->current_token;
			if (tok.text) tok.text = strdup(tok.text);
			match(parser, TOKEN_STRING);

			char* s = tok.text ? tok.text : "";
			int len = strlen(s);
			int has_interp = 0;
			for (int i = 0; i < len - 1; i++) {
				if (s[i] == '$' && s[i+1] == '{') { has_interp = 1; break; }
			}

			if (!has_interp) {
				primary_node = create_ast_node(AST_LITERAL, tok);
				break;
			}

			ASTNode** parts = NULL;
			int parts_count = 0;

			int i = 0;
			while (i < len) {
				if (s[i] == '$' && i + 1 < len && s[i+1] == '{') {
					int j = i + 2;
					while (j < len && s[j] != '}') j++;
					if (j >= len) {
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

					Lexer* sublexer = create_lexer(expr_txt);
					Parser subparser;
					init_parser(&subparser, sublexer);
					ASTNode* expr_node = parse_expression(&subparser);
					free_parser(&subparser);
					free(sublexer);
					free(expr_txt);

					if (!expr_node) {
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
			if (parts_count == 1) { primary_node = parts[0]; break; }

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
			primary_node = left;
			break;
		}

		case TOKEN_LPAREN: {
			match(parser, TOKEN_LPAREN);
			if (parser->current_token.type == TOKEN_INT || parser->current_token.type == TOKEN_FLOAT ||
				parser->current_token.type == TOKEN_STRING || parser->current_token.type == TOKEN_BOOLEAN ||
				parser->current_token.type == TOKEN_CHAR) {
				Token type_token = parser->current_token;
				if (type_token.text) type_token.text = strdup(type_token.text);
				match(parser, parser->current_token.type);
				if (!expect(parser, TOKEN_RPAREN, PARSER_EXPECTED, "')' after cast type", parser->current_token.text)) return NULL;

				ASTNode* expr = parse_unary(parser);
				if (!expr) {
					set_error(parser, PARSER_EXPECTED, "expression after cast", parser->current_token.text);
					return NULL;
				}

				ASTNode* type_node = create_ast_node(AST_TYPE, type_token);
				ASTNode* cast_node = create_ast_node(AST_CAST_EXPR, (Token){0});
				cast_node->child_count = 2;
				cast_node->children = malloc(sizeof(ASTNode*) * 2);
				cast_node->children[0] = type_node;
				cast_node->children[1] = expr;
				primary_node = cast_node;
				break;
			}

			// Otherwise, normal parenthesized expression
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
			primary_node = expr;
			break;
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
			primary_node = array_node;
			break;
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
			primary_node = node;
			break;
		}

		default:
			set_error(parser, PARSER_UNEXPECTED_TOKEN, parser->current_token.text);
			return NULL;
	}

	if (primary_node && (parser->current_token.type == TOKEN_INCREMENT || parser->current_token.type == TOKEN_DECREMENT)) {
		Token inc_op_token = parser->current_token;
		if (inc_op_token.text) inc_op_token.text = strdup(inc_op_token.text);
		match(parser, parser->current_token.type);
		ASTNode* inc_op_node = create_ast_node(AST_OPERATORS, inc_op_token);
		ASTNode* increment_node = create_ast_node(AST_INCREMENT_EXPR, (Token){0});
		increment_node->child_count = 2;
		increment_node->children = malloc(sizeof(ASTNode*) * 2);
		increment_node->children[0] = primary_node;
		increment_node->children[1] = inc_op_node;
		return increment_node;
	}

	return primary_node;
}
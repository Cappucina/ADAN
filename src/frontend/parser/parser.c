#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "parser_utils.h"
#include "../../helper.h"
#include "../../stm.h"

Parser* parser_init(Scanner* scanner)
{
	Parser* parser = (Parser*)malloc(sizeof(Parser));
	if (!parser)
	{
		printf("Failed to allocate memory for Parser! (Error)\n");
		return NULL;
	}

	parser->scanner = scanner;
	parser->symbol_table_stack = sts_init();
	parser->error_count = 0;
	parser->panic = false;
	parser->token_position = 0;
	parser->scope_depth = 0;
	parser->recovery_mode = false;
	parser->current = NULL;
	parser->ahead1 = NULL;
	parser->ahead2 = NULL;

	advance_token(parser);
	advance_token(parser);
	advance_token(parser);

	return parser;
}

void parser_free(Parser* parser)
{
	if (!parser)
		return;

	if (parser->current)
	{
		free(parser->current->lexeme);
		free(parser->current);
	}
	if (parser->ahead1)
	{
		free(parser->ahead1->lexeme);
		free(parser->ahead1);
	}
	if (parser->ahead2)
	{
		free(parser->ahead2->lexeme);
		free(parser->ahead2);
	}

	sts_free(parser->symbol_table_stack);
	free(parser);
}

// Forward declaration stuff

static bool match(Parser* parser, TokenType type);

static bool consume(Parser* parser, TokenType type, const char* error_message);

static void synchronize(Parser* parser);

static void parse_statement(Parser* parser);

static void parse_function_declaration(Parser* parser);

static void parse_variable_declaration(Parser* parser);

static void parse_import_statement(Parser* parser);

static void parse_expression(Parser* parser);

static void parse_primary(Parser* parser);

static void parse_call(Parser* parser);

static void parse_call_statement(Parser* parser);

static void parse_type(Parser* parser);

static void parse_parameter_list(Parser* parser);

static void parse_parameter(Parser* parser);

static void parse_block(Parser* parser);

// Helper implementations

static bool match(Parser* parser, TokenType type)
{
	return peek_current(parser) && peek_current(parser)->type == type;
}

static bool consume(Parser* parser, TokenType type, const char* error_message)
{
	if (match(parser, type))
	{
		advance_token(parser);
		return true;
	}

	error_expected(parser, error_message);
	return false;
}

static void synchronize(Parser* parser)
{
	advance_token(parser);

	while (peek_current(parser) && peek_current(parser)->type != TOKEN_SEMICOLON)
	{
		switch (peek_current(parser)->type)
		{
		case TOKEN_FUN:
		case TOKEN_IMPORT:
		case TOKEN_SET:
		case TOKEN_RBRACE:
		case TOKEN_EOF:
			return;
		default:
			break;
		}

		advance_token(parser);
	}

	if (peek_current(parser) && peek_current(parser)->type == TOKEN_SEMICOLON)
		advance_token(parser);
}

static void parse_type(Parser* parser)
{
	if (match(parser, TOKEN_STRING_TYPE) || match(parser, TOKEN_I32_TYPE) ||
	    match(parser, TOKEN_I64_TYPE) || match(parser, TOKEN_U32_TYPE) ||
	    match(parser, TOKEN_U64_TYPE))
	{
		advance_token(parser);
	}
	else
	{
		error_expected(parser, "type");
		enter_recovery_mode(parser);
		synchronize(parser);
	}
}

static void parse_primary(Parser* parser)
{
	if (match(parser, TOKEN_IDENT) || match(parser, TOKEN_STRING) ||
	    match(parser, TOKEN_NUMBER))
	{
		advance_token(parser);
	}
	else
	{
		error_expected(parser, "primary expression");
		enter_recovery_mode(parser);
		synchronize(parser);
	}
}

static void parse_call(Parser* parser)
{
	consume(parser, TOKEN_IDENT, "Expected function name for call expression.");
	consume(parser, TOKEN_LPAREN, "Expected '(' after function name in call expression.");

	if (!match(parser, TOKEN_RPAREN))
	{
		parse_expression(parser);
		while (match(parser, TOKEN_COMMA))
		{
			advance_token(parser);
			parse_expression(parser);
		}
	}

	consume(parser, TOKEN_RPAREN, "Expected ')' after arguments in call expression.");
}

static void parse_call_statement(Parser* parser)
{
	char* name = clone_string(peek_current(parser)->lexeme);

	parse_call(parser);
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after call statement.");

	if (name)
		parser_use_symbol(parser, name);
	free(name);
}

static void parse_expression(Parser* parser)
{
	if (match(parser, TOKEN_IDENT) && peek_lookahead1(parser)->type == TOKEN_LPAREN)
		parse_call(parser);
	else if (match(parser, TOKEN_IDENT) || match(parser, TOKEN_STRING) ||
	         match(parser, TOKEN_NUMBER))
		parse_primary(parser);
	else
	{
		error_expected(parser, "expression");
		enter_recovery_mode(parser);
		synchronize(parser);
	}
}

static void parse_parameter(Parser* parser)
{
	consume(parser, TOKEN_IDENT, "Expected parameter name.");
	consume(parser, TOKEN_COLON, "Expected ':' after parameter name.");
	parse_type(parser);
}

static void parse_parameter_list(Parser* parser)
{
	if (!match(parser, TOKEN_RPAREN))
	{
		parse_parameter(parser);
		while (match(parser, TOKEN_COMMA))
		{
			advance_token(parser);
			parse_parameter(parser);
		}
	}
}

static void parse_block(Parser* parser)
{
	parser_enter_scope(parser);
	consume(parser, TOKEN_LBRACE, "Expected '{' to start block.");

	while (!match(parser, TOKEN_RBRACE) && !match(parser, TOKEN_EOF) && !parser->recovery_mode)
	{
		parse_statement(parser);
	}

	if (parser->recovery_mode)
		exit_recovery_mode(parser);

	consume(parser, TOKEN_RBRACE, "Expected '}' to end block.");
	parser_exit_scope(parser);
}

static void parse_import_statement(Parser* parser)
{
	consume(parser, TOKEN_IMPORT, "Expected 'import' keyword for import statement.");
	consume(parser, TOKEN_STRING, "Expected string literal after 'import' keyword.");
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after import statement.");
}

static void parse_variable_declaration(Parser* parser)
{
	consume(parser, TOKEN_SET, "Expected 'set' keyword for variable declaration.");

	char* name = clone_string(peek_current(parser)->lexeme);
	consume(parser, TOKEN_IDENT, "Expected variable name after 'set' keyword.");

	consume(parser, TOKEN_COLON, "Expected ':' after variable name.");

	char* type = clone_string(peek_current(parser)->lexeme);
	parse_type(parser);

	consume(parser, TOKEN_EQUALS, "Expected '=' after variable type.");
	parse_expression(parser);
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration.");

	if (name && type && !parser->recovery_mode)
		parser_declare_variable(parser, name, type, 0);

	free(name);
	free(type);
}

static void parse_function_declaration(Parser* parser)
{
	consume(parser, TOKEN_FUN, "Expected 'fun' keyword for function declaration.");

	char* name = clone_string(peek_current(parser)->lexeme);
	consume(parser, TOKEN_IDENT, "Expected function name after 'fun' keyword.");

	consume(parser, TOKEN_LPAREN, "Expected '(' after function name.");
	parse_parameter_list(parser);
	consume(parser, TOKEN_RPAREN, "Expected ')' after parameter list.");

	consume(parser, TOKEN_COLON, "Expected ':' after parameter list for return type.");

	char* return_type = clone_string(peek_current(parser)->lexeme);
	parse_type(parser);

	if (name && return_type && !parser->recovery_mode)
		parser_declare_function(parser, name, return_type);

	free(name);
	free(return_type);

	parse_block(parser);
}

static void parse_statement(Parser* parser)
{
	if (parser->recovery_mode)
		return;

	switch (peek_current(parser)->type)
	{
	case TOKEN_FUN:
		parse_function_declaration(parser);
		break;
	case TOKEN_IMPORT:
		parse_import_statement(parser);
		break;
	case TOKEN_SET:
		parse_variable_declaration(parser);
		break;
	case TOKEN_IDENT:
		parse_call_statement(parser);
		break;
	default:
		error_expected(parser, "statement");
		enter_recovery_mode(parser);
		synchronize(parser);
		break;
	}
}

// Primary parsing function

void parser_parse_program(Parser* parser)
{
	while (peek_current(parser) && peek_current(parser)->type != TOKEN_EOF)
	{
		parse_statement(parser);

		if (parser->recovery_mode)
			exit_recovery_mode(parser);
	}
}
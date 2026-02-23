#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "parser_utils.h"
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
		free(parser->current);
	if (parser->ahead1)
		free(parser->ahead1);
	if (parser->ahead2)
		free(parser->ahead2);

	sts_free(parser->symbol_table_stack);
	free(parser);
}

// Actual parser implementation

/**
 * 
 * @todo Implement the following functions:
 * 
 * - void parser_parse_program(Parser* parser);
 * - static void parse_statement(Parser* parser);
 * - static void parse_function_declaration(Parser* parser);
 * - static void parse_variable_declaration(Parser* parser);
 * - static void parse_import_statement(Parser* parser);
 * - static void parse_expression(Parser* parser);
 * - static void parse_primary(Parser* parser);
 * - static void parse_call(Parser* parser);
 * - static void parse_type(Parser* parser);
 * - static void parse_parameter_list(Parser* parser);
 * - static void parse_parameter(Parser* parser);
 * - static void parse_function_body(Parser* parser);
 * - static void parse_block(Parser* parser);
 * 
 */

static void parse_statement(Parser* parser)
{
	
}

static void parse_function_declaration(Parser* parser)
{

}

static void parse_variable_declaration(Parser* parser)
{

}

static void parse_import_statement(Parser* parser)
{

}

static void parse_expression(Parser* parser)
{

}

static void parse_primary(Parser* parser)
{

}

static void parse_call(Parser* parser)
{

}

static void parse_type(Parser* parser)
{

}

static void parse_parameter_list(Parser* parser)
{

}

static void parse_parameter(Parser* parser)
{

}

static void parse_function_body(Parser* parser)
{

}

static void parse_block(Parser* parser)
{

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

static bool match(Parser* parser, TokenType type)
{
	return peek_current(parser) && peek_current(parser)->type == type;
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
			return;
		default:
			break;
		}

		advance_token(parser);
	}

	if (peek_current(parser) && peek_current(parser)->type == TOKEN_SEMICOLON)
	{
		advance_token(parser);
	}
}

//  Primary parsing function

void parser_parse_program(Parser* parser)
{
    
}
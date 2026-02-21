#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "../scanner/scanner.h"
#include "../../stm.h"

Parser* parser_init(Scanner* scanner)
{
	Parser* parser = (Parser*)malloc(sizeof(Parser));
	if (!parser)
	{
		fprintf(stderr, "Failed to allocate memory for Parser.\n");
		exit(EXIT_FAILURE);
	}
	parser->scanner = scanner;
	parser->current = NULL;
	parser->ahead1 = NULL;
	parser->ahead2 = NULL;
	parser->symbol_table_stack = sts_init();
	parser->error_count = 0;
	parser->panic = false;
	parser->token_position = 0;
	parser->scope_depth = 0;
	parser->recovery_mode = false;
	parser->current = scan_next_token(scanner);
	parser->ahead1 = scan_next_token(scanner);
	parser->ahead2 = scan_next_token(scanner);
	return parser;
}

void parser_free(Parser* parser)
{
	if (parser)
	{
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
	else
	{
		fprintf(stderr, "Attempted to free a NULL Parser pointer.\n");
	}
}

// Symbol Table stuff

void parser_declare_variable(Parser* parser, const char* name, const char* type, unsigned int size)
{
	if (!parser)
	{
		printf("An empty Parser pointer was provided; could not declare variable. (Error)");
		return;
	}

	SymbolEntry* var_entry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
	if (!var_entry)
	{
		printf(
		    "Failed to allocate enough memory for a new SymbolEntry in the parser. "
		    "(Error)");
		return;
	}

	// @todo implement later
}

void parser_declare_function(Parser* parser, const char* name, const char* return_type)
{
}

void parser_use_symbol(Parser* parser, const char* name)
{
}

void parser_enter_scope(Parser* parser)
{
}

void parser_exit_scope(Parser* parser)
{
}

bool parser_symbol_exists(Parser* parser, const char* name)
{
}

// Token lookahead stuff

void advance_token(Parser* parser)
{
}

Token* peek_current(Parser* parser)
{
}

Token* peek_lookahead1(Parser* parser)
{
}

Token* peek_lookahead2(Parser* parser)
{
}

bool match_current(Parser* parser, TokenType type)
{
}

// Error handling and recovery stuff

void error_expected(Parser* parser, const char* expected)
{
}

void error_undefined_symbol(Parser* parser, const char* name)
{
}

void enter_recovery_mode(Parser* parser)
{
}

void exit_recovery_mode(Parser* parser)
{
}
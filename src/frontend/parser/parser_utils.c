#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "parser_utils.h"
#include "../scanner/scanner.h"
#include "../../stm.h"

// Symbol Table stuff

void parser_declare_variable(Parser* parser, const char* name, const char* type, unsigned int size)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return;
	}

	char line_buffer[32];
	snprintf(line_buffer, sizeof(line_buffer), "%zu",
	         parser->current ? parser->current->line : 0);

	stm_insert(parser->symbol_table_stack->current_scope, (char*)name, (char*)type, size,
	           line_buffer,  // decl line
	           NULL,         // usage line
	           NULL          // address
	);
	printf("Declared variable '%s' of type '%s' with size %u in scope level %d. (Info)\n", name,
	       type, size, parser->scope_depth);
}

void parser_declare_function(Parser* parser, const char* name, const char* return_type)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare function. (Error)\n");
		return;
	}

	char line_buffer[32];
	snprintf(line_buffer, sizeof(line_buffer), "%zu",
	         parser->current ? parser->current->line : 0);

	stm_insert(parser->symbol_table_stack->current_scope, (char*)name, (char*)return_type,
	           0,            // functions have size 0
	           line_buffer,  // decl line
	           NULL,         // usage line
	           NULL          // address
	);
	printf("Declared function '%s' with return type '%s' in scope level %d. (Info)\n", name,
	       return_type, parser->scope_depth);
}

void parser_use_symbol(Parser* parser, const char* name)
{
	if (!parser)
	{
		fprintf(stderr,
		        "An empty Parser pointer was provided; could not use symbol. (Error)\n");
		return;
	}

	if (parser->allow_undefined_symbols)
	{
		return;
	}

	SymbolEntry* entry = stm_lookup(parser->symbol_table_stack->current_scope, name);
	if (!entry)
	{
		fprintf(stderr, "Undefined symbol '%s' used. (Error)\n", name);
		parser->error_count++;
	}
}

void parser_enter_scope(Parser* parser)
{
	if (!parser)
	{
		fprintf(stderr,
		        "An empty Parser pointer was provided; could not enter scope. (Error)\n");
		return;
	}

	sts_push_scope(parser->symbol_table_stack);
	parser->scope_depth++;
	printf("Entering scope level %d. (Info)\n", parser->scope_depth);
}

void parser_exit_scope(Parser* parser)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return;
	}

	sts_pop_scope(parser->symbol_table_stack);
	parser->scope_depth--;
	printf("Exiting scope level %d. (Info)\n", parser->scope_depth);
}

bool parser_symbol_exists(Parser* parser, const char* name)
{
	if (!parser)
	{
		fprintf(stderr,
		        "An empty Parser pointer was provided; could not check symbol existence. "
		        "(Error)\n");
		return false;
	}

	SymbolEntry* entry = stm_lookup_local(parser->symbol_table_stack->current_scope, name);
	if (!entry)
	{
		printf("Symbol '%s' does not exist in the current scope. (Info)\n", name);
	}
	return entry != NULL;
}

// Token lookahead stuff

void advance_token(Parser* parser)
{
	if (!parser)
	{
		return;
	}

	if (parser->current)
	{
		free(parser->current->lexeme);
	}
	free(parser->current);

	parser->current = parser->ahead1;
	parser->ahead1 = parser->ahead2;
	parser->ahead2 = scan_next_token(parser->scanner);
	parser->token_position++;
}

Token* peek_current(Parser* parser)
{
	if (!parser)
	{
		fprintf(stderr,
		        "An empty Parser pointer was provided; could not peek current token. "
		        "(Error)\n");
		return NULL;
	}

	return parser->current;
}

Token* peek_lookahead1(Parser* parser)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return NULL;
	}

	return parser->ahead1;
}

Token* peek_lookahead2(Parser* parser)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return NULL;
	}

	return parser->ahead2;
}

bool match_current(Parser* parser, TokenType type)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return false;
	}

	printf("Matching current token '%s' against expected type '%d'. (Info)\n",
	       parser->current ? parser->current->lexeme : "NULL", type);
	return parser->current && parser->current->type == type;
}

// Error handling and recovery stuff

void error_expected(Parser* parser, const char* expected)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return;
	}

	fprintf(stderr, "Expected %s but found '%s'. (Error)\n", expected,
	        parser->current ? parser->current->lexeme : "NULL");
	parser->error_count++;
}

void error_undefined_symbol(Parser* parser, const char* name)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return;
	}

	SymbolEntry* entry = stm_lookup_local(parser->symbol_table_stack->current_scope, name);
	if (!entry)
	{
		fprintf(stderr, "Undefined symbol '%s' used. (Error)\n", name);
		parser->error_count++;
	}
}

void enter_recovery_mode(Parser* parser)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return;
	}

	parser->recovery_mode = true;
	fprintf(stderr, "Critical error found during parsing; entering recovery mode. (Warning)\n");
}

void exit_recovery_mode(Parser* parser)
{
	if (!parser)
	{
		fprintf(
		    stderr,
		    "An empty Parser pointer was provided; could not declare variable. (Error)\n");
		return;
	}

	parser->recovery_mode = false;
	printf("Exiting recovery mode. (Info)\n");
}
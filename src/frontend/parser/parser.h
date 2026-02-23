#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#include "../../stm.h"
#include "../scanner/scanner.h"
#include "../scanner/token.h"

typedef struct Parser
{
	// Tokens (LL(2) require	s 2 lookahead tokens).
	Token* current;
	Token* ahead1;
	Token* ahead2;
	Scanner* scanner;
	SymbolTableStack* symbol_table_stack;  // The Symbol Table Stack contains only Symbol Table
	                                       // Managers, each manager refers to its own scope.
	                                       // Refer to stm.h for more details.
	int error_count;
	bool panic;  // Reserved for critical errors that would break all future parsing.
	int token_position;
	int scope_depth;
	bool recovery_mode;
} Parser;

Parser* parser_init(Scanner* scanner);

void parser_free(Parser* parser);

void parser_parse_program(Parser* parser);

#endif
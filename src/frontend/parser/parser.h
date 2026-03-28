#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

#include "../../stm.h"
#include "../scanner/scanner.h"
#include "../scanner/token.h"
#include "../ast/tree.h"

typedef struct ParserTypeAlias
{
	char* name;
	char* resolved_type;
	struct ParserTypeAlias* next;
} ParserTypeAlias;

typedef struct Parser
{
	Token* current;
	Token* ahead1;
	Token* ahead2;
	Scanner* scanner;
	SymbolTableStack* symbol_table_stack;
	int error_count;
	bool panic;
	int token_position;
	int scope_depth;
	bool recovery_mode;
	bool allow_undefined_symbols;
	ParserTypeAlias* type_aliases;
} Parser;

Parser* parser_init(Scanner* scanner);

void parser_free(Parser* parser);

ASTNode* parser_parse_program(Parser* parser);

#endif
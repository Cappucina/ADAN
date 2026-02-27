#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "parser_utils.h"
#include "../../helper.h"
#include "../../stm.h"
#include "../ast/tree.h"

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
	parser->allow_undefined_symbols = false;
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

static ASTNode* parse_statement(Parser* parser);

static ASTNode* parse_function_declaration(Parser* parser);

static ASTNode* parse_variable_declaration(Parser* parser);

static ASTNode* parse_import_statement(Parser* parser);

static ASTNode* parse_expression(Parser* parser);

static ASTNode* parse_primary(Parser* parser);

static ASTNode* parse_call(Parser* parser);

static ASTNode* parse_call_statement(Parser* parser);

static ASTNode* parse_type(Parser* parser);

static ASTNode** parse_parameter_list(Parser* parser, size_t* count);

static ASTNode* parse_parameter(Parser* parser);

static ASTNode* parse_block(Parser* parser);

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
			case TOKEN_RETURN:
			case TOKEN_RBRACE:
			case TOKEN_EOF:
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

static ASTNode* parse_type(Parser* parser)
{
	if (match(parser, TOKEN_STRING_TYPE) || match(parser, TOKEN_I32_TYPE) ||
	    match(parser, TOKEN_I64_TYPE) || match(parser, TOKEN_U32_TYPE) ||
	    match(parser, TOKEN_U64_TYPE) || match(parser, TOKEN_VOID_TYPE))
	{
		size_t tok_line = peek_current(parser)->line;
		size_t tok_column = peek_current(parser)->column;
		char* type_name = clone_string(peek_current(parser)->lexeme,
		                               strlen(peek_current(parser)->lexeme));
		advance_token(parser);
		ASTNode* node = ast_create_type(type_name, tok_line, tok_column);
		free(type_name);
		return node;
	}
	else
	{
		error_expected(parser, "type");
		enter_recovery_mode(parser);
		synchronize(parser);
		return NULL;
	}
}

static ASTNode* parse_primary(Parser* parser)
{
	if (match(parser, TOKEN_IDENT))
	{
		size_t tok_line = peek_current(parser)->line;
		size_t tok_column = peek_current(parser)->column;
		char* name = clone_string(peek_current(parser)->lexeme,
		                          strlen(peek_current(parser)->lexeme));
		advance_token(parser);
		ASTNode* node = ast_create_identifier(name, tok_line, tok_column);
		free(name);
		return node;
	}
	else if (match(parser, TOKEN_STRING))
	{
		size_t tok_line = peek_current(parser)->line;
		size_t tok_column = peek_current(parser)->column;
		char* value = clone_string(peek_current(parser)->lexeme,
		                           strlen(peek_current(parser)->lexeme));
		advance_token(parser);
		ASTNode* node = ast_create_string_literal(value, tok_line, tok_column);
		free(value);
		return node;
	}
	else if (match(parser, TOKEN_NUMBER))
	{
		size_t tok_line = peek_current(parser)->line;
		size_t tok_column = peek_current(parser)->column;
		char* value = clone_string(peek_current(parser)->lexeme,
		                           strlen(peek_current(parser)->lexeme));
		advance_token(parser);
		ASTNode* node = ast_create_number_literal(value, tok_line, tok_column);
		free(value);
		return node;
	}
	else
	{
		error_expected(parser, "primary expression");
		enter_recovery_mode(parser);
		synchronize(parser);
		return NULL;
	}
}

static ASTNode* parse_call(Parser* parser)
{
	char* callee =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_IDENT, "Expected function name for call expression.");
	consume(parser, TOKEN_LPAREN, "Expected '(' after function name in call expression.");

	ASTNode** args = NULL;
	size_t arg_count = 0;
	size_t arg_capacity = 0;

	if (!match(parser, TOKEN_RPAREN))
	{
		arg_capacity = 4;
		args = malloc(sizeof(ASTNode*) * arg_capacity);
		args[arg_count++] = parse_expression(parser);

		while (match(parser, TOKEN_COMMA))
		{
			advance_token(parser);
			if (arg_count >= arg_capacity)
			{
				arg_capacity *= 2;
				args = realloc(args, sizeof(ASTNode*) * arg_capacity);
			}
			args[arg_count++] = parse_expression(parser);
		}
	}

	consume(parser, TOKEN_RPAREN, "Expected ')' after arguments in call expression.");
	ASTNode* call = ast_create_call(callee, args, arg_count, peek_lookahead1(parser)->line,
	                                peek_lookahead1(parser)->column);
	free(callee);
	return call;
}

static ASTNode* parse_call_statement(Parser* parser)
{
	char* name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));

	ASTNode* call = parse_call(parser);

	size_t stmt_line = call ? call->line : peek_current(parser)->line;
	size_t stmt_column = call ? call->column : peek_current(parser)->column;

	consume(parser, TOKEN_SEMICOLON, "Expected ';' after call statement.");

	if (name)
	{
		parser_use_symbol(parser, name);
	}
	free(name);

	if (call)
	{
		return ast_create_expression_statement(call, stmt_line, stmt_column);
	}
	return NULL;
}

static ASTNode* parse_expression(Parser* parser)
{
	if (match(parser, TOKEN_IDENT) && peek_lookahead1(parser)->type == TOKEN_LPAREN)
		return parse_call(parser);
	else if (match(parser, TOKEN_IDENT) || match(parser, TOKEN_STRING) ||
	         match(parser, TOKEN_NUMBER))
		return parse_primary(parser);
	else
	{
		error_expected(parser, "expression");
		enter_recovery_mode(parser);
		synchronize(parser);
		return NULL;
	}
}

static ASTNode* parse_parameter(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	char* name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_IDENT, "Expected parameter name.");
	consume(parser, TOKEN_COLON, "Expected ':' after parameter name.");
	ASTNode* type = parse_type(parser);
	ASTNode* param = ast_create_parameter(name, type, tok_line, tok_column);
	free(name);
	return param;
}

static ASTNode** parse_parameter_list(Parser* parser, size_t* count)
{
	*count = 0;
	ASTNode** params = NULL;
	size_t capacity = 0;

	if (!match(parser, TOKEN_RPAREN))
	{
		capacity = 4;
		params = malloc(sizeof(ASTNode*) * capacity);
		params[(*count)++] = parse_parameter(parser);

		while (match(parser, TOKEN_COMMA))
		{
			advance_token(parser);
			if (*count >= capacity)
			{
				capacity *= 2;
				params = realloc(params, sizeof(ASTNode*) * capacity);
			}
			params[(*count)++] = parse_parameter(parser);
		}
	}

	return params;
}

static ASTNode* parse_block(Parser* parser)
{
	parser_enter_scope(parser);
	consume(parser, TOKEN_LBRACE, "Expected '{' to start block.");

	ASTNode** statements = NULL;
	size_t count = 0;
	size_t capacity = 0;

	while (!match(parser, TOKEN_RBRACE) && !match(parser, TOKEN_EOF) && !parser->recovery_mode)
	{
		if (count >= capacity)
		{
			capacity = capacity == 0 ? 4 : capacity * 2;
			statements = realloc(statements, sizeof(ASTNode*) * capacity);
		}
		statements[count++] = parse_statement(parser);
	}

	if (parser->recovery_mode)
	{
		exit_recovery_mode(parser);
	}

	consume(parser, TOKEN_RBRACE, "Expected '}' to end block.");
	parser_exit_scope(parser);
	return ast_create_block(statements, count, peek_current(parser)->line,
	                        peek_current(parser)->column);
}

static ASTNode* parse_import_statement(Parser* parser)
{
	consume(parser, TOKEN_IMPORT, "Expected 'import' keyword for import statement.");
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	char* path =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_STRING, "Expected string literal after 'import' keyword.");
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after import statement.");
	ASTNode* import = ast_create_import(path, tok_line, tok_column);
	free(path);
	return import;
}

static ASTNode* parse_variable_declaration(Parser* parser)
{
	consume(parser, TOKEN_SET, "Expected 'set' keyword for variable declaration.");

	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	char* name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_IDENT, "Expected variable name after 'set' keyword.");

	consume(parser, TOKEN_COLON, "Expected ':' after variable name.");

	char* type_name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	ASTNode* type = parse_type(parser);

	consume(parser, TOKEN_EQUALS, "Expected '=' after variable type.");
	ASTNode* initializer = parse_expression(parser);
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration.");

	if (name && type_name && !parser->recovery_mode)
	{
		parser_declare_variable(parser, name, type_name, 0);
	}
	free(type_name);

	ASTNode* var_decl =
	    ast_create_variable_declaration(name, type, initializer, tok_line, tok_column);
	free(name);
	return var_decl;
}

static ASTNode* parse_function_declaration(Parser* parser)
{
	consume(parser, TOKEN_FUN, "Expected 'fun' keyword for function declaration.");

	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	char* name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_IDENT, "Expected function name after 'fun' keyword.");

	consume(parser, TOKEN_LPAREN, "Expected '(' after function name.");
	size_t param_count = 0;
	ASTNode** params = parse_parameter_list(parser, &param_count);
	consume(parser, TOKEN_RPAREN, "Expected ')' after parameter list.");

	consume(parser, TOKEN_COLON, "Expected ':' after parameter list for return type.");

	char* return_type_name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	ASTNode* return_type = parse_type(parser);

	if (name && return_type_name && !parser->recovery_mode)
	{
		parser_declare_function(parser, name, return_type_name);
	}
	free(return_type_name);

	ASTNode* body = parse_block(parser);
	ASTNode* func_decl = ast_create_function_declaration(name, params, param_count, return_type,
	                                                     body, tok_line, tok_column);
	free(name);
	return func_decl;
}

static ASTNode* parse_return_statement(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	consume(parser, TOKEN_RETURN, "Expected 'return' keyword.");

	ASTNode* expr = NULL;
	if (!match(parser, TOKEN_SEMICOLON))
	{
		expr = parse_expression(parser);
	}
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after return statement.");
	return ast_create_return(expr, tok_line, tok_column);
}

static ASTNode* parse_statement(Parser* parser)
{
	if (parser->recovery_mode)
	{
		return NULL;
	}
	switch (peek_current(parser)->type)
	{
		case TOKEN_FUN:
			return parse_function_declaration(parser);
		case TOKEN_IMPORT:
			return parse_import_statement(parser);
		case TOKEN_SET:
			return parse_variable_declaration(parser);
		case TOKEN_RETURN:
			return parse_return_statement(parser);
		case TOKEN_IDENT:
			return parse_call_statement(parser);
		default:
			error_expected(parser, "statement");
			enter_recovery_mode(parser);
			synchronize(parser);
			return NULL;
	}
}

// Primary parsing function

ASTNode* parser_parse_program(Parser* parser)
{
	ASTNode** decls = NULL;
	size_t count = 0;
	size_t capacity = 0;

	while (peek_current(parser) && peek_current(parser)->type != TOKEN_EOF)
	{
		if (count >= capacity)
		{
			capacity = capacity == 0 ? 8 : capacity * 2;
			decls = realloc(decls, sizeof(ASTNode*) * capacity);
		}

		ASTNode* stmt = parse_statement(parser);
		if (stmt)
		{
			decls[count++] = stmt;
		}

		if (parser->recovery_mode)
		{
			exit_recovery_mode(parser);
		}
	}

	return ast_create_program(decls, count, peek_current(parser)->line,
	                          peek_current(parser)->column);
}
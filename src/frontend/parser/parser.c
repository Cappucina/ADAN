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
	{
		return;
	}

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

static ASTNode* parse_if_statement(Parser* parser);

static ASTNode* parse_variable_declaration(Parser* parser);

static ASTNode* parse_import_statement(Parser* parser);

static ASTNode* parse_expression(Parser* parser);

static ASTNode* parse_or(Parser* parser);

static ASTNode* parse_and(Parser* parser);

static ASTNode* parse_not(Parser* parser);

static ASTNode* parse_comparison(Parser* parser);

static ASTNode* parse_additive(Parser* parser);

static ASTNode* parse_multiplicative(Parser* parser);

static ASTNode* parse_power(Parser* parser);

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
	    match(parser, TOKEN_U64_TYPE) || match(parser, TOKEN_VOID_TYPE) ||
	    match(parser, TOKEN_F32_TYPE) || match(parser, TOKEN_F64_TYPE) ||
	    match(parser, TOKEN_BOOL_TYPE))
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
		if (peek_lookahead1(parser) && peek_lookahead1(parser)->type == TOKEN_LPAREN)
		{
			return parse_call(parser);
		}
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

		while (match(parser, TOKEN_INTERP_START))
		{
			advance_token(parser);
			ASTNode* expr = parse_expression(parser);
			consume(parser, TOKEN_INTERP_END,
			        "Expected '}' to close interpolated expression.");
			node = ast_create_binary_op("+", node, expr, tok_line, tok_column);

			if (match(parser, TOKEN_STRING))
			{
				char* next_str = clone_string(peek_current(parser)->lexeme,
				                              strlen(peek_current(parser)->lexeme));
				size_t next_line = peek_current(parser)->line;
				size_t next_col = peek_current(parser)->column;
				advance_token(parser);
				ASTNode* next_node =
				    ast_create_string_literal(next_str, next_line, next_col);
				free(next_str);
				node = ast_create_binary_op("+", node, next_node, tok_line,
				                            tok_column);
			}
		}

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
	else if (match(parser, TOKEN_TRUE) || match(parser, TOKEN_FALSE))
	{
		size_t tok_line = peek_current(parser)->line;
		size_t tok_column = peek_current(parser)->column;
		bool b_val = (peek_current(parser)->type == TOKEN_TRUE);
		advance_token(parser);
		return ast_create_boolean_literal(b_val, tok_line, tok_column);
	}
	else if (match(parser, TOKEN_LPAREN))
	{
		Token* la1 = peek_lookahead1(parser);
		Token* la2 = peek_lookahead2(parser);
		if (la1 && la2 && la2->type == TOKEN_RPAREN &&
		    (la1->type == TOKEN_STRING_TYPE || la1->type == TOKEN_I32_TYPE ||
		     la1->type == TOKEN_I64_TYPE || la1->type == TOKEN_U32_TYPE ||
		     la1->type == TOKEN_U64_TYPE || la1->type == TOKEN_VOID_TYPE ||
		     la1->type == TOKEN_F32_TYPE || la1->type == TOKEN_F64_TYPE ||
		     la1->type == TOKEN_BOOL_TYPE))
		{
			size_t cast_line = peek_current(parser)->line;
			size_t cast_col = peek_current(parser)->column;
			advance_token(parser);  // consume '('
			ASTNode* target_type = parse_type(parser);
			consume(parser, TOKEN_RPAREN, "Expected ')' after cast type.");
			ASTNode* expr = parse_primary(parser);
			return ast_create_cast(target_type, expr, cast_line, cast_col);
		}
		advance_token(parser);
		ASTNode* expr = parse_expression(parser);
		consume(parser, TOKEN_RPAREN, "Expected ')' after grouped expression.");
		return expr;
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
	return parse_or(parser);
}

static ASTNode* parse_or(Parser* parser)
{
	ASTNode* left = parse_and(parser);

	while (match(parser, TOKEN_OR))
	{
		size_t op_line = peek_current(parser)->line;
		size_t op_col = peek_current(parser)->column;
		advance_token(parser);
		ASTNode* right = parse_and(parser);
		if (!right)
			return left;
		left = ast_create_binary_op("or", left, right, op_line, op_col);
	}
	return left;
}

static ASTNode* parse_and(Parser* parser)
{
	ASTNode* left = parse_not(parser);

	while (match(parser, TOKEN_AND))
	{
		size_t op_line = peek_current(parser)->line;
		size_t op_col = peek_current(parser)->column;
		advance_token(parser);
		ASTNode* right = parse_not(parser);
		if (!right)
			return left;
		left = ast_create_binary_op("and", left, right, op_line, op_col);
	}
	return left;
}

static ASTNode* parse_not(Parser* parser)
{
	if (match(parser, TOKEN_NOT))
	{
		size_t op_line = peek_current(parser)->line;
		size_t op_col = peek_current(parser)->column;
		advance_token(parser);
		ASTNode* right = parse_comparison(parser);
		if (!right)
			return NULL;
		// A NOT operator is essentially a bitwise or logical inversion. Wait, ADAN uses
		// `not` right? Can represented as binary op with NULL left, or unary. Our binary op
		// supports left and right. For unary, usually left is NULL.
		return ast_create_binary_op("not", NULL, right, op_line, op_col);
	}
	return parse_comparison(parser);
}

static ASTNode* parse_comparison(Parser* parser)
{
	ASTNode* left = parse_additive(parser);

	while (match(parser, TOKEN_EQUALS_EQUALS) || match(parser, TOKEN_BANG_EQUALS) ||
	       match(parser, TOKEN_LESS) || match(parser, TOKEN_LESS_EQUAL) ||
	       match(parser, TOKEN_GREATER) || match(parser, TOKEN_GREATER_EQUAL))
	{
		size_t op_line = peek_current(parser)->line;
		size_t op_col = peek_current(parser)->column;
		char* op_str = clone_string(peek_current(parser)->lexeme,
		                            strlen(peek_current(parser)->lexeme));
		advance_token(parser);
		ASTNode* right = parse_additive(parser);
		if (!right)
		{
			free(op_str);
			return left;
		}
		left = ast_create_binary_op(op_str, left, right, op_line, op_col);
		free(op_str);
	}
	return left;
}

static ASTNode* parse_additive(Parser* parser)
{
	ASTNode* left = parse_multiplicative(parser);

	while (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS))
	{
		size_t op_line = peek_current(parser)->line;
		size_t op_col = peek_current(parser)->column;
		char op[2] = {peek_current(parser)->lexeme[0], '\0'};
		advance_token(parser);
		ASTNode* right = parse_multiplicative(parser);
		if (!right)
		{
			return left;
		}
		left = ast_create_binary_op(op, left, right, op_line, op_col);
	}
	return left;
}

static ASTNode* parse_multiplicative(Parser* parser)
{
	ASTNode* left = parse_power(parser);

	while (match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH) ||
	       match(parser, TOKEN_PERCENT))
	{
		size_t op_line = peek_current(parser)->line;
		size_t op_col = peek_current(parser)->column;
		char op[2] = {peek_current(parser)->lexeme[0], '\0'};
		advance_token(parser);
		ASTNode* right = parse_power(parser);
		if (!right)
		{
			return left;
		}
		left = ast_create_binary_op(op, left, right, op_line, op_col);
	}
	return left;
}

static ASTNode* parse_power(Parser* parser)
{
	ASTNode* base = parse_primary(parser);

	if (match(parser, TOKEN_CARET))
	{
		size_t op_line = peek_current(parser)->line;
		size_t op_col = peek_current(parser)->column;
		advance_token(parser);
		ASTNode* exp = parse_power(parser);
		if (!exp)
		{
			return base;
		}
		return ast_create_binary_op("^", base, exp, op_line, op_col);
	}
	return base;
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

	Token* after = peek_current(parser);
	bool is_plus_assign = after && after->type == TOKEN_PLUS_EQUALS;
	bool is_plain_assign = after && after->type == TOKEN_EQUALS;

	if (is_plus_assign || is_plain_assign)
	{
		advance_token(parser);  // consume '=' or '+='
		ASTNode* rhs = parse_expression(parser);
		consume(parser, TOKEN_SEMICOLON, "Expected ';' after assignment.");

		ASTNode* value = rhs;
		if (is_plus_assign && name && rhs)
		{
			ASTNode* lhs_id = ast_create_identifier(name, tok_line, tok_column);
			value = ast_create_binary_op("+", lhs_id, rhs, tok_line, tok_column);
		}

		ASTNode* assign = ast_create_assignment(name, value, tok_line, tok_column);
		free(name);
		return assign;
	}

	// Declaration: `set name: type = expr;`
	consume(parser, TOKEN_COLON, "Expected ':' after variable name.");

	if (!peek_current(parser)->lexeme)
	{
		error_expected(parser, "type name after ':'");
		enter_recovery_mode(parser);
		free(name);
		return NULL;
	}

	char* type_name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	printf("Parsing variable declaration for '%s' of type '%s'\n", name, type_name);
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

static ASTNode* parse_if_statement(Parser* parser)
{
	consume(parser, TOKEN_IF, "Expected 'if' keyword for if statement.");
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	ASTNode* condition = parse_expression(parser);
	ASTNode* then_branch = parse_block(parser);
	if (match(parser, TOKEN_ELSE))
	{
		// i think this properly handles `else if` since the else branch can be another if
		// statement with its own else? idk
		advance_token(parser);
		ASTNode* else_branch = parse_block(parser);
		return ast_create_if(condition, then_branch, else_branch, tok_line, tok_column);
	}
	return ast_create_if(condition, then_branch, NULL, tok_line, tok_column);
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
		case TOKEN_IF:
			return parse_if_statement(parser);
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
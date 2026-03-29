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
	parser->type_aliases = NULL;

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

	while (parser->type_aliases)
	{
		ParserTypeAlias* next = parser->type_aliases->next;
		free(parser->type_aliases->name);
		free(parser->type_aliases->resolved_type);
		free(parser->type_aliases);
		parser->type_aliases = next;
	}

	sts_free(parser->symbol_table_stack);
	free(parser);
}

static bool match(Parser* parser, TokenType type);

static bool consume(Parser* parser, TokenType type, const char* error_message);

static void synchronize(Parser* parser);

static ASTNode* parse_statement(Parser* parser);

static ASTNode* parse_function_declaration(Parser* parser);

static ASTNode* parse_if_statement(Parser* parser);

static ASTNode* parse_while_statement(Parser* parser);

static ASTNode* parse_for_statement(Parser* parser);

static ASTNode* parse_variable_declaration(Parser* parser);

static ASTNode* parse_type_declaration(Parser* parser);

static ASTNode* parse_import_statement(Parser* parser);

static ASTNode* parse_break_statement(Parser* parser);

static ASTNode* parse_continue_statement(Parser* parser);

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

static ASTNode* parse_identifier_statement(Parser* parser);

static ASTNode* parse_type(Parser* parser);

static ASTNode** parse_parameter_list(Parser* parser, size_t* count, bool* is_variadic,
                                      char** variadic_name, ASTNode** variadic_type);

static ASTNode* parse_parameter(Parser* parser);

static ASTNode* parse_block(Parser* parser);

static ASTNode* parse_object_literal(Parser* parser);

static ASTNode* parse_array_literal(Parser* parser);

static char* parse_type_name(Parser* parser);

static ASTNode** parse_call_arguments(Parser* parser, size_t* count);

static ASTNode* parse_postfix(Parser* parser);

static ASTNode* apply_postfix(Parser* parser, ASTNode* expr);

static bool parse_variadic_parameter(Parser* parser, char** variadic_name, ASTNode** variadic_type);

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
			case TOKEN_CONST:
			case TOKEN_TYPE:
			case TOKEN_RETURN:
			case TOKEN_BREAK:
			case TOKEN_CONTINUE:
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

static bool is_type_token(TokenType type)
{
	return type == TOKEN_STRING_TYPE || type == TOKEN_I32_TYPE || type == TOKEN_I64_TYPE ||
	       type == TOKEN_U32_TYPE || type == TOKEN_U64_TYPE || type == TOKEN_VOID_TYPE ||
	       type == TOKEN_F32_TYPE || type == TOKEN_F64_TYPE || type == TOKEN_BOOL_TYPE ||
	       type == TOKEN_I8_TYPE || type == TOKEN_U8_TYPE || type == TOKEN_ANY_TYPE;
}

static bool append_text(char** buffer, size_t* length, size_t* capacity, const char* text)
{
	if (!text)
	{
		return false;
	}

	size_t text_length = strlen(text);
	if (*length + text_length + 1 > *capacity)
	{
		size_t next_capacity = *capacity == 0 ? 32 : *capacity;
		while (*length + text_length + 1 > next_capacity)
		{
			next_capacity *= 2;
		}
		char* resized = realloc(*buffer, next_capacity);
		if (!resized)
		{
			return false;
		}
		*buffer = resized;
		*capacity = next_capacity;
	}

	memcpy(*buffer + *length, text, text_length);
	*length += text_length;
	(*buffer)[*length] = '\0';
	return true;
}

static bool starts_with_text(const char* text, const char* prefix)
{
	return text && prefix && strncmp(text, prefix, strlen(prefix)) == 0;
}

static bool derive_import_namespace(const char* raw_path, char* output, size_t output_size)
{
	if (!raw_path || !output || output_size == 0)
	{
		return false;
	}

	const char* start = raw_path;
	size_t length = strlen(raw_path);
	if (length >= 2 && ((raw_path[0] == '"' && raw_path[length - 1] == '"') ||
	                    (raw_path[0] == '\'' && raw_path[length - 1] == '\'')))
	{
		start = raw_path + 1;
		length -= 2;
	}

	const char* end = start + length;
	const char* slash = end;
	while (slash > start && slash[-1] != '/')
	{
		slash--;
	}

	size_t name_length = (size_t)(end - slash);
	if (name_length == 0 || name_length + 1 > output_size)
	{
		return false;
	}

	memcpy(output, slash, name_length);
	output[name_length] = '\0';
	return true;
}

static bool parser_is_namespace_symbol(Parser* parser, const char* name)
{
	if (!parser || !parser->symbol_table_stack || !parser->symbol_table_stack->current_scope ||
	    !name)
	{
		return false;
	}

	SymbolEntry* entry = stm_lookup(parser->symbol_table_stack->current_scope, name);
	return entry && entry->type && starts_with_text(entry->type, "module");
}

static char* build_namespace_symbol_name(const char* ns, const char* member)
{
	if (!ns || !member)
	{
		return NULL;
	}

	size_t length = strlen(ns) + strlen(member) + strlen("__modulecall____") + 1;
	char* result = malloc(length);
	if (!result)
	{
		return NULL;
	}

	snprintf(result, length, "__modulecall_%s__%s", ns, member);
	return result;
}

static bool parse_variadic_parameter(Parser* parser, char** variadic_name, ASTNode** variadic_type)
{
	if (!variadic_name || !variadic_type)
	{
		return false;
	}
	*variadic_name = NULL;
	*variadic_type = NULL;
	consume(parser, TOKEN_ELLIPSIS, "Expected '...' for variadic parameter.");
	if (!match(parser, TOKEN_IDENT))
	{
		error_expected(parser, "variadic parameter name after '...'");
		return false;
	}
	*variadic_name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_IDENT, "Expected variadic parameter name.");
	consume(parser, TOKEN_COLON, "Expected ':' after variadic parameter name.");
	*variadic_type = parse_type(parser);
	if (!*variadic_name || !*variadic_type)
	{
		if (*variadic_name)
		{
			free(*variadic_name);
			*variadic_name = NULL;
		}
		if (*variadic_type)
		{
			ast_free(*variadic_type);
			*variadic_type = NULL;
		}
		return false;
	}
	return true;
}

static char* wrap_array_type(char* inner)
{
	if (!inner)
	{
		return NULL;
	}

	size_t inner_length = strlen(inner);
	char* wrapped = malloc(inner_length + 8);
	if (!wrapped)
	{
		free(inner);
		return NULL;
	}

	snprintf(wrapped, inner_length + 8, "array<%s>", inner);
	free(inner);
	return wrapped;
}

static char* parse_object_type_name(Parser* parser)
{
	char* buffer = NULL;
	size_t length = 0;
	size_t capacity = 0;

	consume(parser, TOKEN_LBRACE, "Expected '{' to start object type.");
	if (!append_text(&buffer, &length, &capacity, "object{"))
	{
		return NULL;
	}

	if (!match(parser, TOKEN_RBRACE))
	{
		while (true)
		{
			if (!match(parser, TOKEN_IDENT))
			{
				free(buffer);
				error_expected(parser, "property name in object type");
				return NULL;
			}

			if (!append_text(&buffer, &length, &capacity, peek_current(parser)->lexeme))
			{
				free(buffer);
				return NULL;
			}
			advance_token(parser);

			consume(parser, TOKEN_COLON,
			        "Expected ':' after object type property name.");
			if (!append_text(&buffer, &length, &capacity, ":"))
			{
				free(buffer);
				return NULL;
			}

			char* property_type = parse_type_name(parser);
			if (!property_type)
			{
				free(buffer);
				return NULL;
			}
			if (!append_text(&buffer, &length, &capacity, property_type))
			{
				free(property_type);
				free(buffer);
				return NULL;
			}
			free(property_type);

			if (!match(parser, TOKEN_COMMA))
			{
				break;
			}
			advance_token(parser);
			if (match(parser, TOKEN_RBRACE))
			{
				break;
			}
			if (!append_text(&buffer, &length, &capacity, ","))
			{
				free(buffer);
				return NULL;
			}
		}
	}

	consume(parser, TOKEN_RBRACE, "Expected '}' to end object type.");
	if (!append_text(&buffer, &length, &capacity, "}"))
	{
		free(buffer);
		return NULL;
	}
	return buffer;
}

static char* parse_type_name(Parser* parser)
{
	char* result = NULL;

	if (peek_current(parser) && is_type_token(peek_current(parser)->type))
	{
		result = clone_string(peek_current(parser)->lexeme,
		                      strlen(peek_current(parser)->lexeme));
		advance_token(parser);
	}
	else if (match(parser, TOKEN_IDENT))
	{
		const char* aliased =
		    parser_resolve_type_alias(parser, peek_current(parser)->lexeme);
		if (!aliased)
		{
			error_expected(parser, "type");
			enter_recovery_mode(parser);
			synchronize(parser);
			return NULL;
		}
		result = clone_string(aliased, strlen(aliased));
		advance_token(parser);
	}
	else if (match(parser, TOKEN_LBRACE))
	{
		result = parse_object_type_name(parser);
	}
	else
	{
		error_expected(parser, "type");
		enter_recovery_mode(parser);
		synchronize(parser);
		return NULL;
	}

	while (match(parser, TOKEN_LBRACKET) && peek_lookahead1(parser) &&
	       peek_lookahead1(parser)->type == TOKEN_RBRACKET)
	{
		advance_token(parser);
		advance_token(parser);
		result = wrap_array_type(result);
		if (!result)
		{
			return NULL;
		}
	}

	return result;
}

static ASTNode* parse_type(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	char* type_name = parse_type_name(parser);
	if (!type_name)
	{
		return NULL;
	}
	ASTNode* node = ast_create_type(type_name, tok_line, tok_column);
	free(type_name);
	return node;
}

static ASTNode* parse_primary(Parser* parser)
{
	if (match(parser, TOKEN_IDENT) ||
	    (peek_current(parser) && is_type_token(peek_current(parser)->type) &&
	     parser_is_namespace_symbol(parser, peek_current(parser)->lexeme)))
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
		     la1->type == TOKEN_BOOL_TYPE || la1->type == TOKEN_ANY_TYPE ||
		     (la1->type == TOKEN_IDENT &&
		      parser_resolve_type_alias(parser, la1->lexeme) != NULL)))
		{
			size_t cast_line = peek_current(parser)->line;
			size_t cast_col = peek_current(parser)->column;
			advance_token(parser);
			ASTNode* target_type = parse_type(parser);
			consume(parser, TOKEN_RPAREN, "Expected ')' after cast type.");
			ASTNode* expr = parse_primary(parser);
			expr = apply_postfix(parser, expr);
			return ast_create_cast(target_type, expr, cast_line, cast_col);
		}
		advance_token(parser);
		ASTNode* expr = parse_expression(parser);
		consume(parser, TOKEN_RPAREN, "Expected ')' after grouped expression.");
		return expr;
	}
	else if (match(parser, TOKEN_LBRACE))
	{
		return parse_object_literal(parser);
	}
	else if (match(parser, TOKEN_LBRACKET))
	{
		return parse_array_literal(parser);
	}
	else
	{
		error_expected(parser, "primary expression");
		enter_recovery_mode(parser);
		synchronize(parser);
		return NULL;
	}
}

static ASTNode** parse_call_arguments(Parser* parser, size_t* count)
{
	*count = 0;
	ASTNode** args = NULL;
	size_t capacity = 0;

	consume(parser, TOKEN_LPAREN, "Expected '(' after callee.");
	if (!match(parser, TOKEN_RPAREN))
	{
		while (true)
		{
			if (*count >= capacity)
			{
				capacity = capacity == 0 ? 4 : capacity * 2;
				args = realloc(args, sizeof(ASTNode*) * capacity);
			}
			args[(*count)++] = parse_expression(parser);
			if (!match(parser, TOKEN_COMMA))
			{
				break;
			}
			advance_token(parser);
			if (match(parser, TOKEN_RPAREN))
			{
				break;
			}
		}
	}
	consume(parser, TOKEN_RPAREN, "Expected ')' after arguments in call expression.");
	return args;
}

static ASTNode* parse_call(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	char* callee =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_IDENT, "Expected function name for call expression.");
	size_t arg_count = 0;
	ASTNode** args = parse_call_arguments(parser, &arg_count);
	ASTNode* call = ast_create_call(callee, args, arg_count, tok_line, tok_column);
	free(callee);
	return call;
}

static ASTNode* build_call_from_callee(Parser* parser, ASTNode* callee)
{
	size_t arg_count = 0;
	ASTNode** args = parse_call_arguments(parser, &arg_count);
	ASTNode* call = NULL;

	if (!callee)
	{
		return NULL;
	}

	if (callee->type == AST_IDENTIFIER)
	{
		call = ast_create_call(callee->identifier.name, args, arg_count, callee->line,
		                       callee->column);
		ast_free(callee);
		return call;
	}

	if (callee->type == AST_MEMBER_ACCESS && callee->member_access.property &&
	    callee->member_access.property->type == AST_IDENTIFIER)
	{
		if (callee->member_access.object && callee->member_access.object->type == AST_IDENTIFIER &&
		    parser_is_namespace_symbol(parser, callee->member_access.object->identifier.name))
		{
			char* namespaced = build_namespace_symbol_name(
			    callee->member_access.object->identifier.name,
			    callee->member_access.property->identifier.name);
			if (!namespaced)
			{
				for (size_t i = 0; i < arg_count; i++)
				{
					ast_free(args[i]);
				}
				free(args);
				ast_free(callee);
				return NULL;
			}

			call = ast_create_call(namespaced, args, arg_count, callee->line, callee->column);
			free(namespaced);
			ast_free(callee);
			return call;
		}

		const char* method_name = callee->member_access.property->identifier.name;
		const char* lowered_name = method_name;
		char* dynamic_name = NULL;
		if (strcmp(method_name, "push") == 0)
		{
			lowered_name = "__array_push";
		}
		else if (strcmp(method_name, "format") == 0)
		{
			lowered_name = "__string_format";
		}
		else if (strcmp(method_name, "pop") == 0)
		{
			lowered_name = "__array_pop";
		}
		else if (strcmp(method_name, "clear") == 0)
		{
			lowered_name = "__array_clear";
		}
		else
		{
			size_t dynamic_len = strlen(method_name) + strlen("__member_") + 1;
			dynamic_name = malloc(dynamic_len);
			if (!dynamic_name)
			{
				for (size_t i = 0; i < arg_count; i++)
				{
					ast_free(args[i]);
				}
				free(args);
				ast_free(callee);
				return NULL;
			}
			snprintf(dynamic_name, dynamic_len, "__member_%s", method_name);
			lowered_name = dynamic_name;
		}

		ASTNode** rewritten_args = malloc(sizeof(ASTNode*) * (arg_count + 1));
		if (!rewritten_args)
		{
			for (size_t i = 0; i < arg_count; i++)
			{
				ast_free(args[i]);
			}
			free(args);
			ast_free(callee);
			return NULL;
		}

		rewritten_args[0] = callee->member_access.object;
		callee->member_access.object = NULL;
		for (size_t i = 0; i < arg_count; i++)
		{
			rewritten_args[i + 1] = args[i];
		}
		free(args);
		call = ast_create_call(lowered_name, rewritten_args, arg_count + 1, callee->line,
		                       callee->column);
		free(dynamic_name);
		ast_free(callee);
		return call;
	}

	error_expected(parser, "callable expression");
	for (size_t i = 0; i < arg_count; i++)
	{
		ast_free(args[i]);
	}
	free(args);
	ast_free(callee);
	return NULL;
}

static ASTNode* apply_postfix(Parser* parser, ASTNode* expr)
{
	while (expr)
	{
		if (match(parser, TOKEN_LPAREN))
		{
			expr = build_call_from_callee(parser, expr);
		}
		else if (match(parser, TOKEN_DOT))
		{
			size_t op_line = peek_current(parser)->line;
			size_t op_col = peek_current(parser)->column;
			advance_token(parser);
			if (!match(parser, TOKEN_IDENT))
			{
				ast_free(expr);
				error_expected(parser, "property name after '.'");
				return NULL;
			}
			ASTNode* property = ast_create_identifier(peek_current(parser)->lexeme,
			                                          peek_current(parser)->line,
			                                          peek_current(parser)->column);
			advance_token(parser);
			expr = ast_create_member_access(expr, property, op_line, op_col);
		}
		else if (match(parser, TOKEN_LBRACKET))
		{
			size_t op_line = peek_current(parser)->line;
			size_t op_col = peek_current(parser)->column;
			advance_token(parser);
			ASTNode* index = parse_expression(parser);
			consume(parser, TOKEN_RBRACKET, "Expected ']' after index.");
			expr = ast_create_array_access(expr, index, op_line, op_col);
		}
		else
		{
			break;
		}
	}
	return expr;
}

static ASTNode* parse_postfix(Parser* parser)
{
	ASTNode* expr = parse_primary(parser);
	return apply_postfix(parser, expr);
}

static ASTNode* parse_identifier_statement(Parser* parser)
{
	Token* ident = peek_current(parser);
	size_t tok_line = ident->line;
	size_t tok_column = ident->column;
	ASTNode* target = parse_expression(parser);
	ASTNode* value = NULL;
	char* name = NULL;

	if (match(parser, TOKEN_EQUALS))
	{
		if (!target || target->type != AST_IDENTIFIER)
		{
			ast_free(target);
			error_expected(parser, "assignable identifier");
			return NULL;
		}
		name = clone_string(target->identifier.name, strlen(target->identifier.name));
		advance_token(parser);
		value = parse_expression(parser);
	}
	else if (match(parser, TOKEN_PLUS_EQUALS))
	{
		if (!target || target->type != AST_IDENTIFIER)
		{
			ast_free(target);
			error_expected(parser, "assignable identifier");
			return NULL;
		}
		name = clone_string(target->identifier.name, strlen(target->identifier.name));
		advance_token(parser);
		ASTNode* right = parse_expression(parser);
		value = ast_create_binary_op("+", ast_create_identifier(name, tok_line, tok_column),
		                             right, tok_line, tok_column);
	}
	else if (match(parser, TOKEN_MINUS_EQUALS))
	{
		if (!target || target->type != AST_IDENTIFIER)
		{
			ast_free(target);
			error_expected(parser, "assignable identifier");
			return NULL;
		}
		name = clone_string(target->identifier.name, strlen(target->identifier.name));
		advance_token(parser);
		ASTNode* right = parse_expression(parser);
		value = ast_create_binary_op("-", ast_create_identifier(name, tok_line, tok_column),
		                             right, tok_line, tok_column);
	}
	else if (match(parser, TOKEN_STAR_EQUALS))
	{
		if (!target || target->type != AST_IDENTIFIER)
		{
			ast_free(target);
			error_expected(parser, "assignable identifier");
			return NULL;
		}
		name = clone_string(target->identifier.name, strlen(target->identifier.name));
		advance_token(parser);
		ASTNode* right = parse_expression(parser);
		value = ast_create_binary_op("*", ast_create_identifier(name, tok_line, tok_column),
		                             right, tok_line, tok_column);
	}
	else if (match(parser, TOKEN_SLASH_EQUALS))
	{
		if (!target || target->type != AST_IDENTIFIER)
		{
			ast_free(target);
			error_expected(parser, "assignable identifier");
			return NULL;
		}
		name = clone_string(target->identifier.name, strlen(target->identifier.name));
		advance_token(parser);
		ASTNode* right = parse_expression(parser);
		value = ast_create_binary_op("/", ast_create_identifier(name, tok_line, tok_column),
		                             right, tok_line, tok_column);
	}
	else if (match(parser, TOKEN_PLUS_PLUS))
	{
		if (!target || target->type != AST_IDENTIFIER)
		{
			ast_free(target);
			error_expected(parser, "assignable identifier");
			return NULL;
		}
		name = clone_string(target->identifier.name, strlen(target->identifier.name));
		advance_token(parser);
		value = ast_create_binary_op("+", ast_create_identifier(name, tok_line, tok_column),
		                             ast_create_number_literal("1", tok_line, tok_column),
		                             tok_line, tok_column);
	}
	else if (match(parser, TOKEN_MINUS_MINUS))
	{
		if (!target || target->type != AST_IDENTIFIER)
		{
			ast_free(target);
			error_expected(parser, "assignable identifier");
			return NULL;
		}
		name = clone_string(target->identifier.name, strlen(target->identifier.name));
		advance_token(parser);
		value = ast_create_binary_op("-", ast_create_identifier(name, tok_line, tok_column),
		                             ast_create_number_literal("1", tok_line, tok_column),
		                             tok_line, tok_column);
	}
	else
	{
		consume(parser, TOKEN_SEMICOLON, "Expected ';' after expression statement.");
		return ast_create_expression_statement(target, tok_line, tok_column);
	}

	consume(parser, TOKEN_SEMICOLON, "Expected ';' after identifier statement.");
	ast_free(target);
	parser_use_symbol(parser, name);
	ASTNode* assign = ast_create_assignment(name, value, tok_line, tok_column);
	free(name);
	return assign;
}

static ASTNode* parse_while_statement(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	consume(parser, TOKEN_WHILE, "Expected 'while' keyword.");

	ASTNode* condition = parse_expression(parser);
	ASTNode* body = parse_block(parser);

	return ast_create_while(condition, body, tok_line, tok_column);
}

static ASTNode* parse_for_statement(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_col = peek_current(parser)->column;
	consume(parser, TOKEN_FOR, "Expected 'for' keyword.");

	ASTNode* var_decl = parse_variable_declaration(parser);
	ASTNode* condition = parse_expression(parser);
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after for loop condition.");

	Token* inc_ident = peek_current(parser);
	size_t tok_line_inc = inc_ident->line;
	size_t tok_col_inc = inc_ident->column;
	char* inc_name = clone_string(inc_ident->lexeme, inc_ident->length);
	consume(parser, TOKEN_IDENT, "Expected identifier for loop increment.");

	ASTNode* inc_value = NULL;
	if (match(parser, TOKEN_EQUALS))
	{
		advance_token(parser);
		inc_value = parse_expression(parser);
	}
	else if (match(parser, TOKEN_PLUS_EQUALS))
	{
		advance_token(parser);
		ASTNode* right = parse_expression(parser);
		inc_value = ast_create_binary_op(
		    "+", ast_create_identifier(inc_name, tok_line_inc, tok_col_inc), right,
		    tok_line_inc, tok_col_inc);
	}
	else if (match(parser, TOKEN_MINUS_EQUALS))
	{
		advance_token(parser);
		ASTNode* right = parse_expression(parser);
		inc_value = ast_create_binary_op(
		    "-", ast_create_identifier(inc_name, tok_line_inc, tok_col_inc), right,
		    tok_line_inc, tok_col_inc);
	}
	else if (match(parser, TOKEN_PLUS_PLUS))
	{
		advance_token(parser);
		inc_value = ast_create_binary_op(
		    "+", ast_create_identifier(inc_name, tok_line_inc, tok_col_inc),
		    ast_create_number_literal("1", tok_line_inc, tok_col_inc), tok_line_inc,
		    tok_col_inc);
	}
	else if (match(parser, TOKEN_MINUS_MINUS))
	{
		advance_token(parser);
		inc_value = ast_create_binary_op(
		    "-", ast_create_identifier(inc_name, tok_line_inc, tok_col_inc),
		    ast_create_number_literal("1", tok_line_inc, tok_col_inc), tok_line_inc,
		    tok_col_inc);
	}
	else
	{
		error_expected(parser, "assignment or increment operator");
	}

	ASTNode* increment = ast_create_assignment(inc_name, inc_value, tok_line_inc, tok_col_inc);
	free(inc_name);

	ASTNode* body = parse_block(parser);

	return ast_create_for(var_decl, condition, increment, body, tok_line, tok_col);
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
	ASTNode* base = parse_postfix(parser);

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

static ASTNode** parse_parameter_list(Parser* parser, size_t* count, bool* is_variadic,
                                      char** variadic_name, ASTNode** variadic_type)
{
	*count = 0;
	*is_variadic = false;
	if (variadic_name)
	{
		*variadic_name = NULL;
	}
	if (variadic_type)
	{
		*variadic_type = NULL;
	}
	ASTNode** params = NULL;
	size_t capacity = 0;

	if (!match(parser, TOKEN_RPAREN))
	{
		if (match(parser, TOKEN_ELLIPSIS))
		{
			*is_variadic =
			    parse_variadic_parameter(parser, variadic_name, variadic_type);
			return params;
		}
		capacity = 4;
		params = malloc(sizeof(ASTNode*) * capacity);
		params[(*count)++] = parse_parameter(parser);

		while (match(parser, TOKEN_COMMA))
		{
			advance_token(parser);
			if (match(parser, TOKEN_ELLIPSIS))
			{
				*is_variadic =
				    parse_variadic_parameter(parser, variadic_name, variadic_type);
				break;
			}
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
	char namespace_name[128];
	if (derive_import_namespace(path, namespace_name, sizeof(namespace_name)))
	{
		parser_declare_variable(parser, namespace_name, "module", false, 0);
	}
	ASTNode* import = ast_create_import(path, tok_line, tok_column);
	free(path);
	return import;
}

static ASTNode* parse_variable_declaration(Parser* parser)
{
	bool is_mutable = match(parser, TOKEN_SET);
	if (is_mutable)
	{
		consume(parser, TOKEN_SET, "Expected 'set' keyword for variable declaration.");
	}
	else
	{
		consume(parser, TOKEN_CONST, "Expected declaration keyword.");
	}

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
		advance_token(parser);
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

	consume(parser, TOKEN_COLON, "Expected ':' after variable name.");
	if (!peek_current(parser)->lexeme)
	{
		error_expected(parser, "type name after ':'");
		enter_recovery_mode(parser);
		free(name);
		return NULL;
	}
	ASTNode* type = parse_type(parser);
	char* type_name = type && type->type_node.name
	                      ? clone_string(type->type_node.name, strlen(type->type_node.name))
	                      : NULL;

	consume(parser, TOKEN_EQUALS, "Expected '=' after variable type.");
	ASTNode* initializer = parse_expression(parser);
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration.");

	if (name && type_name && !parser->recovery_mode)
	{
		parser_declare_variable(parser, name, type_name, is_mutable, 0);
	}
	free(type_name);

	ASTNode* var_decl = ast_create_variable_declaration(name, type, initializer, is_mutable,
	                                                    tok_line, tok_column);
	free(name);
	return var_decl;
}

static ASTNode* parse_type_declaration(Parser* parser)
{
	consume(parser, TOKEN_TYPE, "Expected 'type' keyword for type declaration.");

	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	char* name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_IDENT, "Expected type name after 'type' keyword.");
	consume(parser, TOKEN_EQUALS, "Expected '=' after type name.");
	ASTNode* value_type = parse_type(parser);
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after type declaration.");

	if (!value_type)
	{
		free(name);
		return NULL;
	}

	if (!parser_declare_type_alias(parser, name, value_type->type_node.name))
	{
		fprintf(stderr, "Duplicate type alias '%s'. (Error)\n", name);
		parser->error_count++;
	}

	ASTNode* type_decl = ast_create_type_declaration(name, value_type, tok_line, tok_column);
	free(name);
	return type_decl;
}

static ASTNode* parse_function_declaration(Parser* parser)
{
	consume(parser, TOKEN_FUN, "Expected 'function' keyword for function declaration.");

	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	char* name =
	    clone_string(peek_current(parser)->lexeme, strlen(peek_current(parser)->lexeme));
	consume(parser, TOKEN_IDENT, "Expected function name after 'function' keyword.");

	consume(parser, TOKEN_LPAREN, "Expected '(' after function name.");
	size_t param_count = 0;
	bool is_variadic = false;
	char* variadic_name = NULL;
	ASTNode* variadic_type = NULL;
	ASTNode** params = parse_parameter_list(parser, &param_count, &is_variadic, &variadic_name,
	                                        &variadic_type);
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
	                                                     body, is_variadic, variadic_name,
	                                                     variadic_type, tok_line, tok_column);
	free(variadic_name);
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
		advance_token(parser);
		ASTNode* else_branch = NULL;
		if (peek_current(parser) && peek_current(parser)->type == TOKEN_IF)
		{
			else_branch = parse_if_statement(parser);
		}
		else
		{
			else_branch = parse_block(parser);
		}
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

static ASTNode* parse_break_statement(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	consume(parser, TOKEN_BREAK, "Expected 'break' keyword.");
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after break statement.");
	return ast_create_break(tok_line, tok_column);
}

static ASTNode* parse_continue_statement(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	consume(parser, TOKEN_CONTINUE, "Expected 'continue' keyword.");
	consume(parser, TOKEN_SEMICOLON, "Expected ';' after continue statement.");
	return ast_create_continue(tok_line, tok_column);
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
		case TOKEN_TYPE:
			return parse_type_declaration(parser);
		case TOKEN_IMPORT:
			return parse_import_statement(parser);
		case TOKEN_IF:
			return parse_if_statement(parser);
		case TOKEN_WHILE:
			return parse_while_statement(parser);
		case TOKEN_FOR:
			return parse_for_statement(parser);
		case TOKEN_SET:
		case TOKEN_CONST:
			return parse_variable_declaration(parser);
		case TOKEN_RETURN:
			return parse_return_statement(parser);
		case TOKEN_BREAK:
			return parse_break_statement(parser);
		case TOKEN_CONTINUE:
			return parse_continue_statement(parser);
		case TOKEN_IDENT:
			return parse_identifier_statement(parser);
		default:
			error_expected(parser, "statement");
			enter_recovery_mode(parser);
			synchronize(parser);
			return NULL;
	}
}
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

static ASTNode* parse_object_literal(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	consume(parser, TOKEN_LBRACE, "Expected '{' to start object literal.");

	ASTObjectProperty* properties = NULL;
	size_t count = 0;
	size_t capacity = 0;

	if (!match(parser, TOKEN_RBRACE))
	{
		do
		{
			if (match(parser, TOKEN_RBRACE))
			{
				break;
			}
			if (count >= capacity)
			{
				capacity = capacity == 0 ? 4 : capacity * 2;
				properties = realloc(properties, sizeof(ASTObjectProperty) * capacity);
			}

			char* key = clone_string(peek_current(parser)->lexeme,
			                         strlen(peek_current(parser)->lexeme));
			consume(parser, TOKEN_IDENT, "Expected property name.");
			consume(parser, TOKEN_COLON, "Expected ':' after property name.");
			ASTNode* value = parse_expression(parser);

			properties[count].key = key;
			properties[count].value = value;
			count++;
		} while (match(parser, TOKEN_COMMA) && (advance_token(parser), true));
	}

	consume(parser, TOKEN_RBRACE, "Expected '}' to end object literal.");
	return ast_create_object_literal(properties, count, tok_line, tok_column);
}

static ASTNode* parse_array_literal(Parser* parser)
{
	size_t tok_line = peek_current(parser)->line;
	size_t tok_column = peek_current(parser)->column;
	consume(parser, TOKEN_LBRACKET, "Expected '[' to start array literal.");

	ASTNode** elements = NULL;
	size_t count = 0;
	size_t capacity = 0;

	if (!match(parser, TOKEN_RBRACKET))
	{
		do
		{
			if (match(parser, TOKEN_RBRACKET))
			{
				break;
			}
			if (count >= capacity)
			{
				capacity = capacity == 0 ? 4 : capacity * 2;
				elements = realloc(elements, sizeof(ASTNode*) * capacity);
			}
			elements[count++] = parse_expression(parser);
		} while (match(parser, TOKEN_COMMA) && (advance_token(parser), true));
	}

	consume(parser, TOKEN_RBRACKET, "Expected ']' to end array literal.");
	return ast_create_array_literal(elements, count, tok_line, tok_column);
}

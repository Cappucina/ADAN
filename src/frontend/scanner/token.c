#include <stdlib.h>
#include <stdio.h>

#include "token.h"

void token_stream_free(Token* tokens)
{
	if (!tokens)
	{
		return;
	}
	for (size_t i = 0; tokens[i].type != TOKEN_EOF; i++)
	{
		free(tokens[i].lexeme);
	}
	free(tokens);
}

Token* make_token(TokenType type, size_t column, size_t line, char* lexeme, size_t length)
{
	Token* token = (Token*)calloc(1, sizeof(Token));
	token->type = type;
	token->column = column;
	token->line = line;
	token->lexeme = lexeme;
	token->length = length;
	return token;
}

char* token_type_to_string(TokenType type)
{
	switch (type)
	{
		case TOKEN_EOF:
			return "EOF";
		case TOKEN_IDENT:
			return "IDENT";
		case TOKEN_FUN:
			return "FUN";
		case TOKEN_IMPORT:
			return "IMPORT";
		case TOKEN_SET:
			return "SET";
		case TOKEN_RETURN:
			return "RETURN";
		case TOKEN_STRING_TYPE:
			return "STRING_TYPE";
		case TOKEN_I32_TYPE:
			return "I32_TYPE";
		case TOKEN_I64_TYPE:
			return "I64_TYPE";
		case TOKEN_VOID_TYPE:
			return "VOID_TYPE";
		case TOKEN_U32_TYPE:
			return "U32_TYPE";
		case TOKEN_U64_TYPE:
			return "U64_TYPE";
		case TOKEN_STRING:
			return "STRING";
		case TOKEN_NUMBER:
			return "NUMBER";
		case TOKEN_LPAREN:
			return "LPAREN";
		case TOKEN_RPAREN:
			return "RPAREN";
		case TOKEN_LBRACE:
			return "LBRACE";
		case TOKEN_RBRACE:
			return "RBRACE";
		case TOKEN_COLON:
			return "COLON";
		case TOKEN_SEMICOLON:
			return "SEMICOLON";
		case TOKEN_EQUALS:
			return "EQUALS";
		case TOKEN_QUOTE:
			return "QUOTE";
		case TOKEN_COMMA:
			return "COMMA";
		default:
			return "UNKNOWN";
	}
}

void print_token_stream(Token* tokens)
{
	for (size_t i = 0; tokens[i].type != TOKEN_EOF; i++)
	{
		printf("Token: %.*s (Type: %s, Line: %zu, Column: %zu)\n", (int)tokens[i].length,
		       tokens[i].lexeme, token_type_to_string(tokens[i].type), tokens[i].line,
		       tokens[i].column);
	}
}
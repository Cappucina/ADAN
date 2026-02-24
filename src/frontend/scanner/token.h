#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <stddef.h>

typedef enum TokenType
{
	// Special
	TOKEN_EOF,
	TOKEN_IDENT,

	// Keywords
	TOKEN_FUN,
	TOKEN_IMPORT,
	TOKEN_SET,
	TOKEN_RETURN,

	// Types
	TOKEN_STRING_TYPE,
	TOKEN_I32_TYPE,
	TOKEN_I64_TYPE,
	TOKEN_VOID_TYPE,
	TOKEN_U32_TYPE,
	TOKEN_U64_TYPE,

	// Literals
	TOKEN_STRING,
	TOKEN_NUMBER,

	// Symbols
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_COLON,
	TOKEN_SEMICOLON,
	TOKEN_EQUALS,
	TOKEN_QUOTE,
	TOKEN_COMMA,
} TokenType;

typedef struct Token
{
	TokenType type;
	size_t column;
	size_t line;
	char* lexeme;  // Literal value of the token.
	size_t length;
} Token;

typedef struct Keyword
{
	const char* word;
	TokenType type;
} Keyword;

void token_stream_free(Token* tokens);

Token* make_token(TokenType type, size_t column, size_t line, char* lexeme, size_t length);

void print_token_stream(Token* tokens);

#endif
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
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_WHILE,
	TOKEN_OR,   // or
	TOKEN_AND,  // and
	TOKEN_NOT,  // not
	TOKEN_FOR,

	// Types
	TOKEN_STRING_TYPE,
	TOKEN_I8_TYPE,
	TOKEN_I32_TYPE,
	TOKEN_I64_TYPE,
	TOKEN_U8_TYPE,
	TOKEN_U32_TYPE,
	TOKEN_U64_TYPE,
	TOKEN_F32_TYPE,
	TOKEN_F64_TYPE,
	TOKEN_VOID_TYPE,
	TOKEN_BOOL_TYPE,

	// Literals
	TOKEN_STRING,
	TOKEN_NUMBER,
	TOKEN_TRUE,
	TOKEN_FALSE,

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
	TOKEN_BACKTICK,
	TOKEN_DOLLAR,
	TOKEN_INTERP_START,
	TOKEN_INTERP_END,

	// Operators
	TOKEN_PLUS,
	TOKEN_PLUS_EQUALS,
	TOKEN_PLUS_PLUS,
	TOKEN_MINUS,
	TOKEN_MINUS_EQUALS,
	TOKEN_MINUS_MINUS,
	TOKEN_STAR,
	TOKEN_STAR_EQUALS,
	TOKEN_EQUALS_EQUALS,
	TOKEN_BANG_EQUALS,
	TOKEN_LESS,
	TOKEN_LESS_EQUAL,
	TOKEN_GREATER,
	TOKEN_GREATER_EQUAL,
	TOKEN_SLASH,
	TOKEN_SLASH_EQUALS,
	TOKEN_CARET,
	TOKEN_PERCENT,
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
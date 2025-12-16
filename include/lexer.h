#include <stdbool.h>

#ifndef LEXER_H
#define LEXER_H

typedef enum {
	TOKEN_IDENTIFIER,    // For variable, function, or any kind of declaration.

	//
	//  Types
	//
	TOKEN_INT,		  // A whole number where the limit is your CPU's bit limit. (x32, x64, ...)
	TOKEN_FLOAT,	  // A more specific number allowing for decimal placements.
	TOKEN_STRING,	  // A collection of characters wrapped inside of quotation marks. ("")
	TOKEN_BOOLEAN,	  // Simply a true or false statement. Typically used to check conditions.
	TOKEN_ARRAY,	  // Statically sized object of values. Cannot mix different types into the same array.
	TOKEN_CHAR,	   	  // A single character, typically represented in ASCII or UNICODE.
	TOKEN_NULL,	      // Represents an invalid or a non-existant return value. Unlike `void`, `null` is a valid return type and can be matched.
	TOKEN_VOID,	      // Represents the act of returning nothing. Matching will `null` will return false.

	//
	//  Operands
	//
	TOKEN_ADD_IMMEDIATE,  // Used to add a value directly to a variable. (e.g., x += 5)
	TOKEN_SUB_IMMEDIATE,  // Used to subtract a value directly from a variable. (e.g., x -= 5)
	TOKEN_MUL_IMMEDIATE,  // Used to multiply a variable by a value directly. (e.g., x *= 5)
	TOKEN_DIV_IMMEDIATE,  // Used to divide a variable by a value directly. (e.g., x /= 5)
	TOKEN_MOD_IMMEDIATE,  // Used to get the modulus of a variable by a value directly. (e.g., x %= 5)
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_ASTERISK,
	TOKEN_SLASH,
	TOKEN_PERCENT,
	TOKEN_CAROT,
	TOKEN_EXPONENT,
	TOKEN_AT,

	//
	//  Bitwise Operands
	TOKEN_BITWISE_AND,
	TOKEN_BITWISE_OR,
	TOKEN_BITWISE_NOT,
	TOKEN_BITWISE_XOR,
	TOKEN_BITWISE_NAND, // Didn't find standard char must be not used often
	TOKEN_BITWISE_NOR, // same here
	TOKEN_BITWISE_XNOR, // and here
	TOKEN_BITWISE_ZERO_FILL_LEFT_SHIFT,
	TOKEN_BITWISE_SIGNED_RIGHT_SHIFT,
	TOKEN_BITWISE_ZERO_FILL_RIGHT_SHIFT,

	//
	//  Increment, Decrement
	// 
	TOKEN_INCREMENT,
	TOKEN_DECREMENT,
	
	//
	//  Symbols
	//
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_LBRACKET,
	TOKEN_RBRACKET,
	TOKEN_SEMICOLON,
	TOKEN_COMMA,
	TOKEN_PERIOD,
	TOKEN_APOSTROPHE,
	TOKEN_QUOTATION,
	TOKEN_NOT,
	TOKEN_AMPERSAND,
	TOKEN_TYPE_DECL,

	//
	//  Signs
	//
	TOKEN_EQUALS,
	TOKEN_GREATER,
	TOKEN_LESS,
	TOKEN_GREATER_EQUALS,
	TOKEN_LESS_EQUALS,
	TOKEN_ASSIGN,
	TOKEN_NOT_EQUALS,
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_ELLIPSIS,
	TOKEN_LEFT_SHIFT,
	TOKEN_RIGHT_SHIFT,

	//
	//  Keywords
	//
	TOKEN_IF,
	TOKEN_WHILE,
	TOKEN_FOR,
	TOKEN_INCLUDE,
	TOKEN_BREAK,
	TOKEN_CONTINUE,
	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_PROGRAM,
	TOKEN_RETURN,
	TOKEN_ELSE,

	//
	//  Literals
	//
	TOKEN_INT_LITERAL,
	TOKEN_FLOAT_LITERAL,
	TOKEN_SINGLE_COMMENT,
	TOKEN_BLOCK_COMMENT,

	//
	//  Special
	//
	TOKEN_ERROR,    // When something goes wrong or when coming across an unexpected token. Better than returning NULL itself.
	TOKEN_EOF,	    // Used when representing the lack of any more tokens in a file.
} TokenType;

typedef struct {
	TokenType type;
	char *text;
	int line;
	int column;
} Token;

typedef struct {
	const char *src;
	int position;
	int line;
} Lexer;

static inline bool is_digit(char c) {
	return (c >= '0' && c <= '9');
}

static inline bool is_whitespace(char c) {
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

Lexer* create_lexer(const char *src);

void free_lexer(Lexer *lexer);

Token* make_token(Lexer *lexer, TokenType type, const char *parts[], int part_count);

Token* next_token(Lexer *lexer);

void free_token(Token *token);

void print_token(Token *token);

//
//  Preview the next upcoming token without advancing to it. Returns the
//   token that's ahead.
//
static inline char peek(Lexer *lexer) {
	return lexer->src[lexer->position + 1] ? lexer->src[lexer->position + 1] : '\0';
}

//
//  Bumps cursor +1 space to go to the next token.
//
static inline void advance(Lexer *lexer) {
	if (lexer->src[lexer->position] == '\n') {
		lexer->line++;
	}
	lexer->position++;
}

#endif
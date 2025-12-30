#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#include "errors.h"

typedef enum
{
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_IDENTIFIER,

    /**
     *
     * Types
     */
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_BOOL,
    TOKEN_CHAR,
    TOKEN_NULL,
    TOKEN_VOID,

    /**
     *
     * Operators
     */
    TOKEN_ADD,
    TOKEN_SUBTRACT,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,
    TOKEN_CARET,
    TOKEN_EXPONENT,
    TOKEN_ADDRESS_OF,
    TOKEN_REFERENCE,

    /**
     *
     * (In/de)crementing
     */
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,

    /**
     *
     * Bitwise operators
     */
    TOKEN_BITWISE_AND,
    TOKEN_BITWISE_OR,
    TOKEN_BITWISE_NOT,
    TOKEN_BITWISE_XOR,
    TOKEN_BITWISE_NAND,
    TOKEN_BITWISE_NOR,
    TOKEN_BITWISE_XNOR,
    TOKEN_BITWISE_ZERO_FILL_LEFT_SHIFT,
    TOKEN_BITWISE_SIGNED_RIGHT_SHIFT,
    TOKEN_BITWISE_ZERO_FILL_RIGHT_SHIFT,

    /**
     *
     * Symbols
     */
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_PERIOD,
    TOKEN_APOSTROPHE,
    TOKEN_QUOTATION,
    TOKEN_NOT,
    TOKEN_AND,
    TOKEN_TYPE_DECLARATOR,
    TOKEN_EQUALS,
    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_GREATER_EQUALS,
    TOKEN_LESS_EQUALS,
    TOKEN_ASSIGN,
    TOKEN_NOT_EQUALS,
    TOKEN_OR,
    TOKEN_ELLIPSIS,

    /**
     *
     * Type literals
     */
    TOKEN_TRUE_LITERAL,
    TOKEN_INT_LITERAL,
    TOKEN_FALSE_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_SINGLE_COMMENT,
    TOKEN_MULTI_COMMENT,

    /**
     *
     * Keywords
     */
    TOKEN_IF,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_INCLUDE,
    TOKEN_CONTINUE,
    TOKEN_PROGRAM,
    TOKEN_RETURN,
    TOKEN_ELSE,
    TOKEN_STRUCT,
    TOKEN_BREAK
} TokenType;

typedef struct
{
    TokenType type;
    size_t position;
    size_t length;
    size_t line;
    size_t column;
    char* literal;
} Token;

typedef struct
{
    const char* source;
    int position;
    size_t length;
    size_t line;
    size_t column;
} Lexer;

Lexer* create_lexer(ErrorList* error_list);

void free_lexer(Lexer* lx);

Token create_token();

void next_token(Lexer* lx);

char peek_char(Lexer* lx);

#endif
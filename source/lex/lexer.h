#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../../include/diagnostics.h"

typedef enum Tokens
{
    // Special
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_IDENTIFIER,

    // Keywords
    TOKEN_PROGRAM,
    TOKEN_INCLUDE,
    TOKEN_STRUCT,
    TOKEN_FOR,
    TOKEN_IF,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_NULL,
    TOKEN_ELSE,

    // Types
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_BOOL,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_VOID,

    // Literal types
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,

    // Symbols
    TOKEN_OPEN_PAREN,      // (
    TOKEN_CLOSE_PAREN,     // )
    TOKEN_OPEN_BRACE,      // {
    TOKEN_CLOSE_BRACE,     // }
    TOKEN_SEMICOLON,       // ;
    TOKEN_EQUAL,           // = (Used for ASSIGNMENT, not comparison!)
    TOKEN_EQUALITY,        // ==
    TOKEN_NOT,             // !
    TOKEN_NOT_EQUALS,      // !==
    TOKEN_TYPE_DECL,       // ::
    TOKEN_MUL,             // *
    TOKEN_DIV,             // /
    TOKEN_SUB,             // -
    TOKEN_ADD,             // +
    TOKEN_MOD,             // %
    TOKEN_EXPONENT,        // **
    TOKEN_QUOTE,           // "
    TOKEN_APOSTROPHE,      // '
    TOKEN_ELLIPSIS,        // ...
                           // TOKEN_POINTER, // * (Maybe?)
    TOKEN_AND,             // &&
    TOKEN_OR,              // ||
    TOKEN_MUL_EQUALS,      // *=
    TOKEN_DIV_EQUALS,      // /=
    TOKEN_SUB_EQUALS,      // -=
    TOKEN_ADD_EQUALS,      // +=
    TOKEN_MOD_EQUALS,      // %=
    TOKEN_COMMA,           // ,
    TOKEN_PERIOD,          // .
    TOKEN_GREATER,         // >
    TOKEN_LESS,            // <
    TOKEN_GREATER_EQUALS,  // >=
    TOKEN_LESS_EQUALS,     // <=
    TOKEN_ADD_ADD,         // ++
    TOKEN_SUB_SUB,         // --
    TOKEN_OPEN_BRACKET,    // [
    TOKEN_CLOSE_BRACKET,   // ]

    // Bitwise operators
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
} Tokens;

typedef struct
{
    const char* lexeme;
    uint32_t start;
    uint32_t length;
    const char* file;
    uint32_t line;
    uint32_t column;
    Tokens type;
} Token;

typedef struct
{
    const char* source;
    uint32_t position;
    uint32_t length;
    const char* file;
    uint32_t line;
    uint32_t column;
    ErrorList* errors;
} Lexer;

typedef struct
{
    const char* name;
    Tokens type;
} Keyword;

extern Keyword keywords[];

char peek_char(Lexer* lex);

char peek_next(Lexer* lex, uint32_t offset);

char next_char(Lexer* lex);

Lexer* create_lexer(const char* source, ErrorList* error_list, const char* file);

void free_lexer(Lexer* lex);

Token create_token(Lexer* lex, const char* lexeme, uint32_t start, uint32_t length, Tokens type);

Token lex(Lexer* lx);

Token lex_identifier(Lexer* lex);

Token lex_number(Lexer* lex);

Token lex_string(Lexer* lex);

Token lex_operator(Lexer* lex);

Token lex_char(Lexer* lex);

void skip_whitespace(Lexer* lex);

#endif
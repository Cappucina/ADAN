#include <stdbool.h>

#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENTIFIER,    // For variable, function, or any kind of declaration.

    // Types
    TOKEN_INT,        // A whole number where the limit is your CPU's bit limit. (x32, x64, ...)
    TOKEN_FLOAT,      // A more specific number allowing for decimal placements.
    TOKEN_STRING,     // A collection of characters wrapped inside of quotation marks. ("")
    TOKEN_BOOLEAN,    // Simply a true or false statement. Typically used to check conditions.
    TOKEN_ARRAY,      // Statically sized object of values. Cannot mix different types into the same array.
    TOKEN_CHAR,       // A single character, typically represented in ASCII or UNICODE.
    TOKEN_NULL,       // Represents an invalid or a non-existant return value. Unlike `void`, `null` is a valid return type and can be matched.
    TOKEN_VOID,       // Represents the act of returning nothing. Matching will `null` will return false.

    // Operands
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERISK,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_CAROT,

    // Symbols
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_PERIOD,
    TOKEN_APOSTROPHE,
    TOKEN_QUOTATION,

    // Keywords
    TOKEN_IF,         // Comparison between two values.
    TOKEN_WHILE,      // Loops until the condition provided is true.
    TOKEN_FOR,        // Loops specific code for a certain amount of times.
    TOKEN_INCLUDE,    // Import third party libraries into your project to get more functionality.
    TOKEN_BREAK,      // Stops a loop regardless if the condition is met.
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_PROGRAM,    // Defines a new function, allowing code to be reusable.

    // Special
    TOKEN_EOF,    // Used when representing the lack of any more tokens in a file.
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    int line;
} Token;

typedef struct {
    const char *src;
    int position;
    int line;
} Lexer;

inline bool is_digit(char c) {
    return (c >= '0' && c <= '9');
}

inline bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

// 
//  Preview the next upcoming token without advancing to it. Returns the
//   token that's ahead.
// 
inline int peek(Lexer *lexer) {
    return lexer->src[lexer->position + 1];
}

// 
//  Bumps cursor +1 space to go to the next token.
// 
inline void advance(Lexer *lexer) {
    if (lexer->src[lexer->position] == '\n') {
        lexer->line++;
    }
    lexer->position++;
}

#endif
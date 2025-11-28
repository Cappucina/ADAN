#include <stdbool.h>

#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_IDENTIFIER,    // For variable, function, or any kind of declaration.

    //
    //  Types
    //
    TOKEN_INT,        // A whole number where the limit is your CPU's bit limit. (x32, x64, ...)
    TOKEN_FLOAT,      // A more specific number allowing for decimal placements.
    TOKEN_STRING,     // A collection of characters wrapped inside of quotation marks. ("")
    TOKEN_BOOLEAN,    // Simply a true or false statement. Typically used to check conditions.
    TOKEN_ARRAY,      // Statically sized object of values. Cannot mix different types into the same array.
    TOKEN_CHAR,       // A single character, typically represented in ASCII or UNICODE.
    TOKEN_NULL,       // Represents an invalid or a non-existant return value. Unlike `void`, `null` is a valid return type and can be matched.
    TOKEN_VOID,       // Represents the act of returning nothing. Matching will `null` will return false.

    //
    //  Operands
    //
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERISK,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_CAROT,

    //
    //  Symbols
    //
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_PERIOD,
    TOKEN_APOSTROPHE,
    TOKEN_QUOTATION,
    TOKEN_NOT,           // Is used for negation. Use `!=` for checking for not-equal values.
    TOKEN_TYPE_DECL,     // Used when declaring a type of something, usually a variable or program.

    //
    //  Signs
    //
    TOKEN_EQUALS,             // Used during checking the equality between two values. (==, NOT =)
    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_GREATER_EQUALS,
    TOKEN_LESS_EQUALS,
    TOKEN_ASSIGN,             // Used explicitly for variable assignment, NOT equality checking.
    TOKEN_NOT_EQUALS,
    TOKEN_AND,

    //
    //  Keywords
    //
    TOKEN_IF,         // Comparison between two values.
    TOKEN_WHILE,      // Loops until the condition provided is true.
    TOKEN_FOR,        // Loops specific code for a certain amount of times.
    TOKEN_INCLUDE,    // Import third party libraries into your project to get more functionality.
    TOKEN_BREAK,      // Stops a loop regardless if the condition is met.
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_PROGRAM,    // Defines a new function, allowing code to be reusable.
    TOKEN_RETURN,     // Return a value from a program or loop statement. Does not stop loops directly.

    //
    //  Literals
    //
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,

    //
    //  Special
    //
    TOKEN_ERROR,    // When something goes wrong or when coming across an unexpected token. Better than returning NULL itself.
    TOKEN_EOF,      // Used when representing the lack of any more tokens in a file.
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

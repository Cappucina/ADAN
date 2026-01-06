#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static ErrorList* error_list;

static Token match_operator(Lexer* lex, char expected, TokenType if_match, TokenType if_no_match,
                            size_t start);

struct
{
    const char* name;
    TokenType type;
} keywords[] = {
    {"if", TOKEN_IF},           {"while", TOKEN_WHILE},       {"for", TOKEN_FOR},
    {"include", TOKEN_INCLUDE}, {"continue", TOKEN_CONTINUE}, {"program", TOKEN_PROGRAM},
    {"return", TOKEN_RETURN},   {"else", TOKEN_ELSE},         {"struct", TOKEN_STRUCT},
    {"break", TOKEN_BREAK}};

static bool is_identifier_start(char c)
{
    return (isalpha(c) || c == '_');
}

static bool is_identifier_body(char c)
{
    return (isalnum(c) || c == '_');
}

static TokenType is_keyword(const char* name, size_t length)
{
    if (!name) return TOKEN_IDENTIFIER;
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
    {
        if (strlen(keywords[i].name) == length && strncmp(name, keywords[i].name, length) == 0)
        {
            return keywords[i].type;
        }
    }

    return TOKEN_IDENTIFIER;
}

Lexer* create_lexer(const char* source, ErrorList* el)
{
    Lexer* new = (Lexer*)malloc(sizeof(Lexer));
    if (!new) return NULL;

    new->source = source;
    new->position = 0;
    new->line = 1;
    new->column = 1;
    new->length = strlen(source);
    error_list = el;
    return new;
}

void free_lexer(Lexer* lex)
{
    if (lex) free(lex);
    return;
}

char peek_char(Lexer* lex)
{
    if (lex->position >= lex->length)
    {
        return '\0';
    }
    return lex->source[lex->position];
}

char peek_next(Lexer* lex, size_t offset)
{
    return (lex->position + offset < lex->length) ? lex->source[lex->position + offset] : '\0';
}

char next_char(Lexer* lex)
{
    char current = lex->source[lex->position++];
    if (current == '\n')
    {
        lex->line++;
        lex->column = 1;
    }
    else
    {
        lex->column++;
    }

    return current;
}

Token create_token(Lexer* lex, const char* lexeme, size_t start, size_t length, TokenType type)
{
    Token tk = {.lexeme = lexeme,
                .start = start,
                .length = length,
                .line = lex->line,
                .column = lex->column,
                .type = type};
    return tk;
}

/**
 *
 * Various helper functions to help out majorly during
 *  the lexical scanning process.
 */
Token lex_identifier(Lexer* lex)
{
    size_t start = lex->position;
    next_char(lex);
    while (is_identifier_body(peek_char(lex)))
    {
        next_char(lex);
    }

    size_t length = lex->position - start;
    const char* lexeme = &lex->source[start];
    TokenType type = is_keyword(lexeme, length);
    return create_token(lex, lexeme, start, length, type);
}

Token lex_number(Lexer* lex)
{
    size_t start = lex->position;
    while (isdigit(peek_char(lex)))
    {
        next_char(lex);
    }

    if (peek_char(lex) == '.' && isdigit(peek_next(lex, 1)))
    {
        next_char(lex);
        while (isdigit(peek_char(lex)))
        {
            next_char(lex);
        }

        size_t length = lex->position - start;
        const char* lexeme = &lex->source[start];
        return create_token(lex, lexeme, start, length, TOKEN_FLOAT_LITERAL);
    }
    else
    {
        size_t length = lex->position - start;
        const char* lexeme = &lex->source[start];
        return create_token(lex, lexeme, start, length, TOKEN_INT_LITERAL);
    }
}

Token lex_string(Lexer* lex)
{
    size_t start = lex->position;
    next_char(lex);
    while (peek_char(lex) != '"' && peek_char(lex) != '\0')
    {
        next_char(lex);
    }

    if (peek_char(lex) == '\0')
    {
        // do error shit later
    }

    next_char(lex);
    size_t length = lex->position - start;
    const char* lexeme = &lex->source[start];
    return create_token(lex, lexeme, start, length, TOKEN_STRING);
}

Token lex_operator(Lexer* lex)
{
    size_t start = lex->position;
    char c = next_char(lex);
    switch (c)
    {
        case ':':
            return match_operator(lex, ':', TOKEN_TYPE_DECLARATOR, TOKEN_ERROR, start);
        case '&':
            return match_operator(lex, '&', TOKEN_AND, TOKEN_BITWISE_AND, start);
        case '=':
            return match_operator(lex, '=', TOKEN_EQUALS, TOKEN_ASSIGN, start);
        case '+':
            return match_operator(lex, '+', TOKEN_INCREMENT, TOKEN_ADD, start);
        case '-':
            return match_operator(lex, '-', TOKEN_DECREMENT, TOKEN_SUBTRACT, start);
        case '>':
            if (peek_char(lex) == '>')
            {
                next_char(lex);
                if (peek_char(lex) == '>')
                {
                    next_char(lex);
                    return create_token(lex, &lex->source[start], start, lex->position - start,
                                        TOKEN_BITWISE_ZERO_FILL_RIGHT_SHIFT);
                }
                return create_token(lex, &lex->source[start], start, lex->position - start,
                                    TOKEN_BITWISE_SIGNED_RIGHT_SHIFT);
            }
            else if (peek_char(lex) == '=')
            {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start,
                                    TOKEN_GREATER_EQUALS);
            }
            return create_token(lex, &lex->source[start], start, 1, TOKEN_GREATER);
        case '<':
            if (peek_char(lex) == '<')
            {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start,
                                    TOKEN_BITWISE_ZERO_FILL_LEFT_SHIFT);
            }
            else if (peek_char(lex) == '=')
            {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start,
                                    TOKEN_LESS_EQUALS);
            }
            return create_token(lex, &lex->source[start], start, 1, TOKEN_LESS);
        case '!':
            return match_operator(lex, '=', TOKEN_NOT_EQUALS, TOKEN_NOT, start);
        case '|':
            return match_operator(lex, '|', TOKEN_OR, TOKEN_BITWISE_OR, start);
        case '*':
            return match_operator(lex, '*', TOKEN_EXPONENT, TOKEN_MULTIPLY, start);
        case '/':
            return create_token(lex, &lex->source[start], start, 1, TOKEN_DIVIDE);
        case '%':
            return create_token(lex, &lex->source[start], start, 1, TOKEN_MODULO);
        case '^':
            return create_token(lex, &lex->source[start], start, 1, TOKEN_CARET);
        case '~':
            return create_token(lex, &lex->source[start], start, 1, TOKEN_BITWISE_NOT);
        case '.':
            if (peek_char(lex) == '.')
            {
                next_char(lex);
                if (peek_char(lex) == '.')
                {
                    next_char(lex);
                    return create_token(lex, &lex->source[start], start, lex->position - start,
                                        TOKEN_ELLIPSIS);
                }
            }
            return create_token(lex, &lex->source[start], start, lex->position - start,
                                TOKEN_PERIOD);
        default: {
            size_t length = lex->position - start;
            return create_token(lex, &lex->source[start], start, length, TOKEN_ERROR);
        }
    }
}

Token lex_char(Lexer* lex)
{
    size_t start = lex->position;
    next_char(lex);
    while (peek_char(lex) != '\'' && peek_char(lex) != '\0')
    {
        next_char(lex);
    }

    if (peek_char(lex) == '\0')
    {
        // do error shit later
    }

    next_char(lex);
    size_t length = lex->position - start;
    const char* lexeme = &lex->source[start];
    return create_token(lex, lexeme, start, length, TOKEN_CHAR);
}

void skip_whitespace(Lexer* lex)
{
    while (true)
    {
        while (true)
        {
            char pc = peek_char(lex);
            if (pc != ' ' && pc != '\t' && pc != '\r' && pc != '\n')
            {
                break;
            }
            next_char(lex);
        }
        char pc = peek_char(lex);
        if (pc == '/')
        {
            if (peek_next(lex, 1) == '/')
            {
                next_char(lex);
                next_char(lex);
                while (pc != '\n' && pc != '\0')
                {
                    next_char(lex);
                    pc = peek_char(lex);
                }
                continue;
            }
            else if (peek_next(lex, 1) == '*')
            {
                next_char(lex);
                next_char(lex);
                while (!(pc == '*' && peek_next(lex, 1) == '/') && pc != '\0')
                {
                    next_char(lex);
                    pc = peek_char(lex);
                }
                next_char(lex);
                next_char(lex);
                continue;
            }
        }
        break;
    }
}

static Token match_operator(Lexer* lex, char expected, TokenType if_match, TokenType if_no_match,
                            size_t start)
{
    char pc = peek_char(lex);
    if (pc == expected)
    {
        next_char(lex);
        size_t length = lex->position - start;
        return create_token(lex, &lex->source[start], start, length, if_match);
    }

    size_t length = lex->position - start;
    return create_token(lex, &lex->source[start], start, length, if_no_match);
}

static bool is_operator_start(char c)
{
    return strchr(":&=+->!|*/%^~.<", c) != NULL;
}

/**
 *
 * Tokenize keywords ("include", "if", "program", etc.),
 *  symbols ("::", "&&", "||", etc.), and more.
 *
 * Turning literal forms into their token version, then
 *  group them into an array to be handled by the parser.
 */

Token lex(Lexer* lx)
{
    skip_whitespace(lx);
    size_t start = lx->position;
    char c = peek_char(lx);
    if (c == '\0') {
        return create_token(lx, &lx->source[start], start, 0, TOKEN_EOF);
    } else if (is_identifier_start(c)) {
        return lex_identifier(lx);
    } else if (isdigit(c)) {
        return lex_number(lx);
    } else if (c == '"') {
        return lex_string(lx);
    } else if (c == '\'') {
        return lex_char(lx);
    } else if (c == '(') {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_LEFT_PAREN);
    } else if (c == ')') {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_RIGHT_PAREN);
    } else if (c == '[') {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_LEFT_BRACKET);
    } else if (c == ']') {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_RIGHT_BRACKET);
    } else if (c == '{') {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_LEFT_BRACE);
    } else if (c == '}') {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_RIGHT_BRACE);
    } else if (c == ';') {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_SEMICOLON);
    } else if (c == ',') {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_COMMA);
    } else if (is_operator_start(c)) {
        return lex_operator(lx);
    } else {
        next_char(lx);
        return create_token(lx, &lx->source[start], start, 1, TOKEN_ERROR);
    }
}

#include "lexer.h"

Keyword keywords[] = {
    {"if", TOKEN_IF},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"include", TOKEN_INCLUDE},
    {"continue", TOKEN_CONTINUE},
    {"program", TOKEN_PROGRAM},
    {"return", TOKEN_RETURN},
    {"else", TOKEN_ELSE},
    {"struct", TOKEN_STRUCT},
    {"break", TOKEN_BREAK}};

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

static Token match_operator(Lexer* lex, char expected, Tokens if_match, Tokens if_no_match, uint32_t start);

static bool is_ident_start(char c)
{
    return (isalpha(c) || c == '_');
}

static bool is_ident(char c)
{
    return (isalnum(c) || c == '_');
}

static Tokens is_keyword(const char* name, uint32_t length)
{
    if (!name) return TOKEN_IDENTIFIER;
    for (uint32_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
    {
        if (strlen(keywords[i].name) == length && strncmp(name, keywords[i].name, length) == 0)
        {
            return keywords[i].type;
        }
    }

    return TOKEN_IDENTIFIER;
}

Lexer* create_lexer(const char* source, ErrorList* el, const char* file)
{
    Lexer* new = (Lexer*)malloc(sizeof(Lexer));
    if (!new)
    {
        exit(-ENOMEM);
        return NULL;
    }

    new->source = source;
    new->position = 0;
    new->file = file;
    new->line = 1;
    new->column = 1;
    new->length = (uint32_t)strlen(source);
    new->errors = el;

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

char peek_next(Lexer* lex, uint32_t offset)
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

Token create_token(Lexer* lex, const char* lexeme, uint32_t start, uint32_t length, Tokens type)
{
    Token tk = {.lexeme = lexeme,
                .start = start,
                .length = length,
                .file = lex->file,
                .line = lex->line,
                .column = lex->column,
                .type = type};

    return tk;
}

Token lex_identifier(Lexer* lex)
{
    uint32_t start = lex->position;
    if (!is_ident_start(peek_char(lex)))
    {
        next_char(lex);
    }
    else
    {
        next_char(lex);
        while (is_ident(peek_char(lex)))
        {
            next_char(lex);
        }
    }

    uint32_t length = lex->position - start;
    const char* lexeme = &lex->source[start];
    Tokens type = is_keyword(lexeme, length);
    return create_token(lex, lexeme, start, length, type);
}

Token lex_number(Lexer* lex)
{
    uint32_t start = lex->position;
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

        uint32_t length = lex->position - start;
        const char* lexeme = &lex->source[start];
        return create_token(lex, lexeme, start, length, TOKEN_FLOAT_LITERAL);
    }
    else
    {
        uint32_t length = lex->position - start;
        const char* lexeme = &lex->source[start];
        return create_token(lex, lexeme, start, length, TOKEN_INT_LITERAL);
    }
}

Token lex_string(Lexer* lex)
{
    uint32_t start = lex->position;
    uint32_t start_line = lex->line;
    uint32_t start_column = lex->column;
    next_char(lex);
    while (peek_char(lex) != '"' && peek_char(lex) != '\0')
    {
        next_char(lex);
    }

    if (peek_char(lex) == '\0')
    {
        error(lex->errors, "source", start_line, start_column, LEXER, "Unclosed string literal");
    }
    else
    {
        next_char(lex);
    }

    uint32_t length = lex->position - start;
    const char* lexeme = &lex->source[start];
    return create_token(lex, lexeme, start, length, TOKEN_STRING);
}

Token lex_operator(Lexer* lex)
{
    uint32_t start = lex->position;
    char c = next_char(lex);
    switch (c)
    {
        case ':':
            return match_operator(lex, ':', TOKEN_TYPE_DECL, TOKEN_ERROR, start);
        case '&':
            if (peek_char(lex) == '=') {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start, TOKEN_BITWISE_AND);
            }
            return match_operator(lex, '&', TOKEN_AND, TOKEN_BITWISE_AND, start);
        case '|':
            if (peek_char(lex) == '=') {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start, TOKEN_BITWISE_OR);
            }
            return match_operator(lex, '|', TOKEN_OR, TOKEN_BITWISE_OR, start);
        case '^':
            if (peek_char(lex) == '=') {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start, TOKEN_BITWISE_XOR);
            }
            return create_token(lex, &lex->source[start], start, 1, TOKEN_BITWISE_XOR);
        case '=':
            return match_operator(lex, '=', TOKEN_EQUALITY, TOKEN_EQUAL, start);
        case '+':
            if (peek_char(lex) == '=') {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start, TOKEN_ADD_EQUALS);
            }
            return match_operator(lex, '+', TOKEN_ADD_ADD, TOKEN_ADD, start);
        case '-':
            if (peek_char(lex) == '=') {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start, TOKEN_SUB_EQUALS);
            }
            return match_operator(lex, '-', TOKEN_SUB_SUB, TOKEN_SUB, start);
        case '*':
            if (peek_char(lex) == '=') {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start, TOKEN_MUL_EQUALS);
            }
            return match_operator(lex, '*', TOKEN_EXPONENT, TOKEN_MUL, start);
        case '/':
            if (peek_char(lex) == '=') {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start, TOKEN_DIV_EQUALS);
            }
            return create_token(lex, &lex->source[start], start, 1, TOKEN_DIV);
        case '%':
            if (peek_char(lex) == '=') {
                next_char(lex);
                return create_token(lex, &lex->source[start], start, lex->position - start, TOKEN_MOD_EQUALS);
            }
            return create_token(lex, &lex->source[start], start, 1, TOKEN_MOD);
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
            uint32_t length = lex->position - start;
            char invalid_char = lex->source[start];
            error(lex->errors, "source", lex->line, lex->column, LEXER,
                  "Invalid operator character '%c'", invalid_char);
            return create_token(lex, &lex->source[start], start, length, TOKEN_ERROR);
        }
    }
}

Token lex_char(Lexer* lex)
{
    uint32_t start = lex->position;
    uint32_t start_line = lex->line;
    uint32_t start_column = lex->column;
    next_char(lex);
    while (peek_char(lex) != '\'' && peek_char(lex) != '\0')
    {
        next_char(lex);
    }

    if (peek_char(lex) == '\0')
    {
        error(lex->errors, "source", start_line, start_column, LEXER, "Unclosed character literal");
    }
    else
    {
        next_char(lex);
    }

    uint32_t length = lex->position - start;
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

static Token match_operator(Lexer* lex, char expected, Tokens if_match, Tokens if_no_match,
                            uint32_t start)
{
    char pc = peek_char(lex);
    if (pc == expected)
    {
        next_char(lex);
        uint32_t length = lex->position - start;
        return create_token(lex, &lex->source[start], start, length, if_match);
    }

    uint32_t length = lex->position - start;
    return create_token(lex, &lex->source[start], start, length, if_no_match);
}

static bool is_operator_start(char c)
{
    return strchr(":&=+->!|*/%^~.<", c) != NULL;
}

Token lex(Lexer* lexer)
{
    skip_whitespace(lexer);
    uint32_t start = lexer->position;
    char c = peek_char(lexer);
    if (c == '\0')
    {
        return create_token(lexer, &lexer->source[start], start, 0, TOKEN_EOF);
    }
    else if (is_ident_start(c))
    {
        return lex_identifier(lexer);
    }
    else if (isdigit(c))
    {
        return lex_number(lexer);
    }
    else if (c == '"')
    {
        return lex_string(lexer);
    }
    else if (c == '\'')
    {
        return lex_char(lexer);
    }
    else if (c == '(')
    {
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_OPEN_PAREN);
    }
    else if (c == ')')
    {
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_CLOSE_PAREN);
    }
    else if (c == '[')
    {
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_OPEN_BRACKET);
    }
    else if (c == ']')
    {
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_CLOSE_BRACKET);
    }
    else if (c == '{')
    {
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_OPEN_BRACE);
    }
    else if (c == '}')
    {
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_CLOSE_BRACE);
    }
    else if (c == ';')
    {
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_SEMICOLON);
    }
    else if (c == ',')
    {
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_COMMA);
    }
    else if (is_operator_start(c))
    {
        return lex_operator(lexer);
    }
    else
    {
        error(lexer->errors, "source", lexer->line, lexer->column, LEXER, "Unexpected character '%c'", c);
        next_char(lexer);
        return create_token(lexer, &lexer->source[start], start, 1, TOKEN_ERROR);
    }
}
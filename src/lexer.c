#include <stdio.h>
#include <stdlib.h>
#include <lexer.h>
#include <string.h>

Lexer *create_lexer(const char *src) {
    Lexer *new_lex = (Lexer*) malloc(sizeof(Lexer));
    new_lex->src = src;
    
    new_lex->position = 0;
    new_lex->line = 1;

    return new_lex;
}

Token *make_token(Lexer *lexer, TokenType type, const char *text) {
    Token *new_token = malloc(sizeof(Token));
    if (!new_token) return NULL;

    new_token->type = type;
    new_token->text = strdup(text);
    new_token->line = lexer->line;

    advance(lexer);
    return new_token;
}

char *capture_word(Lexer *lexer) {
    int start = lexer->position;
    while ((lexer->src[lexer->position] >= 'a' && lexer->src[lexer->position] <= 'z') || (lexer->src[lexer->position] >= 'A' && lexer->src[lexer->position] <= 'Z') || is_digit(lexer->src[lexer->position]) || lexer->src[lexer->position] == '_') {
        advance(lexer);
    }

    int length = lexer->position - start;
    char *word = malloc(length + 1);

    strncpy(word, lexer->src + start, length);
    word[length] = '\0';

    return word;
}

Token *next_token(Lexer *lexer) {
    while (is_whitespace(lexer->src[lexer->position])) advance(lexer);
    char c = lexer->src[lexer->position];

    if (c == '\0') {
        Token *token = malloc(sizeof(Token));
        token->type = TOKEN_EOF;
        token->text = NULL;
        token->line = lexer->line;
        
        return token;
    }

    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        char *word = capture_word(lexer);
        typedef struct { const char *text; TokenType type; } Keyword;
        Keyword keywords[] = {
            // 
            //  Types
            // 
            {"int", TOKEN_INT},
            {"float", TOKEN_FLOAT},
            {"string", TOKEN_STRING},
            {"boolean", TOKEN_BOOLEAN},
            {"array", TOKEN_ARRAY},
            {"char", TOKEN_CHAR},
            {"null", TOKEN_NULL},
            {"void", TOKEN_VOID},
            
            // 
            //  Keywords
            // 
            {"if", TOKEN_IF},
            {"while", TOKEN_WHILE},
            {"for", TOKEN_FOR},
            {"include", TOKEN_INCLUDE},
            {"break", TOKEN_BREAK},
            {"true", TOKEN_TRUE},
            {"false", TOKEN_FALSE},
            {"program", TOKEN_PROGRAM}
        };

        TokenType type = TOKEN_IDENTIFIER;
        for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
            if (strcmp(word, keywords[i].text) == 0) {
                type = keywords[i].type;
                break;
            }
        }

        Token *token = malloc(sizeof(Token));
        token->type = type;
        token->text = word;
        token->line = lexer->line;
    
        return token;
    }

    switch(c) {
        case '+': return make_token(lexer, TOKEN_PLUS, "+");
        case '-': return make_token(lexer, TOKEN_MINUS, "-");
        case '*': return make_token(lexer, TOKEN_ASTERISK, "*");
        case '/': return make_token(lexer, TOKEN_SLASH, "/");
        case '%': return make_token(lexer, TOKEN_PERCENT, "%");
        case '^': return make_token(lexer, TOKEN_CAROT, "^");
        case '(': return make_token(lexer, TOKEN_LPAREN, "(");
        case ')': return make_token(lexer, TOKEN_RPAREN, ")");
        case '{': return make_token(lexer, TOKEN_LBRACE, "{");
        case '}': return make_token(lexer, TOKEN_RBRACE, "}");
        case ';': return make_token(lexer, TOKEN_SEMICOLON, ";");
        case ',': return make_token(lexer, TOKEN_COMMA, ",");
        case '.': return make_token(lexer, TOKEN_PERIOD, ".");
        case '\'': return make_token(lexer, TOKEN_APOSTROPHE, "'");
        case '"': return make_token(lexer, TOKEN_QUOTATION, "\"");
    }

    advance(lexer);
    return NULL;
}

void free_token(Token *token) {
    if (NULL == token) return;
    if (token->text != NULL) {
        free(token->text);
        token->text = NULL;
    }
}

void print_token(Token *token) {
    printf("Token line: %d | Token type: %d | Token text: '%s'\n",
       token->line, token->type, token->text);
}
#include <stdio.h>
#include <stdlib.h>
#include <lexer.h>

Lexer *create_lexer(const char *src) {
    Lexer *new_lex = (Lexer*) malloc(sizeof(Lexer));
    new_lex->src = src;
    
    new_lex->position = 0;
    new_lex->line = 1;

    return new_lex;
}

Token *next_token(Lexer *lexer) {
    
}

void free_token(Token *token) {
    if (NULL == token) return;
    if (token->text != NULL) {
        free(token->text);
        token->text = NULL;
    }
}

// DEBUG

void print_token(Token *token) {
    printf("Token line: %d | Token type: %d | Token text: '%s'\n",
       token->line, token->type, token->text);
}
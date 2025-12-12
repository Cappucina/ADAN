#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include <string.h>

Lexer* create_lexer(const char *src) {
	Lexer* new_lex = (Lexer*) malloc(sizeof(Lexer));
	new_lex->src = src;
	
	new_lex->position = 0;
	new_lex->line = 1;

	return new_lex;
}

Token* make_token(Lexer* lexer, TokenType type, const char* texts[], int count) {
	Token *new_token = malloc(sizeof(Token));
	if (!new_token) return NULL;

	int total_len = 0;
	for (int i = 0; i < count; i++) {
		total_len += strlen(texts[i]);
	}

	char* combined = malloc(total_len + 1);
	combined[0] = '\0';

	for (int i = 0; i < count; i++) {
		strcat(combined, texts[i]);
	}

	new_token->type = type;
	new_token->text = combined;
	new_token->line = lexer->line;
	new_token->column = lexer->position;

	for (int i = 0; i < total_len; i++) {
		advance(lexer);
	}

	return new_token;
}

char* capture_word(Lexer* lexer) {
	int start = lexer->position;
	while ((lexer->src[lexer->position] >= 'a' && lexer->src[lexer->position] <= 'z') || (lexer->src[lexer->position] >= 'A' && lexer->src[lexer->position] <= 'Z') || is_digit(lexer->src[lexer->position]) || lexer->src[lexer->position] == '_') {
		advance(lexer);
	}

	int length = lexer->position - start;
	char* word = malloc(length + 1);

	strncpy(word, lexer->src + start, length);
	word[length] = '\0';

	return word;
}

Token* next_token(Lexer* lexer) {
	while (is_whitespace(lexer->src[lexer->position])) advance(lexer);
	
	char c = lexer->src[lexer->position];
	char next = lexer->src[lexer->position + 1];

	if (c == '\0') {
		Token *token = malloc(sizeof(Token));
		token->type = TOKEN_EOF;
		token->text = NULL;
		token->line = lexer->line;
		token->column = lexer->position;
		
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
			{"program", TOKEN_PROGRAM},
			{"return", TOKEN_RETURN},
			{"else", TOKEN_ELSE},
		};

		TokenType type = TOKEN_IDENTIFIER;
		for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
			if (strcmp(word, keywords[i].text) == 0) {
				type = keywords[i].type;
				break;
			}
		}

		Token* token = malloc(sizeof(Token));
		token->type = type;
		token->text = word;
		token->line = lexer->line;
		token->column = lexer->position - strlen(word);
	
		return token;
	}

	if (c == '"') {
		advance(lexer);
		int start = lexer->position;

		while (lexer->src[lexer->position] != '"' && lexer->src[lexer->position] != '\0') advance(lexer);

		int length = lexer->position - start;
		char* text = malloc(length + 1);
		strncpy(text, lexer->src + start, length);
		text[length] = '\0';

		if (lexer->src[lexer->position] == '"') advance(lexer);

		Token* token = malloc(sizeof(Token));
		token->type = TOKEN_STRING;
		token->text = text;
		token->line = lexer->line;
		token->column = start - 1;

		return token;
	}

	if (c == '>' && next == '=') return make_token(lexer, TOKEN_GREATER_EQUALS, (const char*[]){">", "="}, 2);
	if (c == '<' && next == '=') return make_token(lexer, TOKEN_LESS_EQUALS, (const char*[]){"<", "="}, 2);
	if (c == '!' && next == '=') return make_token(lexer, TOKEN_NOT_EQUALS, (const char*[]){"!", "="}, 2);
	if (c == '&' && next == '&') return make_token(lexer, TOKEN_AND, (const char*[]){"&", "&"}, 2);
	if (c == '=' && next == '=') return make_token(lexer, TOKEN_EQUALS, (const char*[]){"=", "="}, 2);
	if (c == ':' && next == ':') return make_token(lexer, TOKEN_TYPE_DECL, (const char*[]){":", ":"}, 2);
	if (c == '+' && next == '+') return make_token(lexer, TOKEN_INCREMENT, (const char*[]){"+", "+"}, 2);
	if (c == '-' && next == '-') return make_token(lexer, TOKEN_DECREMENT, (const char*[]){"-", "-"}, 2);
	if (c == '/' && next == '/') return make_token(lexer, TOKEN_SINGLE_COMMENT, (const char*[]){"/", "/"}, 2);

	switch(c) {
		case '+': return make_token(lexer, TOKEN_PLUS, (const char*[]){"+"}, 1);
		case '-': return make_token(lexer, TOKEN_MINUS, (const char*[]){"-"}, 1);
		case '*': return make_token(lexer, TOKEN_ASTERISK, (const char*[]){"*"}, 1);
		case '/': return make_token(lexer, TOKEN_SLASH, (const char*[]){"/"}, 1);
		case '%': return make_token(lexer, TOKEN_PERCENT, (const char*[]){"%"}, 1);
		case '^': return make_token(lexer, TOKEN_CAROT, (const char*[]){"^"}, 1);
		case '(': return make_token(lexer, TOKEN_LPAREN, (const char*[]){"("}, 1);
		case ')': return make_token(lexer, TOKEN_RPAREN, (const char*[]){")"}, 1);
		case '{': return make_token(lexer, TOKEN_LBRACE, (const char*[]){"{"}, 1);
		case '}': return make_token(lexer, TOKEN_RBRACE, (const char*[]){"}"}, 1);
		case ';': return make_token(lexer, TOKEN_SEMICOLON, (const char*[]){";"}, 1);
		case ',': return make_token(lexer, TOKEN_COMMA, (const char*[]){","}, 1);
		case '.': return make_token(lexer, TOKEN_PERIOD, (const char*[]){"."}, 1);
		case '\'': return make_token(lexer, TOKEN_APOSTROPHE, (const char*[]){"'"}, 1);
		case '>': return make_token(lexer, TOKEN_GREATER, (const char*[]){">"}, 1);
		case '<': return make_token(lexer, TOKEN_LESS, (const char*[]){"<"}, 1);
		case '!': return make_token(lexer, TOKEN_NOT, (const char*[]){"!"}, 1);
		case '=': return make_token(lexer, TOKEN_ASSIGN, (const char*[]){"="}, 1);
	}

	if (is_digit(c)) {
		int start = lexer->position;
		while (is_digit(lexer->src[lexer->position])) advance(lexer);

		// 
		//  Handle floating point and signed integer literals.
		// 
		if (lexer->src[lexer->position] == '.') {
			advance(lexer);

			while (is_digit(lexer->src[lexer->position])) advance(lexer);
			int length = lexer->position - start;
			char* text = malloc(length + 1);
			
			strncpy(text, lexer->src + start, length);
			text[length] = '\0';

			Token* token = malloc(sizeof(Token));
			token->type = TOKEN_FLOAT_LITERAL;
			token->text = text;
			token->line = lexer->line;
			token->column = start;

			return token;
		} else {
			int length = lexer->position - start;
			char* text = malloc(length + 1);

			strncpy(text, lexer->src + start, length);
			text[length] = '\0';

			Token* token = malloc(sizeof(Token));
			token->type = TOKEN_INT_LITERAL;
			token->text = text;
			token->line = lexer->line;
			token->column = start;
			
			return token;
		}
	}

	advance(lexer);
	return NULL;
}

void free_token(Token* token) {
	if (NULL == token) return;
	if (token->text != NULL) {
		free(token->text);
		token->text = NULL;
	}
	free(token);
}

void print_token(Token* token) {
	printf("Token line: %d | Token type: %d | Token text: '%s'\n",
	   token->line, token->type, token->text);
}
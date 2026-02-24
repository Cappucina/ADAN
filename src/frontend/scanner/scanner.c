#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "../../macros.h"
#include "../../helper.h"
#include "scanner.h"

const Keyword keywords[] = {
    {"fun", TOKEN_FUN},
    {"import", TOKEN_IMPORT},
    {"set", TOKEN_SET},
    {"return", TOKEN_RETURN},

    {"string", TOKEN_STRING_TYPE},
    {"i32", TOKEN_I32_TYPE},
    {"i64", TOKEN_I64_TYPE},
    {"u32", TOKEN_U32_TYPE},
    {"u64", TOKEN_U64_TYPE},
    {"void", TOKEN_VOID_TYPE},
};

Scanner* scanner_init(char* source)
{
	Scanner* scanner = (Scanner*)calloc(1, sizeof(Scanner));
	if (!scanner)
	{
		fprintf(stderr, "No memory left to allocate for a new scanner! (Error)\n");
		return NULL;
	}
	scanner->position = 0;
	scanner->source = source;
	scanner->length = strlen(source);
	scanner->start = 0;
	scanner->column = 1;
	scanner->line = 1;
	return scanner;
}

void scanner_free(Scanner* scanner)
{
	if (!scanner)
	{
		fprintf(stderr, "No scanner provided; nothing to free! (Error)\n");
		return;
	}
	free(scanner);
}

char peek(Scanner* scanner)
{
	if (scanner->position >= scanner->length)
		return '\0';
	return scanner->source[scanner->position];
}

char peek_next(Scanner* scanner)
{
	if (scanner->position + 1 >= scanner->length)
		return '\0';
	return scanner->source[scanner->position + 1];
}

char advance(Scanner* scanner)
{
	if (scanner->position >= scanner->length)
		return '\0';
	char curr = scanner->source[scanner->position++];
	if (curr == '\n')
	{
		scanner->line++;
		scanner->column = 1;
	}
	else
	{
		scanner->column++;
	}
	return curr;
}

bool is_at_end(Scanner* scanner)
{
	return scanner->position >= scanner->length;
}

bool is_alpha(const char c)
{
	return isalpha((unsigned char)c) != 0;
}

bool is_digit(const char c)
{
	return isdigit((unsigned char)c) != 0;
}

bool is_alphanumeric(const char c)
{
	return isalnum((unsigned char)c) != 0;
}

bool is_whitespace(const char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

TokenType is_keyword(const char* keyword)
{
	for (size_t i = 0; i < ARRAY_LENGTH(keywords); i++)
	{
		if (strcmp(keyword, keywords[i].word) == 0)
		{
			return keywords[i].type;
		}
	}
	return TOKEN_IDENT;
}

void skip_spaces(Scanner* scanner)
{
	while (!is_at_end(scanner) && is_whitespace(peek(scanner)))
	{
		advance(scanner);
	}
}

Token* scan_next_token(Scanner* scanner)
{
	skip_spaces(scanner);
	if (is_at_end(scanner))
	{
		return make_token(TOKEN_EOF, scanner->column, scanner->line, NULL, 0);
	}

	scanner->start = scanner->position;
	char current = peek(scanner);
	if (is_alpha(current) || current == '_')
	{
		size_t start_pos = scanner->position;
		size_t start_col = scanner->column;
		size_t start_line = scanner->line;

		while (is_alphanumeric(peek(scanner)) || peek(scanner) == '_')
		{
			advance(scanner);
		}

		size_t length = scanner->position - start_pos;
		char* lexeme = clone_string(scanner->source + start_pos, length);
		TokenType type = is_keyword(lexeme);
		return make_token(type, start_col, start_line, lexeme, length);
	}

	if (is_digit(current))
	{
		size_t start_pos = scanner->position;
		size_t start_col = scanner->column;
		size_t start_line = scanner->line;

		while (is_digit(peek(scanner)))
		{
			advance(scanner);
		}

		if (peek(scanner) == '.' && is_digit(peek_next(scanner)))
		{
			advance(scanner);
			while (is_digit(peek(scanner)))
			{
				advance(scanner);
			}
		}

		size_t length = scanner->position - start_pos;
		char* lexeme = clone_string(scanner->source + start_pos, length);
		return make_token(TOKEN_NUMBER, start_col, start_line, lexeme, length);
	}

	if (current == '"')
	{
		size_t start_pos = scanner->position;
		size_t start_col = scanner->column;
		size_t start_line = scanner->line;

		advance(scanner);

		while (!is_at_end(scanner) && peek(scanner) != '"')
		{
			if (peek(scanner) == '\\' && !is_at_end(scanner))
			{
				advance(scanner);
				if (!is_at_end(scanner))
				{
					advance(scanner);
				}
			}
			else
			{
				advance(scanner);
			}
		}

		if (is_at_end(scanner))
		{
			printf("Unterminated string at line %zu, column %zu\n", start_line,
			       start_col);
			return make_token(TOKEN_EOF, start_col, start_line, NULL, 0);
		}

		advance(scanner);

		size_t length = scanner->position - start_pos;
		char* lexeme = clone_string(scanner->source + start_pos, length);
		return make_token(TOKEN_STRING, start_col, start_line, lexeme, length);
	}

	if (current == '/' && peek_next(scanner) == '/')
	{
		while (!is_at_end(scanner) && peek(scanner) != '\n')
		{
			advance(scanner);
		}
		return scan_next_token(scanner);
	}

	if (current == '/' && peek_next(scanner) == '*')
	{
		advance(scanner);
		advance(scanner);

		while (!is_at_end(scanner))
		{
			if (peek(scanner) == '*' && peek_next(scanner) == '/')
			{
				advance(scanner);
				advance(scanner);
				break;
			}
			advance(scanner);
		}
		return scan_next_token(scanner);
	}

	size_t token_col = scanner->column;
	size_t token_line = scanner->line;

	switch (current)
	{
		case '(':
			advance(scanner);
			return make_token(TOKEN_LPAREN, token_col, token_line, clone_string("(", 1),
			                  1);
		case ')':
			advance(scanner);
			return make_token(TOKEN_RPAREN, token_col, token_line, clone_string(")", 1),
			                  1);
		case '{':
			advance(scanner);
			return make_token(TOKEN_LBRACE, token_col, token_line, clone_string("{", 1),
			                  1);
		case '}':
			advance(scanner);
			return make_token(TOKEN_RBRACE, token_col, token_line, clone_string("}", 1),
			                  1);
		case ':':
			advance(scanner);
			return make_token(TOKEN_COLON, token_col, token_line, clone_string(":", 1),
			                  1);
		case ';':
			advance(scanner);
			return make_token(TOKEN_SEMICOLON, token_col, token_line,
			                  clone_string(";", 1), 1);
		case '=':
			advance(scanner);
			return make_token(TOKEN_EQUALS, token_col, token_line, clone_string("=", 1),
			                  1);
		case ',':
			advance(scanner);
			return make_token(TOKEN_COMMA, token_col, token_line, clone_string(",", 1),
			                  1);
		default:
			advance(scanner);
			printf("Unexpected character '%c' at line %zu, column %zu\n", current,
			       token_line, token_col);
			return scan_next_token(scanner);
	}
}
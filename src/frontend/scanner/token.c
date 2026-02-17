#include <stdlib.h>
#include <stdio.h>

#include "token.h"

void token_stream_free(Token* tokens)
{
	if (!tokens)
		return;
	for (size_t i = 0; tokens[i].type != TOKEN_EOF; i++) {
		free(tokens[i].lexeme);
	}
	free(tokens);
}

Token* make_token(TokenType type, size_t column, size_t line, char* lexeme, size_t length)
{
	// @todo Implement later.
}
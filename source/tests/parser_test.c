#include "parse/parser.h"
#include "lex/lexer.h"

#include "buffer.h"
#include "test.h"

static int test_create_parser(void)
{
    Buffer* buffer = buffer_create(sizeof(Token));
    Analyzer* parser = create_parser(buffer, create_errors());
    ASSERT_NOT_NULL(parser, "Parser should be created successfully");
    free_parser(parser);
    return 0;
}

static int test_free_parser(void)
{
    Buffer* buffer = buffer_create(sizeof(Token));
    Analyzer* parser = create_parser(buffer, create_errors());
    ASSERT_NOT_NULL(parser, "Parser should be created successfully");
    free_parser(parser);
    return 0;
}

static int test_match_token(void)
{
    Buffer* buffer = buffer_create(sizeof(Token));

    Token* token = malloc(sizeof(Token));

    token->column = 1;
    token->length = 3;
    token->file = "test.c";
    token->line = 1;
    token->type = TOKEN_IDENTIFIER;
    token->lexeme = "var";
    buffer_push(buffer, token);

    token->column = 5;
    token->length = 1;
    token->file = "test.c";
    token->line = 1;
    token->type = TOKEN_EQUALS;
    token->lexeme = "=";
    buffer_push(buffer, token);

    token->column = 7;
    token->length = 2;
    token->file = "test.c";
    token->line = 1;
    token->type = TOKEN_INT_LITERAL;
    token->lexeme = "42";
    buffer_push(buffer, token);

    free(token);

    Analyzer* parser = create_parser(buffer, create_errors());
    ASSERT_NOT_NULL(parser, "Parser should be created successfully");

    ASSERT_TRUE(match(parser, TOKEN_IDENTIFIER), "Should match TOKEN_IDENTIFIER");
    ASSERT_FALSE(match(parser, TOKEN_EOF), "Should not match TOKEN_EOF");

    advance(parser);

    ASSERT_TRUE(match(parser, TOKEN_EQUALS), "Should match TOKEN_EQUALS");
    ASSERT_FALSE(match(parser, TOKEN_EOF), "Should not match TOKEN_EOF");

    advance(parser);

    ASSERT_TRUE(match(parser, TOKEN_INT_LITERAL), "Should match TOKEN_INT_LITERAL");
    ASSERT_FALSE(match(parser, TOKEN_EOF), "Should not match TOKEN_EOF");

    free_parser(parser);
    return 0;
}

static int test_advance_parser(void)
{
    Buffer* buffer = buffer_create(sizeof(Token));

    Token* addToken = malloc(sizeof(Token));

    addToken->column = 1;
    addToken->length = 3;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_IDENTIFIER;
    addToken->lexeme = "var";
    buffer_push(buffer, addToken);

    addToken->column = 5;
    addToken->length = 1;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_EQUALS;
    addToken->lexeme = "=";
    buffer_push(buffer, addToken);

    addToken->column = 7;
    addToken->length = 2;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_INT_LITERAL;
    addToken->lexeme = "42";
    buffer_push(buffer, addToken);

    free(addToken);

    Analyzer* parser = create_parser(buffer, create_errors());
    ASSERT_NOT_NULL(parser, "Parser should be created successfully");

    Token tmp_token = advance(parser);
    Token* token = &tmp_token;
    ASSERT_NOT_NULL(token, "Should advance to second token");
    ASSERT_EQ(token->type, TOKEN_EQUALS, "WShould advance to TOKEN_EQUALS");

    tmp_token = current_token(parser);
    token = &tmp_token;
    ASSERT_NOT_NULL(token, "Should get current token");
    ASSERT_EQ(token->type, TOKEN_EQUALS, "Current token should still be TOKEN_EQUALS");

    tmp_token = advance(parser);
    token = &tmp_token;
    ASSERT_NOT_NULL(token, "Should advance to third token");
    ASSERT_EQ(token->type, TOKEN_INT_LITERAL, "Should advance to TOKEN_INT_LITERAL");

    tmp_token = current_token(parser);
    token = &tmp_token;
    ASSERT_NOT_NULL(token, "Should get current token");
    ASSERT_EQ(token->type, TOKEN_INT_LITERAL, "Current token should still be TOKEN_INT_LITERAL");

    free_parser(parser);
    return 0;
}

static int test_peek_token(void)
{
    Buffer* buffer = buffer_create(sizeof(Token));

    Token* addToken = malloc(sizeof(Token));

    addToken->column = 1;
    addToken->length = 3;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_IDENTIFIER;
    addToken->lexeme = "var";
    buffer_push(buffer, addToken);

    addToken->column = 5;
    addToken->length = 1;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_EQUALS;
    addToken->lexeme = "=";
    buffer_push(buffer, addToken);

    addToken->column = 7;
    addToken->length = 2;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_INT_LITERAL;
    addToken->lexeme = "42";
    buffer_push(buffer, addToken);

    free(addToken);

    Analyzer* parser = create_parser(buffer, create_errors());
    ASSERT_NOT_NULL(parser, "Parser should be created successfully");

    Token tmp_token = peek(parser);
    Token* token = &tmp_token;
    ASSERT_NOT_NULL(token, "Should peek second token");
    ASSERT_EQ(token->type, TOKEN_EQUALS, "Should peek TOKEN_EQUALS");

    tmp_token = current_token(parser);
    token = &tmp_token;
    ASSERT_NOT_NULL(token, "Should get current token");
    ASSERT_EQ(token->type, TOKEN_IDENTIFIER, "Current token should still be TOKEN_IDENTIFIER");

    free_parser(parser);
    return 0;
}

static int test_current_token(void)
{
    Buffer* buffer = buffer_create(sizeof(Token));

    Token* addToken = malloc(sizeof(Token));

    addToken->column = 1;
    addToken->length = 3;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_IDENTIFIER;
    addToken->lexeme = "var";
    buffer_push(buffer, addToken);

    addToken->column = 5;
    addToken->length = 1;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_EQUALS;
    addToken->lexeme = "=";
    buffer_push(buffer, addToken);

    addToken->column = 7;
    addToken->length = 2;
    addToken->file = "test.c";
    addToken->line = 1;
    addToken->type = TOKEN_INT_LITERAL;
    addToken->lexeme = "42";
    buffer_push(buffer, addToken);

    free(addToken);

    Analyzer* parser = create_parser(buffer, create_errors());
    ASSERT_NOT_NULL(parser, "Parser should be created successfully");

    Token tmp_token = current_token(parser);
    Token* token = &tmp_token;  // did you edit this? it returns Token not a pointer to Token. This is the easiest solution. I don't like it becuse it has another new thing for the stack.
    ASSERT_NOT_NULL(token, "Should get current token");
    ASSERT_EQ(token->type, TOKEN_IDENTIFIER, "Should get TOKEN_IDENTIFIER");

    advance(parser);

    tmp_token = current_token(parser);
    token = &tmp_token;
    ASSERT_NOT_NULL(token, "Should get current token");
    ASSERT_EQ(token->type, TOKEN_EQUALS, "Should get TOKEN_EQUALS");

    free_parser(parser);
    return 0;
}

int run_parser_tests(TestSuite* suite)
{
    if (!suite)
    {
        return -1;
    }

    test_suite_run_test(suite, "parser_create_parser", test_create_parser);
    test_suite_run_test(suite, "parser_free_parser", test_free_parser);
    test_suite_run_test(suite, "parser_match_token", test_match_token);
    test_suite_run_test(suite, "parser_advance_parser", test_advance_parser);
    test_suite_run_test(suite, "parser_peek_token", test_peek_token);
    test_suite_run_test(suite, "parser_current_token", test_current_token);

    return 0;
}

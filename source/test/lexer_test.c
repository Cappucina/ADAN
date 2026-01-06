#include "lexer.h"

#include <stdlib.h>
#include <string.h>

#include "diagnostic.h"
#include "test.h"

static int test_create_lexer(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("int x = 5;", errors);

    ASSERT_NOT_NULL(lex, "create_lexer should not return NULL");
    ASSERT_EQ(lex->position, 0, "Initial position should be 0");
    ASSERT_EQ(lex->line, 1, "Initial line should be 1");
    ASSERT_EQ(lex->column, 1, "Initial column should be 1");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_peek_char(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("hello", errors);

    ASSERT_NOT_NULL(lex, "create_lexer should not return NULL");
    char c = peek_char(lex);
    ASSERT_EQ(c, 'h', "peek_char should return first character");
    ASSERT_EQ(lex->position, 0, "peek_char should not advance position");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_next_char(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("ab", errors);

    ASSERT_NOT_NULL(lex, "create_lexer should not return NULL");
    char c1 = next_char(lex);
    ASSERT_EQ(c1, 'a', "next_char should return first character");
    ASSERT_EQ(lex->position, 1, "next_char should advance position");
    ASSERT_EQ(lex->column, 2, "column should increment");

    char c2 = next_char(lex);
    ASSERT_EQ(c2, 'b', "next_char should return second character");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_peek_next(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("hello", errors);

    ASSERT_NOT_NULL(lex, "create_lexer should not return NULL");
    char c = peek_next(lex, 1);
    ASSERT_EQ(c, 'e', "peek_next with offset 1 should return second character");
    ASSERT_EQ(lex->position, 0, "peek_next should not advance position");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_next_char_newline(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("a\nb", errors);

    ASSERT_NOT_NULL(lex, "create_lexer should not return NULL");
    next_char(lex);
    ASSERT_EQ(lex->column, 2, "column should be 2 after first character");

    next_char(lex);
    ASSERT_EQ(lex->line, 2, "line should increment on newline");
    ASSERT_EQ(lex->column, 1, "column should reset on newline");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_create_token(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("int x;", errors);

    Token tok = create_token(lex, "int", 0, 3, TOKEN_INT);
    ASSERT_EQ(tok.type, TOKEN_INT, "token type should match");
    ASSERT_NOT_NULL(tok.lexeme, "token lexeme should not be NULL");
    ASSERT_EQ(tok.length, 3, "token length should be 3");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_identifier(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("myVariable", errors);

    Token tok = lex_identifier(lex);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER, "token should be identifier");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_keyword_if(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("if", errors);

    Token tok = lex_identifier(lex);
    ASSERT_EQ(tok.type, TOKEN_IF, "keyword 'if' should be recognized");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_keyword_while(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("while", errors);

    Token tok = lex_identifier(lex);
    ASSERT_EQ(tok.type, TOKEN_WHILE, "keyword 'while' should be recognized");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_keyword_return(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("return", errors);

    Token tok = lex_identifier(lex);
    ASSERT_EQ(tok.type, TOKEN_RETURN, "keyword 'return' should be recognized");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_number_integer(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("42", errors);

    Token tok = lex_number(lex);
    ASSERT_EQ(tok.type, TOKEN_INT_LITERAL, "token should be integer literal");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_number_float(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("3.14", errors);

    Token tok = lex_number(lex);
    ASSERT_EQ(tok.type, TOKEN_FLOAT_LITERAL, "token should be float literal");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_string(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("\"hello\"", errors);

    Token tok = lex_string(lex);
    ASSERT_EQ(tok.type, TOKEN_STRING, "token should be string");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_skip_whitespace(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("   abc", errors);

    skip_whitespace(lex);
    ASSERT_EQ(lex->position, 3, "skip_whitespace should advance position past spaces");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_operator_type_declarator(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("::", errors);

    Token tok = lex_operator(lex);
    ASSERT_EQ(tok.type, TOKEN_TYPE_DECLARATOR, "token should be type declarator operator");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_peek_char_eof(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("x", errors);

    next_char(lex);
    char c = peek_char(lex);
    ASSERT_EQ(c, '\0', "peek_char at EOF should return null terminator");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lexer_empty_string(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("", errors);

    ASSERT_NOT_NULL(lex, "create_lexer should handle empty string");
    ASSERT_EQ(lex->length, 0, "lexer length should be 0");
    char c = peek_char(lex);
    ASSERT_EQ(c, '\0', "peek_char on empty string should return null terminator");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

int run_lexer_tests(TestSuite* suite)
{
    if (!suite)
    {
        return -1;
    }

    test_suite_run_test(suite, "lexer_create_lexer", test_create_lexer);
    test_suite_run_test(suite, "lexer_peek_char", test_peek_char);
    test_suite_run_test(suite, "lexer_next_char", test_next_char);
    test_suite_run_test(suite, "lexer_peek_next", test_peek_next);
    test_suite_run_test(suite, "lexer_next_char_newline", test_next_char_newline);
    test_suite_run_test(suite, "lexer_create_token", test_create_token);
    test_suite_run_test(suite, "lexer_lex_identifier", test_lex_identifier);
    test_suite_run_test(suite, "lexer_keyword_if", test_lex_keyword_if);
    test_suite_run_test(suite, "lexer_keyword_while", test_lex_keyword_while);
    test_suite_run_test(suite, "lexer_keyword_return", test_lex_keyword_return);
    test_suite_run_test(suite, "lexer_number_integer", test_lex_number_integer);
    test_suite_run_test(suite, "lexer_number_float", test_lex_number_float);
    test_suite_run_test(suite, "lexer_string", test_lex_string);
    test_suite_run_test(suite, "lexer_skip_whitespace", test_skip_whitespace);
    test_suite_run_test(suite, "lexer_operator_type_declarator", test_lex_operator_type_declarator);
    test_suite_run_test(suite, "lexer_peek_char_eof", test_peek_char_eof);
    test_suite_run_test(suite, "lexer_empty_string", test_lexer_empty_string);

    return 0;
}

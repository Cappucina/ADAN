#include "../lex/lexer.h"

#include <stdlib.h>
#include <string.h>

#include "diagnostic.h"
#include "test.h"
#include <stdio.h>

static const char* token_to_string(TokenType type)
{
    switch (type)
    {
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_ERROR: return "TOKEN_ERROR";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_INT: return "TOKEN_INT";
        case TOKEN_FLOAT: return "TOKEN_FLOAT";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_BOOL: return "TOKEN_BOOL";
        case TOKEN_CHAR: return "TOKEN_CHAR";
        case TOKEN_NULL: return "TOKEN_NULL";
        case TOKEN_VOID: return "TOKEN_VOID";
        case TOKEN_ADD: return "TOKEN_ADD";
        case TOKEN_SUBTRACT: return "TOKEN_SUBTRACT";
        case TOKEN_MULTIPLY: return "TOKEN_MULTIPLY";
        case TOKEN_DIVIDE: return "TOKEN_DIVIDE";
        case TOKEN_MODULO: return "TOKEN_MODULO";
        case TOKEN_CARET: return "TOKEN_CARET";
        case TOKEN_EXPONENT: return "TOKEN_EXPONENT";
        case TOKEN_ADDRESS_OF: return "TOKEN_ADDRESS_OF";
        case TOKEN_REFERENCE: return "TOKEN_REFERENCE";
        case TOKEN_INCREMENT: return "TOKEN_INCREMENT";
        case TOKEN_DECREMENT: return "TOKEN_DECREMENT";
        case TOKEN_BITWISE_AND: return "TOKEN_BITWISE_AND";
        case TOKEN_BITWISE_OR: return "TOKEN_BITWISE_OR";
        case TOKEN_BITWISE_NOT: return "TOKEN_BITWISE_NOT";
        case TOKEN_BITWISE_XOR: return "TOKEN_BITWISE_XOR";
        case TOKEN_BITWISE_ZERO_FILL_LEFT_SHIFT: return "TOKEN_BITWISE_ZERO_FILL_LEFT_SHIFT";
        case TOKEN_BITWISE_SIGNED_RIGHT_SHIFT: return "TOKEN_BITWISE_SIGNED_RIGHT_SHIFT";
        case TOKEN_BITWISE_ZERO_FILL_RIGHT_SHIFT: return "TOKEN_BITWISE_ZERO_FILL_RIGHT_SHIFT";
        case TOKEN_LEFT_PAREN: return "TOKEN_LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "TOKEN_RIGHT_PAREN";
        case TOKEN_LEFT_BRACE: return "TOKEN_LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "TOKEN_RIGHT_BRACE";
        case TOKEN_LEFT_BRACKET: return "TOKEN_LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET: return "TOKEN_RIGHT_BRACKET";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_PERIOD: return "TOKEN_PERIOD";
        case TOKEN_APOSTROPHE: return "TOKEN_APOSTROPHE";
        case TOKEN_QUOTATION: return "TOKEN_QUOTATION";
        case TOKEN_NOT: return "TOKEN_NOT";
        case TOKEN_AND: return "TOKEN_AND";
        case TOKEN_TYPE_DECLARATOR: return "TOKEN_TYPE_DECLARATOR";
        case TOKEN_EQUALS: return "TOKEN_EQUALS";
        case TOKEN_GREATER: return "TOKEN_GREATER";
        case TOKEN_LESS: return "TOKEN_LESS";
        case TOKEN_GREATER_EQUALS: return "TOKEN_GREATER_EQUALS";
        case TOKEN_LESS_EQUALS: return "TOKEN_LESS_EQUALS";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_NOT_EQUALS: return "TOKEN_NOT_EQUALS";
        case TOKEN_OR: return "TOKEN_OR";
        case TOKEN_ELLIPSIS: return "TOKEN_ELLIPSIS";
        case TOKEN_TRUE_LITERAL: return "TOKEN_TRUE_LITERAL";
        case TOKEN_INT_LITERAL: return "TOKEN_INT_LITERAL";
        case TOKEN_FALSE_LITERAL: return "TOKEN_FALSE_LITERAL";
        case TOKEN_FLOAT_LITERAL: return "TOKEN_FLOAT_LITERAL";
        case TOKEN_IF: return "TOKEN_IF";
        case TOKEN_WHILE: return "TOKEN_WHILE";
        case TOKEN_FOR: return "TOKEN_FOR";
        case TOKEN_INCLUDE: return "TOKEN_INCLUDE";
        case TOKEN_CONTINUE: return "TOKEN_CONTINUE";
        case TOKEN_PROGRAM: return "TOKEN_PROGRAM";
        case TOKEN_RETURN: return "TOKEN_RETURN";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_STRUCT: return "TOKEN_STRUCT";
        case TOKEN_BREAK: return "TOKEN_BREAK";
        default: return "TOKEN_UNKNOWN";
    }
}

static void print_token_stream(const char* src)
{
    ErrorList* errors = create_errors();
    Lexer* lx = create_lexer(src, errors, "(test)");
    if (!lx)
    {
        free_errors(errors);
        return;
    }

    printf("Token stream for source: %s\n", src);
    while (true)
    {
        Token t = lex(lx);
        const char* name = token_to_string(t.type);
        printf("%s: '%.*s' (start=%u len=%u line=%u col=%u)\n",
               name,
               (int)t.length,
               t.lexeme ? t.lexeme : "",
               t.start,
               t.length,
               t.line,
               t.column);
        if (t.type == TOKEN_EOF || t.type == TOKEN_ERROR)
            break;
    }

    free_lexer(lx);
    free_errors(errors);
}

static int test_create_lexer(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("int x = 5;", errors, "test");

    ASSERT_NOT_NULL(lex, "create_lexer should not return NULL");
    ASSERT_EQ(lex->position, 0, "Initial position should be 0");
    ASSERT_EQ(lex->line, 1, "Initial line should be 1");
    ASSERT_EQ(lex->column, 1, "Initial column should be 1");
    ASSERT_STREQ(lex->file, "test", "Source name should match");
    ASSERT_EQ(lex->length, strlen("int x = 5;"), "Length should match input string length");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_peek_char(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("hello", errors, "test");

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
    Lexer* lex = create_lexer("ab", errors, "test");

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
    Lexer* lex = create_lexer("hello", errors, "test");

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
    Lexer* lex = create_lexer("a\nb", errors, "test");

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
    Lexer* lex = create_lexer("int x;", errors, "test");

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
    Lexer* lex = create_lexer("myVariable", errors, "test");

    Token tok = lex_identifier(lex);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER, "token should be identifier");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_keyword_if(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("if", errors, "test");

    Token tok = lex_identifier(lex);
    ASSERT_EQ(tok.type, TOKEN_IF, "keyword 'if' should be recognized");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_keyword_while(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("while", errors, "test");

    Token tok = lex_identifier(lex);
    ASSERT_EQ(tok.type, TOKEN_WHILE, "keyword 'while' should be recognized");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_keyword_return(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("return", errors, "test");

    Token tok = lex_identifier(lex);
    ASSERT_EQ(tok.type, TOKEN_RETURN, "keyword 'return' should be recognized");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_number_integer(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("42", errors, "test");

    Token tok = lex_number(lex);
    ASSERT_EQ(tok.type, TOKEN_INT_LITERAL, "token should be integer literal");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_number_float(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("3.14", errors, "test");

    Token tok = lex_number(lex);
    ASSERT_EQ(tok.type, TOKEN_FLOAT_LITERAL, "token should be float literal");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_string(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("\"hello\"", errors, "test");

    Token tok = lex_string(lex);
    ASSERT_EQ(tok.type, TOKEN_STRING, "token should be string");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_skip_whitespace(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("   abc", errors, "test");

    skip_whitespace(lex);
    ASSERT_EQ(lex->position, 3, "skip_whitespace should advance position past spaces");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_operator_type_declarator(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("::", errors, "test");

    Token tok = lex_operator(lex);
    ASSERT_EQ(tok.type, TOKEN_TYPE_DECLARATOR, "token should be type declarator operator");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_peek_char_eof(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("x", errors, "test");

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
    Lexer* lex = create_lexer("", errors, "test");

    ASSERT_NOT_NULL(lex, "create_lexer should handle empty string");
    ASSERT_EQ(lex->length, 0, "lexer length should be 0");
    char c = peek_char(lex);
    ASSERT_EQ(c, '\0', "peek_char on empty string should return null terminator");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_unclosed_string_error(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("\"unclosed string", errors, "test");

    Token tok = lex_string(lex);
    ASSERT_EQ(tok.type, TOKEN_STRING, "unclosed string should still create token");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_unclosed_char_error(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("'x", errors, "test");

    Token tok = lex_char(lex);
    ASSERT_EQ(tok.type, TOKEN_CHAR, "unclosed char should still create token");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_invalid_operator_error(void)
{
    ErrorList* errors = create_errors();
    Lexer* lex = create_lexer("@", errors, "test");

    Token tok = lex_operator(lex);
    ASSERT_EQ(tok.type, TOKEN_ERROR, "invalid operator should create error token");

    free_lexer(lex);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_identifier(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("myVar", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER, "lex should recognize identifier");
    ASSERT_EQ(tok.length, 5, "identifier length should be 5");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_number(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("42", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_INT_LITERAL, "lex should recognize integer");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_string(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("\"hello\"", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_STRING, "lex should recognize string");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_paren(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("(", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_LEFT_PAREN, "lex should recognize left paren");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_brace(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("{", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_LEFT_BRACE, "lex should recognize left brace");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_bracket(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("[", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_LEFT_BRACKET, "lex should recognize left bracket");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_semicolon(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer(";", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_SEMICOLON, "lex should recognize semicolon");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_comma(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer(",", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_COMMA, "lex should recognize comma");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_single_token_operator(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("+", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_ADD, "lex should recognize addition operator");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_whitespace_skip(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("  \t  42", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_INT_LITERAL, "lex should skip whitespace");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_eof(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_EOF, "lex should return EOF for empty input");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_unexpected_char_error(void)
{
    ErrorList* errors = create_errors();
    Lexer* lexer = create_lexer("$", errors, "test");

    Token tok = lex(lexer);
    ASSERT_EQ(tok.type, TOKEN_ERROR, "lex should create error token for invalid character");

    free_lexer(lexer);
    free_errors(errors);
    return 0;
}

static int test_lex_example_program(void)
{
    FILE* f = fopen("examples/adan.adn", "rb");
    if (!f)
    {
        printf("Could not open examples/adan.adn\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf)
    {
        fclose(f);
        printf("Out of memory reading example file\n");
        return 1;
    }

    size_t read = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[read] = '\0';

    print_token_stream(buf);

    ErrorList* errors = create_errors();
    Lexer* lx = create_lexer(buf, errors, "examples/adan.adn");
    if (!lx)
    {
        free(buf);
        free_errors(errors);
        return 1;
    }

    while (true)
    {
        Token t = lex(lx);
        if (t.type == TOKEN_EOF || t.type == TOKEN_ERROR)
            break;
    }

    ASSERT_ERROR_COUNT(errors, 0, "Example program should produce no diagnostics\n");

    free_lexer(lx);
    free_errors(errors);
    free(buf);
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
    test_suite_run_test(suite, "lexer_unclosed_string_error", test_lex_unclosed_string_error);
    test_suite_run_test(suite, "lexer_unclosed_char_error", test_lex_unclosed_char_error);
    test_suite_run_test(suite, "lexer_invalid_operator_error", test_lex_invalid_operator_error);
    test_suite_run_test(suite, "lexer_single_token_identifier", test_lex_single_token_identifier);
    test_suite_run_test(suite, "lexer_single_token_number", test_lex_single_token_number);
    test_suite_run_test(suite, "lexer_single_token_string", test_lex_single_token_string);
    test_suite_run_test(suite, "lexer_single_token_paren", test_lex_single_token_paren);
    test_suite_run_test(suite, "lexer_single_token_brace", test_lex_single_token_brace);
    test_suite_run_test(suite, "lexer_single_token_bracket", test_lex_single_token_bracket);
    test_suite_run_test(suite, "lexer_single_token_semicolon", test_lex_single_token_semicolon);
    test_suite_run_test(suite, "lexer_single_token_comma", test_lex_single_token_comma);
    test_suite_run_test(suite, "lexer_single_token_operator", test_lex_single_token_operator);
    test_suite_run_test(suite, "lexer_whitespace_skip", test_lex_whitespace_skip);
    test_suite_run_test(suite, "lexer_eof", test_lex_eof);
    test_suite_run_test(suite, "lexer_unexpected_char_error", test_lex_unexpected_char_error);
    test_suite_run_test(suite, "lexer_example_program", test_lex_example_program);



    //print_token_stream("int main() { int x = 42; if (x > 0) x = x - 1; return x; }");

    return 0;
}

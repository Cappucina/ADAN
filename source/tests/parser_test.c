
#include "../parse/parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/diagnostics.h"
#include "test.h"

static ASTNode* parse_source(const char* src, ErrorList** out_errors)
{
    ErrorList* errors = create_errors();
    ASTNode* root = parse(src, errors, "<test>");
    if (out_errors)
        *out_errors = errors;
    else
        free_errors(errors);
    return root;
}

static int has_parse_error(ErrorList* errors)
{
    if (!errors) return 0;
    for (size_t i = 0; i < errors->size; ++i)
    {
        if (errors->errors[i].severity == ERROR || errors->errors[i].severity == CRITICAL)
            return 1;
    }
    return 0;
}

static int test_parse_empty_program(void)
{
    ErrorList* errors = NULL;
    ASTNode* root = parse_source("", &errors);
    ASSERT_NOT_NULL(root, "Parser should return non-NULL AST for empty program");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for empty program");
    free_errors(errors);
    return 0;
}

static int test_parse_single_function(void)
{
    const char* src = "program::int main() { return 0; }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for single function");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for single function");
    free_errors(errors);
    return 0;
}

static int test_parse_var_decl(void)
{
    const char* src = "x::int = 42;";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for variable declaration");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for variable declaration");
    free_errors(errors);
    return 0;
}

static int test_parse_if_else(void)
{
    const char* src = "program::int main() { if (1) { return 1; } else { return 0; } }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for if-else statement");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for if-else statement");
    free_errors(errors);
    return 0;
}

static int test_parse_while(void)
{
    const char* src = "program::int main() { while (x < 10) { x += 1; } }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for while loop");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for while loop");
    free_errors(errors);
    return 0;
}

static int test_parse_for(void)
{
    const char* src = "program::int main() { for (i::int = 0; i < 10; i++) { } }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for for loop");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for for loop");
    free_errors(errors);
    return 0;
}

static int test_parse_struct(void)
{
    const char* src = "struct Point { x::int; y::int; };";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for struct definition");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for struct definition");
    free_errors(errors);
    return 0;
}

static int test_parse_include(void)
{
    const char* src = "include org.lib;";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for include statement");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for include statement");
    free_errors(errors);
    return 0;
}

static int test_parse_nested_blocks(void)
{
    const char* src = "program::int main() { { { x::int; } } }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for nested blocks");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for nested blocks");
    free_errors(errors);
    return 0;
}

static int test_parse_assignment_binary(void)
{
    const char* src = "program::int main() { x::int = 1 + 2 * 3; }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for assignment and binary ops");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for assignment and binary ops");
    free_errors(errors);
    return 0;
}

static int test_parse_unary_postfix(void)
{
    const char* src = "program::int main() { x++; --y; }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for unary and postfix ops");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for unary and postfix ops");
    free_errors(errors);
    return 0;
}

static int test_parse_func_params(void)
{
    const char* src = "program::int add(a::int, b::int) { return a + b; }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for function with parameters");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for function with parameters");
    free_errors(errors);
    return 0;
}

static int test_parse_literals(void)
{
    const char* src = "c::char = 'a'; s::string = \"hello\";";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for char and string literals");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for char and string literals");
    free_errors(errors);
    return 0;
}

static int test_parse_bool_null(void)
{
    const char* src = "b::bool = true; b = false; b = NULL;";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for boolean and null literals");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for boolean and null literals");
    free_errors(errors);
    return 0;
}

static int test_parse_pointer_type(void)
{
    const char* src = "p::int*;";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for pointer type");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for pointer type");
    free_errors(errors);
    return 0;
}

static int test_parse_array_type(void)
{
    const char* src = "arr::int[10];";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should return AST for array type");
    ASSERT_FALSE(has_parse_error(errors), "Parser should not report errors for array type");
    free_errors(errors);
    return 0;
}

static int test_parse_invalid_syntax(void)
{
    const char* src = "program::int main( { return 0; }";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_TRUE(root == NULL || has_parse_error(errors), "Parser should fail or report error for invalid syntax");
    free_errors(errors);
    return 0;
}

static int test_parse_missing_semicolon(void)
{
    const char* src = "x::int = 5";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_TRUE(root == NULL || has_parse_error(errors), "Parser should fail or report error for missing semicolon");
    free_errors(errors);
    return 0;
}

static int test_parse_unterminated_string(void)
{
    const char* src = "s::string = \"unterminated;";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_TRUE(root == NULL || has_parse_error(errors), "Parser should fail or report error for unterminated string");
    free_errors(errors);
    return 0;
}

static int test_parse_unknown_type(void)
{
    const char* src = "foo x;";
    ErrorList* errors = NULL;
    ASTNode* root = parse_source(src, &errors);
    ASSERT_NOT_NULL(root, "Parser should accept user-defined types");
    free_errors(errors);
    return 0;
}

int run_parser_tests(TestSuite* suite)
{
    int fails = 0;

    fails += test_suite_run_test(suite, "parser_empty_program", test_parse_empty_program);
    fails += test_suite_run_test(suite, "parser_single_function", test_parse_single_function);
    fails += test_suite_run_test(suite, "parser_var_decl", test_parse_var_decl);
    fails += test_suite_run_test(suite, "parser_if_else", test_parse_if_else);
    fails += test_suite_run_test(suite, "parser_while", test_parse_while);
    fails += test_suite_run_test(suite, "parser_for", test_parse_for);
    fails += test_suite_run_test(suite, "parser_struct", test_parse_struct);
    fails += test_suite_run_test(suite, "parser_include", test_parse_include);
    fails += test_suite_run_test(suite, "parser_nested_blocks", test_parse_nested_blocks);
    fails += test_suite_run_test(suite, "parser_assignment_binary", test_parse_assignment_binary);
    fails += test_suite_run_test(suite, "parser_unary_postfix", test_parse_unary_postfix);
    fails += test_suite_run_test(suite, "parser_func_params", test_parse_func_params);
    fails += test_suite_run_test(suite, "parser_literals", test_parse_literals);
    fails += test_suite_run_test(suite, "parser_bool_null", test_parse_bool_null);
    fails += test_suite_run_test(suite, "parser_pointer_type", test_parse_pointer_type);
    fails += test_suite_run_test(suite, "parser_array_type", test_parse_array_type);
    fails += test_suite_run_test(suite, "parser_invalid_syntax", test_parse_invalid_syntax);
    fails += test_suite_run_test(suite, "parser_missing_semicolon", test_parse_missing_semicolon);
    fails += test_suite_run_test(suite, "parser_unterminated_string", test_parse_unterminated_string);
    fails += test_suite_run_test(suite, "parser_unknown_type", test_parse_unknown_type);

    return fails;
}

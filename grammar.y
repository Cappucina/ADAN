
%{
#include "./include/ast.h"
#include "./source/lex/lexer.h"
#include <stdio.h>
#include <stdlib.h>
ASTNode *root;
int yylex(void);
void yyerror(const char* s);
%}

%union {
    int ival;
    float fval;
    char *sval;
    ASTNode *node;
}

%token <sval> IDENTIFIER
%token <ival> INT_LITERAL
%token <fval> FLOAT_LITERAL
%token <sval> STRING_LITERAL CHAR_LITERAL
%token TRUE FALSE NULL_TOKEN
%token PROGRAM INCLUDE STRUCT FOR IF WHILE RETURN BREAK CONTINUE ELSE
%token INT FLOAT BOOL STRING CHAR VOID
%token OPEN_PAREN CLOSE_PAREN OPEN_BRACE CLOSE_BRACE SEMICOLON COMMA PERIOD OPEN_BRACKET CLOSE_BRACKET
%token EQUAL EQUALITY NOT NOT_EQUALS TYPE_DECL MUL DIV SUB ADD MOD EXPONENT QUOTE APOSTROPHE ELLIPSIS AND OR MUL_EQUALS DIV_EQUALS SUB_EQUALS ADD_EQUALS MOD_EQUALS GREATER LESS GREATER_EQUALS LESS_EQUALS ADD_ADD SUB_SUB BITWISE_AND BITWISE_OR BITWISE_NOT BITWISE_XOR BITWISE_NAND BITWISE_NOR BITWISE_XNOR BITWISE_ZERO_FILL_LEFT_SHIFT BITWISE_SIGNED_RIGHT_SHIFT BITWISE_ZERO_FILL_RIGHT_SHIFT

%type <node> program allowed_top_level includes include include_tail top_level_keywords_seq top_level_keywords function_def param_list_opt param_list param variable_def variable_init_opt struct_def struct struct_members struct_member type default_type user_type pointer_type pointer_stars statement code_block if_stmt else_opt while_stmt for_stmt for_init for_cond_opt for_iter_opt return_stmt break_stmt continue_stmt expression_statement expression assignment assignment_op logical_or logical_and equality comparison additive multiplicative unary postfix primary stmts stmt expr term factor block expression_opt

%%

program
    : allowed_top_level                { root = $1; }
    ;

allowed_top_level
    : includes top_level_keywords_seq   { $$ = ast_allowed_top_level($1, $2); }
    ;

includes
    : includes include                 { $$ = ast_append_include($1, $2); }
    | /* empty */                      { $$ = ast_empty(); }
    ;

include
    : INCLUDE IDENTIFIER include_tail SEMICOLON { $$ = ast_include($2, $3); }
    ;

include_tail
    : PERIOD IDENTIFIER include_tail   { $$ = ast_include_tail($2, $3); }
    | /* empty */                      { $$ = ast_empty(); }
    ;

top_level_keywords_seq
    : top_level_keywords_seq top_level_keywords { $$ = ast_append_top_level($1, $2); }
    | /* empty */                      { $$ = ast_empty(); }
    ;

top_level_keywords
    : function_def                     { $$ = $1; }
    | struct_def                       { $$ = $1; }
    | variable_def                     { $$ = $1; }
    ;

function_def
    : PROGRAM TYPE_DECL type IDENTIFIER OPEN_PAREN param_list_opt CLOSE_PAREN code_block { $$ = ast_function_def($3, $4, $6, $8); }
    ;

param_list_opt
    : param_list                       { $$ = $1; }
    | /* empty */                      { $$ = ast_empty(); }
    ;

param_list
    : param_list COMMA param           { $$ = ast_append_param($1, $3); }
    | param                            { $$ = ast_single_param($1); }
    ;

param
    : IDENTIFIER TYPE_DECL type        { $$ = ast_param($1, $3); }
    ;

variable_def
    : IDENTIFIER TYPE_DECL type variable_init_opt SEMICOLON { $$ = ast_variable_def($1, $3, $4); }
    ;

variable_init_opt
    : EQUAL expression                 { $$ = $2; }
    | /* empty */                      { $$ = ast_empty(); }
    ;

struct_def
    : struct                           { $$ = $1; }
    ;

struct
    : STRUCT IDENTIFIER OPEN_BRACE struct_members CLOSE_BRACE SEMICOLON { $$ = ast_struct($2, $4); }
    ;

struct_members
    : struct_members struct_member      { $$ = ast_append_struct_member($1, $2); }
    | /* empty */                      { $$ = ast_empty(); }
    ;

struct_member
    : variable_def                     { $$ = $1; }
    | function_def                     { $$ = $1; }
    ;

type
    : default_type                     { $$ = $1; }
    | user_type                        { $$ = $1; }
    | pointer_type                     { $$ = $1; }
    | VOID                             { $$ = ast_type_void(); }
    ;

default_type
    : INT                              { $$ = ast_type_int(); }
    | FLOAT                            { $$ = ast_type_float(); }
    | STRING                           { $$ = ast_type_string(); }
    | BOOL                             { $$ = ast_type_bool(); }
    | CHAR                             { $$ = ast_type_char(); }
    ;

user_type
    : IDENTIFIER                       { $$ = ast_type_user($1); }
    ;

pointer_type
    : type pointer_stars               { $$ = ast_type_pointer($1, $2); }
    ;

pointer_stars
    : pointer_stars MUL                { $$ = ast_append_pointer_star($1); }
    | /* empty */                      { $$ = ast_empty(); }
    ;

stmts
    : stmts statement                  { $$ = ast_append_stmt($1, $2); }
    | /* empty */                      { $$ = ast_empty(); }
    ;

statement
    : if_stmt                          { $$ = $1; }
    | while_stmt                        { $$ = $1; }
    | for_stmt                          { $$ = $1; }
    | return_stmt                       { $$ = $1; }
    | break_stmt                        { $$ = $1; }
    | continue_stmt                     { $$ = $1; }
    | expression_statement              { $$ = $1; }
    | code_block                        { $$ = $1; }
    | variable_def                      { $$ = $1; }
    ;

code_block
    : OPEN_BRACE stmts CLOSE_BRACE      { $$ = ast_code_block($2); }
    ;

if_stmt
    : IF OPEN_PAREN expression CLOSE_PAREN code_block else_opt { $$ = ast_if_stmt($3, $5, $6); }
    ;

else_opt
    : ELSE if_stmt                      { $$ = $2; }
    | ELSE code_block                   { $$ = $2; }
    | /* empty */                       { $$ = ast_empty(); }
    ;

while_stmt
    : WHILE OPEN_PAREN expression CLOSE_PAREN code_block { $$ = ast_while_stmt($3, $5); }
    ;

for_stmt
    : FOR OPEN_PAREN for_init SEMICOLON for_cond_opt SEMICOLON for_iter_opt CLOSE_PAREN code_block { $$ = ast_for_stmt($3, $5, $7, $9); }
    ;

for_init
    : variable_def                      { $$ = $1; }
    | expression_statement              { $$ = $1; }
    ;

for_cond_opt
    : expression                        { $$ = $1; }
    | /* empty */                       { $$ = ast_empty(); }
    ;

for_iter_opt
    : expression                        { $$ = $1; }
    | /* empty */                       { $$ = ast_empty(); }
    ;

return_stmt
    : RETURN expression_opt SEMICOLON   { $$ = ast_return_stmt($2); }
    ;

expression_opt
    : expression                        { $$ = $1; }
    | /* empty */                       { $$ = ast_empty(); }
    ;

break_stmt
    : BREAK SEMICOLON                   { $$ = ast_break_stmt(); }
    ;

continue_stmt
    : CONTINUE SEMICOLON                { $$ = ast_continue_stmt(); }
    ;

expression_statement
    : expression SEMICOLON              { $$ = ast_expr_stmt($1); }
    ;

expression
    : assignment                        { $$ = $1; }
    ;

assignment
    : logical_or assignment_op assignment { $$ = ast_assignment($1, $2, $3); }
    | logical_or                        { $$ = $1; }
    ;

assignment_op
    : EQUAL                             { $$ = ast_assign_op("="); }
    | ADD_EQUALS                        { $$ = ast_assign_op("+="); }
    | SUB_EQUALS                        { $$ = ast_assign_op("-="); }
    | MUL_EQUALS                        { $$ = ast_assign_op("*="); }
    | DIV_EQUALS                        { $$ = ast_assign_op("/="); }
    | MOD_EQUALS                        { $$ = ast_assign_op("%="); }
    | BITWISE_AND                       { $$ = ast_assign_op("&="); }
    | BITWISE_OR                        { $$ = ast_assign_op("|="); }
    | BITWISE_XOR                       { $$ = ast_assign_op("^="); }
    ;

logical_or
    : logical_or OR logical_and         { $$ = ast_binary_op("||", $1, $3); }
    | logical_and                       { $$ = $1; }
    ;

logical_and
    : logical_and AND equality          { $$ = ast_binary_op("&&", $1, $3); }
    | equality                          { $$ = $1; }
    ;

equality
    : equality EQUALITY comparison      { $$ = ast_binary_op("==", $1, $3); }
    | equality NOT_EQUALS comparison    { $$ = ast_binary_op("!=", $1, $3); }
    | comparison                        { $$ = $1; }
    ;

comparison
    : comparison LESS additive          { $$ = ast_binary_op("<", $1, $3); }
    | comparison GREATER additive       { $$ = ast_binary_op(">", $1, $3); }
    | comparison LESS_EQUALS additive   { $$ = ast_binary_op("<=", $1, $3); }
    | comparison GREATER_EQUALS additive{ $$ = ast_binary_op(">=", $1, $3); }
    | additive                          { $$ = $1; }
    ;

additive
    : additive ADD multiplicative       { $$ = ast_binary_op("+", $1, $3); }
    | additive SUB multiplicative       { $$ = ast_binary_op("-", $1, $3); }
    | multiplicative                    { $$ = $1; }
    ;

multiplicative
    : multiplicative MUL unary          { $$ = ast_binary_op("*", $1, $3); }
    | multiplicative DIV unary          { $$ = ast_binary_op("/", $1, $3); }
    | multiplicative MOD unary          { $$ = ast_binary_op("%", $1, $3); }
    | unary                             { $$ = $1; }
    ;

unary
    : NOT unary                         { $$ = ast_unary_op("!", $2); }
    | SUB unary                         { $$ = ast_unary_op("-", $2); }
    | BITWISE_AND unary                 { $$ = ast_unary_op("&", $2); }
    | MUL unary                         { $$ = ast_unary_op("*", $2); }
    | postfix                           { $$ = $1; }
    ;

postfix
    : primary ADD_ADD                   { $$ = ast_postfix_op("++", $1); }
    | primary SUB_SUB                   { $$ = ast_postfix_op("--", $1); }
    | primary                           { $$ = $1; }
    ;

primary
    : IDENTIFIER                        { $$ = ast_identifier($1); }
    | INT_LITERAL                       { $$ = ast_int_literal($1); }
    | FLOAT_LITERAL                     { $$ = ast_float_literal($1); }
    | STRING_LITERAL                    { $$ = ast_string_literal($1); }
    | CHAR_LITERAL                      { $$ = ast_char_literal($1[0]); }
    | TRUE                              { $$ = ast_true(); }
    | FALSE                             { $$ = ast_false(); }
    | NULL_TOKEN                        { $$ = ast_null(); }
    | OPEN_PAREN expression CLOSE_PAREN { $$ = $2; }
    ;


stmts
    : stmts statement                  { $$ = ast_append_stmt($1, $2); }
    | /* empty */                      { $$ = ast_empty(); }
    ;

statement
    : if_stmt                          { $$ = $1; }
    | while_stmt                       { $$ = $1; }
    | for_stmt                         { $$ = $1; }
    | return_stmt                      { $$ = $1; }
    | break_stmt                       { $$ = $1; }
    | continue_stmt                    { $$ = $1; }
    | expression_statement             { $$ = $1; }
    | code_block                       { $$ = $1; }
    | variable_def                     { $$ = $1; }
    ;

code_block
    : OPEN_BRACE stmts CLOSE_BRACE      { $$ = ast_code_block($2); }
    ;

if_stmt
    : IF OPEN_PAREN expression CLOSE_PAREN code_block else_opt { $$ = ast_if_stmt($3, $5, $6); }
    ;

else_opt
    : ELSE if_stmt                      { $$ = $2; }
    | ELSE code_block                   { $$ = $2; }
    | /* empty */                       { $$ = ast_empty(); }
    ;

while_stmt
    : WHILE OPEN_PAREN expression CLOSE_PAREN code_block { $$ = ast_while_stmt($3, $5); }
    ;

for_stmt
    : FOR OPEN_PAREN for_init SEMICOLON for_cond_opt SEMICOLON for_iter_opt CLOSE_PAREN code_block { $$ = ast_for_stmt($3, $5, $7, $9); }
    ;

for_init
    : variable_def                      { $$ = $1; }
    | expression_statement              { $$ = $1; }
    ;

for_cond_opt
    : expression                        { $$ = $1; }
    | /* empty */                       { $$ = ast_empty(); }
    ;

for_iter_opt
    : expression                        { $$ = $1; }
    | /* empty */                       { $$ = ast_empty(); }
    ;

return_stmt
    : RETURN expression_opt SEMICOLON   { $$ = ast_return_stmt($2); }
    ;

expression_opt
    : expression                        { $$ = $1; }
    | /* empty */                       { $$ = ast_empty(); }
    ;

break_stmt
    : BREAK SEMICOLON                   { $$ = ast_break_stmt(); }
    ;

continue_stmt
    : CONTINUE SEMICOLON                { $$ = ast_continue_stmt(); }
    ;

expression_statement
    : expression SEMICOLON              { $$ = ast_expr_stmt($1); }
    ;

expression
    : assignment                        { $$ = $1; }
    ;

assignment
    : logical_or assignment_op assignment { $$ = ast_assignment($1, $2, $3); }
    | logical_or                        { $$ = $1; }
    ;

logical_or
    : logical_or OR logical_and         { $$ = ast_binary_op("||", $1, $3); }
    | logical_and                       { $$ = $1; }
    ;

logical_and
    : logical_and AND equality          { $$ = ast_binary_op("&&", $1, $3); }
    | equality                          { $$ = $1; }
    ;

equality
    : equality EQUALITY comparison      { $$ = ast_binary_op("==", $1, $3); }
    | equality NOT_EQUALS comparison    { $$ = ast_binary_op("!=", $1, $3); }
    | comparison                        { $$ = $1; }
    ;

comparison
    : comparison LESS additive          { $$ = ast_binary_op("<", $1, $3); }
    | comparison GREATER additive       { $$ = ast_binary_op(">", $1, $3); }
    | comparison LESS_EQUALS additive   { $$ = ast_binary_op("<=", $1, $3); }
    | comparison GREATER_EQUALS additive{ $$ = ast_binary_op(">=", $1, $3); }
    | additive                          { $$ = $1; }
    ;

additive
    : additive ADD multiplicative       { $$ = ast_binary_op("+", $1, $3); }
    | additive SUB multiplicative       { $$ = ast_binary_op("-", $1, $3); }
    | multiplicative                    { $$ = $1; }
    ;

multiplicative
    : multiplicative MUL unary          { $$ = ast_binary_op("*", $1, $3); }
    | multiplicative DIV unary          { $$ = ast_binary_op("/", $1, $3); }
    | multiplicative MOD unary          { $$ = ast_binary_op("%", $1, $3); }
    | unary                             { $$ = $1; }
    ;

unary
    : NOT unary                         { $$ = ast_unary_op("!", $2); }
    | SUB unary                         { $$ = ast_unary_op("-", $2); }
    | BITWISE_AND unary                 { $$ = ast_unary_op("&", $2); }
    | MUL unary                         { $$ = ast_unary_op("*", $2); }
    | postfix                           { $$ = $1; }
    ;

postfix
    : primary ADD_ADD                   { $$ = ast_postfix_op("++", $1); }
    | primary SUB_SUB                   { $$ = ast_postfix_op("--", $1); }
    | primary                           { $$ = $1; }
    ;

primary
    : IDENTIFIER                        { $$ = ast_identifier($1); }
    | INT_LITERAL                       { $$ = ast_int_literal($1); }
    | FLOAT_LITERAL                     { $$ = ast_float_literal($1); }
    | STRING_LITERAL                    { $$ = ast_string_literal($1); }
    | CHAR_LITERAL                      { $$ = ast_char_literal($1[0]); }
    | TRUE                              { $$ = ast_true(); }
    | FALSE                             { $$ = ast_false(); }
    | NULL_TOKEN                        { $$ = ast_null(); }
    | OPEN_PAREN expression CLOSE_PAREN { $$ = $2; }
    ;

%%


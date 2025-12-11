#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "lexer.h"
#include "ast.h"

typedef struct {
	Lexer* lexer;

	Token current_token;   // Store tokens by value, not pointer
	Token peek_token;

	bool error;
	char* error_message;
} Parser;

//
//  Sets up a new parser by attaching the provided lexer, pulling the
//   first two tokens to initialize the parser state.
//
void init_parser(Parser* parser, Lexer* lexer);

//
//  Releases memory owned by the parser, primarily for the error
//   message buffer.
//
void free_parser(Parser* parser);

// 
//  Fetch the respected error message from the logs file.
// 
const char* get_error_message(int error_code);

//
//  Checks whether the current token is a specific type and advances if
//   true; used for optional grammar elements.
//
bool match(Parser* parser, TokenType type);

//
//  Enforces that the current token must be a specific type, sets an
//   error and returns false if not, otherwise advances.
//
bool expect(Parser* parser, TokenType type, int error_msg, ...);

//
//  Checks whether the next token matches a type without actually
//   advancing the parser.
//
bool peek_is(Parser* parser, TokenType type);

//
//  Stores a formatted error message and marks the parser as failed.
//
void set_error(Parser* parser, int message, ...);

static inline bool has_error(Parser* parser) {
	return parser->error;
}

//
//  Allocates and initializes an AST node of a given type using token
//   metadata.
//
ASTNode* create_ast_node(ASTNodeType type, Token token);

//
//  Appends a child node to a parent AST node.
//
static inline void add_child(ASTNode* parent, ASTNode* child) {
	if (!parent || !child) return;
	ASTNode** new_children = (ASTNode**)realloc(parent->children, sizeof(ASTNode*)*  (parent->child_count + 1));

	if (!new_children) return;
	parent->children = new_children;
	parent->children[parent->child_count] = child;
	parent->child_count++;
}

//
//  Determines which kind of statement needs to be parsed
//   based on the current token.
//
ASTNode* parse_statement(Parser* parser);

ASTNode* parse_break_statement(Parser* parser);

ASTNode* parse_return_statement(Parser* parser);

//
//  Parses a function/program definition.
//
ASTNode* parse_program(Parser* parser);

//
//  Parse entire file with includes and program definition
//
ASTNode* parse_file(Parser* parser);

// 
//  Handles program parameters when either declaring a new
//   program or calling an existing one.
// 
ASTNode* parse_params(Parser* parser);

// 
//  Handles variable declaration, with an identifier followed by
//   a type and an assignment on the right-hand side.
// 
ASTNode* parse_declaration(Parser* parser);

//
//  Parses an identifier followed by "=" and an expression
//   on its right-hand side, ending with a semicolon.
//
ASTNode* parse_assignment(Parser* parser);

//
//  Entry point for expression parsing, usually invoking
//   binary/unary parsing.
//
ASTNode* parse_expression(Parser* parser);

// 
//  Fetch the precedence value of a binary operator. (E.g.: "4" for ^)
// 
int get_node_precedence(ASTNode* node);

//
//  Handles binary operations based on precedence and associativity,
//   examples: *, +, -, /, %, etc.
//
ASTNode* parse_binary(Parser* parser);

//
//  Handles prefix operators such as negation or logical NOT.
//
ASTNode* parse_unary(Parser* parser);

//
//  Handles identifiers, literals, parenthesized expressions.
//
ASTNode* parse_primary(Parser* parser);

//
//  Handles "if (condition) { ... }" statements.
//
ASTNode* parse_if_statement(Parser* parser);

//
//  Parses "while (condition) { ... }" loops.
//
ASTNode* parse_while_statement(Parser* parser);

//
//  Parses "for (init; condition; increment) { ... }" loops.
//
ASTNode* parse_for_statement(Parser* parser);

//
//  Parses function calls like "foo(x, y, z);".
//
ASTNode* parse_function_call(Parser* parser);

//
//  Parses include/import directives for bringing in external
//   libraries to be used in the program.
//
ASTNode* parse_include_statement(Parser* parser);

//
//  Parses a "{ ... }" block containing multiple statements.
//
ASTNode* parse_block(Parser* parser);

//
//  Validates and returns an AST node for an identifier token.
//
ASTNode* parse_identifier(Parser* parser);

//
//  Processes integer, float, string, boolean, etc. tokens
//   into literal AST nodes.
//
ASTNode* parse_literal(Parser* parser);

#endif

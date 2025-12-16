#ifndef AST_H
#define AST_H

#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum {
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_BOOLEAN,
	TYPE_CHAR,
	TYPE_NULL,
	TYPE_VOID,
	TYPE_ARRAY,
	TYPE_UNKNOWN
} Type;

typedef enum { NODE_ACTUAL, NODE_EXPECTED } NodeKind;

typedef enum {
	// 
	//  Keywords
	// 
	AST_FILE,
	AST_PROGRAM,
	AST_IF,
	AST_WHILE,
	AST_INCLUDE,
	AST_BREAK,
	AST_CONTINUE,
	AST_RETURN,
	AST_FOR,
	AST_ELSE,
	
	// 
	//  Uncategorized
	// 
	AST_DECLARATION,
	AST_ASSIGNMENT,
	AST_STATEMENT,
	AST_IDENTIFIER,
	AST_FUNCTION_CALL,
	AST_BLOCK,
	AST_COMPARISON,
	AST_ARRAY_ACCESS,
	AST_MEMBER_ACCESS,
	AST_TYPE,
	AST_PARAMS,
	AST_VARIADIC_PARAM,
	
	// 
	//  Expressions
	// 
	AST_EXPRESSION,
	AST_BINARY_EXPR,
	AST_UNARY_EXPR,
	AST_INCREMENT_EXPR,
	AST_GROUPED_EXPR,
	AST_TERNARY_EXPR,
	AST_CAST_EXPR,
	AST_ADDRESS_OF,
	AST_DEREFERENCE,
	AST_ARRAY_INDEX,
	
	// 
	//  Types of Operators
	// 
	AST_BINARY_OP,
	AST_UNARY_OP,
	AST_LOGICAL_OP,
	AST_OPERATORS,
	
	// 
	//  Literals
	// 
	AST_LITERAL,
	AST_ARRAY_LITERAL,
	AST_SINGLE_COMMENT
} ASTNodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
	ASTNodeType type;
	Token token;
	ASTNode** children;
	int child_count;
	Type annotated_type;
};

typedef struct ExpectedNode {
	ASTNodeType type;
	const char* token_text;
	TokenType token_type;
	int child_count;
	struct ExpectedNode** children;
} ExpectedNode;

static inline const char* node_type_to_string(ASTNodeType type) {
	switch (type) {
		case AST_FILE: return "AST_FILE";
		case AST_PROGRAM: return "AST_PROGRAM";
		case AST_ASSIGNMENT: return "AST_ASSIGNMENT";
		case AST_IDENTIFIER: return "AST_IDENTIFIER";
		case AST_TYPE: return "AST_TYPE";
		case AST_LITERAL: return "AST_LITERAL";
		case AST_IF: return "AST_IF";
		case AST_WHILE: return "AST_WHILE";
		case AST_FOR: return "AST_FOR";
		case AST_BLOCK: return "AST_BLOCK";
		case AST_COMPARISON: return "AST_COMPARISON";
		case AST_INCREMENT_EXPR: return "AST_INCREMENT_EXPR";
		case AST_OPERATORS: return "AST_OPERATORS";
		case AST_PARAMS: return "AST_PARAMS";
		case AST_FUNCTION_CALL: return "AST_FUNCTION_CALL";
		case AST_BINARY_OP: return "AST_BINARY_OP";
		case AST_BINARY_EXPR: return "AST_BINARY_EXPR";
		case AST_UNARY_OP: return "AST_UNARY_OP";
		case AST_UNARY_EXPR: return "AST_UNARY_EXPR";
		case AST_ARRAY_LITERAL: return "AST_ARRAY_LITERAL";
		case AST_ADDRESS_OF: return "AST_ADDRESS_OF";
		case AST_DEREFERENCE: return "AST_DEREFERENCE";
		case AST_ARRAY_INDEX: return "AST_ARRAY_INDEX";
		case AST_CONTINUE: return "AST_CONTINUE";
		case AST_VARIADIC_PARAM: return "AST_VARIADIC_PARAM";
		default: return "AST_UNKNOWN";
	}
}


static inline void free_ast(ASTNode* node) {
	if (!node) return;
	for (int i = 0; i < node->child_count; i++) {
		free_ast(node->children[i]);
	}
	
	if (node->children) free(node->children);
	if (node->token.text) free(node->token.text);

	free(node);
}

static inline void print_ast(void* node, NodeKind kind, int indent) {
	if (!node) return;

	ASTNode* actual = NULL;
	ExpectedNode* expected = NULL;
	ASTNodeType type;
	const char* text = NULL;
	int child_count;
	void** children;

	if (kind == NODE_ACTUAL) {
		actual = (ASTNode*)node;
		type = actual->type;
		text = actual->token.text;
		child_count = actual->child_count;
		children = (void**)actual->children;
	} else {
		expected = (ExpectedNode*)node;
		type = expected->type;
		text = expected->token_text;
		child_count = expected->child_count;
		children = (void**)expected->children;
	}

	for (int i = 0; i < indent; i++) printf("  ");
	printf("%s", node_type_to_string(type));
	if (text) printf("('%s')", text);
	printf("\n");

	for (int i = 0; i < child_count; i++) {
		print_ast(children[i], kind, indent + 1);
	}
}

#endif
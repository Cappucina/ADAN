#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "parser_tests.h"
#include "logs.h"

bool compare_ast(ASTNode* actual, ExpectedNode* expected) {
	if (!actual || !expected) return false;
	if (actual->type != expected->type) return false;
	if (expected->token_text && strcmp(actual->token.text, expected->token_text) != 0) return false;
	if (actual->child_count != expected->child_count) return false;

	for (int i = 0; i < actual->child_count; i++) {
		if (!compare_ast(actual->children[i], expected->children[i])) return false;
	}

	return true;
}

ExpectedNode* create_expected_assignment() {
	ExpectedNode* id_node = malloc(sizeof(ExpectedNode));

	id_node->type = AST_IDENTIFIER;
	id_node->token_text = "x";
	id_node->child_count = 0;
	id_node->children = NULL;

	ExpectedNode* type_node = malloc(sizeof(ExpectedNode));

	type_node->type = AST_TYPE;
	type_node->token_text = "int";
	type_node->child_count = 0;
	type_node->children = NULL;

	ExpectedNode* literal_node = malloc(sizeof(ExpectedNode));

	literal_node->type = AST_LITERAL;
	literal_node->token_text = "5";
	literal_node->child_count = 0;
	literal_node->children = NULL;

	ExpectedNode* assignment_node = malloc(sizeof(ExpectedNode));

	assignment_node->type = AST_DECLARATION;
	assignment_node->token_text = NULL;
	assignment_node->child_count = 3;
	assignment_node->children = malloc(sizeof(ExpectedNode*) * 3);
	assignment_node->children[0] = id_node;
	assignment_node->children[1] = type_node;
	assignment_node->children[2] = literal_node;

	return assignment_node;
}

ExpectedNode* create_expected_if() {
	ExpectedNode* condition = malloc(sizeof(ExpectedNode));

	condition->type = AST_IDENTIFIER;
	condition->token_text = "x";
	condition->child_count = 0;
	condition->children = NULL;

	ExpectedNode* body = malloc(sizeof(ExpectedNode));

	body->type = AST_BLOCK;
	body->token_text = NULL;
	body->child_count = 0;
	body->children = NULL;

	ExpectedNode* if_node = malloc(sizeof(ExpectedNode));

	if_node->type = AST_IF;
	if_node->token_text = NULL;
	if_node->child_count = 2;
	if_node->children = malloc(sizeof(ExpectedNode*) * 2);
	if_node->children[0] = condition;
	if_node->children[1] = body;

	return if_node;
}

ExpectedNode* create_expected_while() {
	ExpectedNode* left = malloc(sizeof(ExpectedNode));

	left->type = AST_IDENTIFIER;
	left->token_text = "x";
	left->child_count = 0;
	left->children = NULL;

	ExpectedNode* right = malloc(sizeof(ExpectedNode));

	right->type = AST_LITERAL;
	right->token_text = "10";
	right->child_count = 0;
	right->children = NULL;

	ExpectedNode* comparison = malloc(sizeof(ExpectedNode));

	comparison->type = AST_COMPARISON;
	comparison->token_text = NULL;
	comparison->token_type = TOKEN_LESS;
	comparison->child_count = 2;
	comparison->children = malloc(sizeof(ExpectedNode*) * 2);
	comparison->children[0] = left;
	comparison->children[1] = right;

	ExpectedNode* block = malloc(sizeof(ExpectedNode));

	block->type = AST_BLOCK;
	block->token_text = NULL;
	block->child_count = 0;
	block->children = NULL;

	ExpectedNode* while_node = malloc(sizeof(ExpectedNode));

	while_node->type = AST_WHILE;
	while_node->token_text = NULL;
	while_node->child_count = 2;
	while_node->children = malloc(sizeof(ExpectedNode*) * 2);
	while_node->children[0] = comparison;
	while_node->children[1] = block;

	return while_node;
}

ExpectedNode* create_expected_continue() {
	ExpectedNode* node = malloc(sizeof(ExpectedNode));
	node->type = AST_CONTINUE;
	node->token_text = NULL;
	node->child_count = 0;
	node->children = NULL;
	return node;
}

ExpectedNode* create_expected_if_logical(TokenType op_type, const char* op_text) {
	ExpectedNode* left = malloc(sizeof(ExpectedNode));
	left->type = AST_IDENTIFIER;
	left->token_text = "x";
	left->child_count = 0;
	left->children = NULL;

	ExpectedNode* right = malloc(sizeof(ExpectedNode));
	right->type = AST_IDENTIFIER;
	right->token_text = "y";
	right->child_count = 0;
	right->children = NULL;

	ExpectedNode* op = malloc(sizeof(ExpectedNode));
	op->type = AST_LOGICAL_OP;
	op->token_text = op_text;
	op->token_type = op_type;
	op->child_count = 2;
	op->children = malloc(sizeof(ExpectedNode*) * 2);
	op->children[0] = left;
	op->children[1] = right;

	ExpectedNode* block = malloc(sizeof(ExpectedNode));
	block->type = AST_BLOCK;
	block->token_text = NULL;
	block->child_count = 0;
	block->children = NULL;

	ExpectedNode* if_node = malloc(sizeof(ExpectedNode));
	if_node->type = AST_IF;
	if_node->token_text = NULL;
	if_node->child_count = 2;
	if_node->children = malloc(sizeof(ExpectedNode*) * 2);
	if_node->children[0] = op;
	if_node->children[1] = block;

	return if_node;
}

ExpectedNode* create_expected_if_compound() {
	ExpectedNode* id = malloc(sizeof(ExpectedNode));
	id->type = AST_IDENTIFIER;
	id->token_text = "i";
	id->child_count = 0;
	id->children = NULL;

	ExpectedNode* lit3 = malloc(sizeof(ExpectedNode));
	lit3->type = AST_LITERAL;
	lit3->token_text = "3";
	lit3->child_count = 0;
	lit3->children = NULL;

	ExpectedNode* cmp_left = malloc(sizeof(ExpectedNode));
	cmp_left->type = AST_COMPARISON;
	cmp_left->token_text = "==";
	cmp_left->token_type = TOKEN_EQUALS;
	cmp_left->child_count = 2;
	cmp_left->children = malloc(sizeof(ExpectedNode*) * 2);
	cmp_left->children[0] = id;
	cmp_left->children[1] = lit3;

	ExpectedNode* lit7 = malloc(sizeof(ExpectedNode));
	lit7->type = AST_LITERAL;
	lit7->token_text = "7";
	lit7->child_count = 0;
	lit7->children = NULL;

	ExpectedNode* id2 = malloc(sizeof(ExpectedNode));
	id2->type = AST_IDENTIFIER;
	id2->token_text = "i";
	id2->child_count = 0;
	id2->children = NULL;

	ExpectedNode* cmp_right = malloc(sizeof(ExpectedNode));
	cmp_right->type = AST_COMPARISON;
	cmp_right->token_text = "==";
	cmp_right->token_type = TOKEN_EQUALS;
	cmp_right->child_count = 2;
	cmp_right->children = malloc(sizeof(ExpectedNode*) * 2);
	cmp_right->children[0] = id2;
	cmp_right->children[1] = lit7;

	ExpectedNode* op = malloc(sizeof(ExpectedNode));
	op->type = AST_LOGICAL_OP;
	op->token_text = "||";
	op->token_type = TOKEN_OR;
	op->child_count = 2;
	op->children = malloc(sizeof(ExpectedNode*) * 2);
	op->children[0] = cmp_left;
	op->children[1] = cmp_right;

	ExpectedNode* block = malloc(sizeof(ExpectedNode));
	block->type = AST_BLOCK;
	block->token_text = NULL;
	block->child_count = 0;
	block->children = NULL;

	ExpectedNode* if_node = malloc(sizeof(ExpectedNode));
	if_node->type = AST_IF;
	if_node->token_text = NULL;
	if_node->child_count = 2;
	if_node->children = malloc(sizeof(ExpectedNode*) * 2);
	if_node->children[0] = op;
	if_node->children[1] = block;

	return if_node;
}

ExpectedNode* create_expected_if_grouped() {
	ExpectedNode* a = malloc(sizeof(ExpectedNode));
	a->type = AST_IDENTIFIER; a->token_text = "this"; a->child_count = 0; a->children = NULL;
	ExpectedNode* b = malloc(sizeof(ExpectedNode));
	b->type = AST_IDENTIFIER; b->token_text = "that"; b->child_count = 0; b->children = NULL;
	ExpectedNode* left_or = malloc(sizeof(ExpectedNode));
	left_or->type = AST_LOGICAL_OP; left_or->token_text = "||"; left_or->token_type = TOKEN_OR;
	left_or->child_count = 2; left_or->children = malloc(sizeof(ExpectedNode*) * 2);
	left_or->children[0] = a; left_or->children[1] = b;

	ExpectedNode* c = malloc(sizeof(ExpectedNode));
	c->type = AST_IDENTIFIER; c->token_text = "then"; c->child_count = 0; c->children = NULL;
	ExpectedNode* d = malloc(sizeof(ExpectedNode));
	d->type = AST_IDENTIFIER; d->token_text = "where"; d->child_count = 0; d->children = NULL;
	ExpectedNode* right_and = malloc(sizeof(ExpectedNode));
	right_and->type = AST_LOGICAL_OP; right_and->token_text = "&&"; right_and->token_type = TOKEN_AND;
	right_and->child_count = 2; right_and->children = malloc(sizeof(ExpectedNode*) * 2);
	right_and->children[0] = c; right_and->children[1] = d;

	ExpectedNode* top_and = malloc(sizeof(ExpectedNode));
	top_and->type = AST_LOGICAL_OP; top_and->token_text = "&&"; top_and->token_type = TOKEN_AND;
	top_and->child_count = 2; top_and->children = malloc(sizeof(ExpectedNode*) * 2);
	top_and->children[0] = left_or; top_and->children[1] = right_and;

	ExpectedNode* block = malloc(sizeof(ExpectedNode));
	block->type = AST_BLOCK; block->token_text = NULL; block->child_count = 0; block->children = NULL;

	ExpectedNode* if_node = malloc(sizeof(ExpectedNode));
	if_node->type = AST_IF; if_node->token_text = NULL; if_node->child_count = 2; if_node->children = malloc(sizeof(ExpectedNode*) * 2);
	if_node->children[0] = top_and; if_node->children[1] = block;
	return if_node;
}

ExpectedNode* create_expected_increment() {
	ExpectedNode* inc_var = malloc(sizeof(ExpectedNode));
	inc_var->type = AST_IDENTIFIER;
	inc_var->token_text = "i";
	inc_var->child_count = 0;
	inc_var->children = NULL;

	ExpectedNode* inc_op = malloc(sizeof(ExpectedNode));
	inc_op->type = AST_OPERATORS;
	inc_op->token_text = "++";
	inc_op->token_type = TOKEN_INCREMENT;
	inc_op->child_count = 0;
	inc_op->children = NULL;

	ExpectedNode* inc_node = malloc(sizeof(ExpectedNode));
	inc_node->type = AST_INCREMENT_EXPR;
	inc_node->token_text = NULL;
	inc_node->child_count = 2;
	inc_node->children = malloc(sizeof(ExpectedNode*) * 2);
	inc_node->children[0] = inc_var;
	inc_node->children[1] = inc_op;
	return inc_node;
}

ExpectedNode* create_expected_increment_literal() {
	ExpectedNode* lit = malloc(sizeof(ExpectedNode));
	lit->type = AST_LITERAL;
	lit->token_text = "2";
	lit->child_count = 0;
	lit->children = NULL;

	ExpectedNode* inc_op = malloc(sizeof(ExpectedNode));
	inc_op->type = AST_OPERATORS;
	inc_op->token_text = "++";
	inc_op->token_type = TOKEN_INCREMENT;
	inc_op->child_count = 0;
	inc_op->children = NULL;

	ExpectedNode* inc_node = malloc(sizeof(ExpectedNode));
	inc_node->type = AST_INCREMENT_EXPR;
	inc_node->token_text = NULL;
	inc_node->child_count = 2;
	inc_node->children = malloc(sizeof(ExpectedNode*) * 2);
	inc_node->children[0] = lit;
	inc_node->children[1] = inc_op;

	ExpectedNode* param = malloc(sizeof(ExpectedNode));
	param->type = AST_PARAMS;
	param->token_text = NULL;
	param->child_count = 1;
	param->children = malloc(sizeof(ExpectedNode*) * 1);
	param->children[0] = inc_node;

	ExpectedNode* ident = malloc(sizeof(ExpectedNode));
	ident->type = AST_IDENTIFIER;
	ident->token_text = "print";
	ident->child_count = 0;
	ident->children = NULL;

	ExpectedNode* call = malloc(sizeof(ExpectedNode));
	call->type = AST_FUNCTION_CALL;
	call->token_text = NULL;
	call->child_count = 2;
	call->children = malloc(sizeof(ExpectedNode*) * 2);
	call->children[0] = ident;
	call->children[1] = param;
	return call;
}

ExpectedNode* create_expected_for() {
	ExpectedNode* index = malloc(sizeof(ExpectedNode));

	index->type = AST_IDENTIFIER;
	index->token_text = "i";
	index->child_count = 0;
	index->children = NULL;

	ExpectedNode* type = malloc(sizeof(ExpectedNode));

	type->type = AST_TYPE;
	type->token_text = "int";
	type->child_count = 0;
	type->children = NULL;

	ExpectedNode* initial_value = malloc(sizeof(ExpectedNode));

	initial_value->type = AST_LITERAL;
	initial_value->token_text = "0";
	initial_value->child_count = 0;
	initial_value->children = NULL;

	ExpectedNode* assignment = malloc(sizeof(ExpectedNode));

	assignment->type = AST_DECLARATION;
	assignment->token_text = NULL;
	assignment->child_count = 3;
	assignment->children = malloc(sizeof(ExpectedNode*) * 3);
	assignment->children[0] = index;
	assignment->children[1] = type;
	assignment->children[2] = initial_value;

	ExpectedNode* cond_left = malloc(sizeof(ExpectedNode));

	cond_left->type = AST_IDENTIFIER;
	cond_left->token_text = "i";
	cond_left->child_count = 0;
	cond_left->children = NULL;

	ExpectedNode* cond_right = malloc(sizeof(ExpectedNode));

	cond_right->type = AST_LITERAL;
	cond_right->token_text = "10";
	cond_right->child_count = 0;
	cond_right->children = NULL;

	ExpectedNode* condition = malloc(sizeof(ExpectedNode));

	condition->type = AST_COMPARISON;
	condition->token_text = NULL;
	condition->token_type = TOKEN_LESS;
	condition->child_count = 2;
	condition->children = malloc(sizeof(ExpectedNode*) * 2);
	condition->children[0] = cond_left;
	condition->children[1] = cond_right;

	ExpectedNode* increment_var = malloc(sizeof(ExpectedNode));

	increment_var->type = AST_IDENTIFIER;
	increment_var->token_text = "i";
	increment_var->child_count = 0;
	increment_var->children = NULL;

	ExpectedNode* increment_op = malloc(sizeof(ExpectedNode));

	increment_op->type = AST_OPERATORS;
	increment_op->token_text = "++";
	increment_op->token_type = TOKEN_INCREMENT;
	increment_op->child_count = 0;
	increment_op->children = NULL;

	ExpectedNode* increment_node = malloc(sizeof(ExpectedNode));

	increment_node->type = AST_INCREMENT_EXPR;
	increment_node->token_text = NULL;
	increment_node->child_count = 2;
	increment_node->children = malloc(sizeof(ExpectedNode*) * 2);
	increment_node->children[0] = increment_var;
	increment_node->children[1] = increment_op;

	ExpectedNode* block = malloc(sizeof(ExpectedNode));

	block->type = AST_BLOCK;
	block->token_text = NULL;
	block->child_count = 0;
	block->children = NULL;

	ExpectedNode* for_node = malloc(sizeof(ExpectedNode));
	for_node->type = AST_FOR;
	for_node->token_text = NULL;
	for_node->child_count = 4;
	for_node->children = malloc(sizeof(ExpectedNode*) * 4);
	for_node->children[0] = assignment;
	for_node->children[1] = condition;
	for_node->children[2] = increment_node;
	for_node->children[3] = block;

	return for_node;
}

ExpectedNode* create_expected_mod_assign() {
	ExpectedNode* id = malloc(sizeof(ExpectedNode));
	if (!id) return NULL;
	id->type = AST_IDENTIFIER;
	id->token_text = "i";
	id->child_count = 0;
	id->children = NULL;

	ExpectedNode* lit = malloc(sizeof(ExpectedNode));
	if (!lit) {
		free(id);
		return NULL;
	}
	lit->type = AST_LITERAL;
	lit->token_text = "2";
	lit->child_count = 0;
	lit->children = NULL;

	ExpectedNode* bin = malloc(sizeof(ExpectedNode));
	if (!bin) {
		free(id);
		free(lit);
		return NULL;
	}
	bin->type = AST_BINARY_OP;
	bin->token_text = NULL;
	bin->token_type = TOKEN_PERCENT;
	bin->child_count = 2;
	bin->children = malloc(sizeof(ExpectedNode*) * 2);
	if (!bin->children) {
		free(id);
		free(lit);
		free(bin);
		return NULL;
	}
	bin->children[0] = id;
	bin->children[1] = lit;

	ExpectedNode* assign_id = malloc(sizeof(ExpectedNode));
	if (!assign_id) {
		free(bin->children);
		free(bin);
		free(id);
		free(lit);
		return NULL;
	}
	assign_id->type = AST_IDENTIFIER;
	assign_id->token_text = "i";
	assign_id->child_count = 0;
	assign_id->children = NULL;

	ExpectedNode* assign = malloc(sizeof(ExpectedNode));
	if (!assign) {
		free(bin->children);
		free(bin);
		free(id);
		free(lit);
		free(assign_id);
		return NULL;
	}
	assign->type = AST_ASSIGNMENT;
	assign->token_text = NULL;
	assign->child_count = 2;
	assign->children = malloc(sizeof(ExpectedNode*) * 2);
	if (!assign->children) {
		free(bin->children);
		free(bin);
		free(id);
		free(lit);
		free(assign_id);
		free(assign);
		return NULL;
	}
	assign->children[0] = assign_id;
	assign->children[1] = bin;

	return assign;
}

ExpectedNode* create_expected_program() {
	ExpectedNode* decl = create_expected_assignment();
	ExpectedNode* type = malloc(sizeof(ExpectedNode));
	
	type->type = AST_TYPE;
	type->token_text = "void";
	type->child_count = 0;
	type->children = NULL;

	ExpectedNode* id = malloc(sizeof(ExpectedNode));
	
	id->type = AST_IDENTIFIER;
	id->token_text = "test";
	id->child_count = 0;
	id->children = NULL;

	ExpectedNode* params = malloc(sizeof(ExpectedNode));
	
	params->type = AST_PARAMS;
	params->token_text = NULL;
	params->child_count = 0;
	params->children = NULL;

	ExpectedNode* block = malloc(sizeof(ExpectedNode));
	
	block->type = AST_BLOCK;
	block->token_text = NULL;
	block->child_count = 1;
	block->children = malloc(sizeof(ExpectedNode*));
	block->children[0] = decl;

	ExpectedNode* program_node = malloc(sizeof(ExpectedNode));
	
	program_node->type = AST_PROGRAM;
	program_node->token_text = NULL;
	program_node->child_count = 4;
	program_node->children = malloc(sizeof(ExpectedNode*) * 4);
	program_node->children[0] = type;
	program_node->children[1] = id;
	program_node->children[2] = params;
	program_node->children[3] = block;

	return program_node;
}

void free_expected_ast(ExpectedNode* node) {
	if (!node) return;
	if (node->children) {
		for (int i = 0; i < node->child_count; i++) {
			free_expected_ast(node->children[i]);
		}
		free(node->children);
	}
	free(node);
}

void run_parser_test(const char* input, ExpectedNode* expected_ast) {
	Lexer* lexer = create_lexer(input);
	Parser parser;

	init_parser(&parser, lexer);

	ASTNode* ast = parse_statement(&parser);

	if (parser.error) {
		printf("PARSE FAIL: '%s'\n  Error: %s\n", input, parser.error_message ? parser.error_message : "Unknown error");
	} else if (!ast) {
		printf("PARSE FAIL: '%s'\n  Error: %s\n", input, ParserErrorMessages[PARSER_NULL_TEST]);
	} else if (!compare_ast(ast, expected_ast)) {
		if (expected_ast == NULL) {
			if (ast->type != AST_FUNCTION_CALL) {
				printf("PARSE FAIL (custom): '%s'\n  Error: not a function call\n", input);
			} else {
				ASTNode* ident = ast->children[0];
				ASTNode* args = ast->children[1];
				if (!ident || ident->type != AST_IDENTIFIER || strcmp(ident->token.text, "print") != 0) {
					printf("PARSE FAIL (custom): '%s'\n  Error: first child not print identifier\n", input);
				} else if (!args || args->type != AST_PARAMS || args->child_count != 1) {
					printf("PARSE FAIL (custom): '%s'\n  Error: wrong args\n", input);
				} else {
					ASTNode* param = args->children[0];
					if (param->type != AST_FUNCTION_CALL || strcmp(param->children[0]->token.text, "test") != 0) {
						printf("PARSE FAIL (custom): '%s'\n  Error: expected param to be test() call\n", input);
					}
				}
			}
		} else {
			printf("PARSE FAIL: '%s'\n  Error: %s\n", input, ParserErrorMessages[PARSER_AST_MISMATCH]);
			printf("Actual AST:\n");
			print_ast(ast, NODE_ACTUAL, 0);
			printf("Expected AST:\n");
			print_ast(expected_ast, NODE_EXPECTED, 0);
			return;
		}
	}

	if (ast) free_ast(ast);
	free_parser(&parser);
	free(lexer);
}

void create_parser_tests() {
	ParserTest tests[] = {
		{ "x::int = 5;", create_expected_assignment() },
		{ "if (x) {}", create_expected_if() },
		{ "while (x < 10) {}", create_expected_while() },
		{ "i %= 2;", create_expected_mod_assign() },
		{ "for (i::int = 0; i < 10; i++) {}", create_expected_for() },
		{ "program::void test() { x::int = 5; }", create_expected_program() },
		{ "print(\"${test()}\");", NULL },
		{ "print(\"${2++}\");", create_expected_increment_literal() },
		{ "i++;", create_expected_increment() },
		{ "continue;", create_expected_continue() },
		{ "if (x || y) {}", create_expected_if_logical(TOKEN_OR, "||") },
		{ "if (x && y) {}", create_expected_if_logical(TOKEN_AND, "&&") },
		{ "if (i == 3 || i == 7) {}", create_expected_if_compound() },
		{ "if ((this || that) && (then && where)) {}", create_expected_if_grouped() },
	};

	int num_tests = sizeof(tests) / sizeof(tests[0]);
	for (int i = 0; i < num_tests; i++) {
		run_parser_test(tests[i].input, tests[i].expected_ast);
		if (tests[i].expected_ast) {
			free_expected_ast(tests[i].expected_ast);
		}
	}

	{
		const char* file_input = "i::int = 0; program::void main() { }";
		Lexer* lexer = create_lexer(file_input);
		Parser parser;
		init_parser(&parser, lexer);
		ASTNode* file_node = parse_file(&parser);
		if (parser.error) {
			printf("PARSE FILE FAIL: '%s'\n  Error: %s\n", file_input, parser.error_message ? parser.error_message : "Unknown error");
		} else if (!file_node || file_node->type != AST_FILE) {
			printf("PARSE FILE FAIL: '%s'\n  Error: expected AST_FILE\n", file_input);
		}

		if (file_node) free_ast(file_node);
		free_parser(&parser);
		free(lexer);
	}
}
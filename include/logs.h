#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <stdarg.h>
#include "lexer.h"
#include "ast.h"

typedef enum {
	LEXER_TYPE_MISMATCH,
	LEXER_TEXT_MISMATCH
} LexerErrors;

static const char* LexerErrorMessages[] = {
	[LEXER_TYPE_MISMATCH] = "Token %d type mismatch: expected type %d, got %d",
	[LEXER_TEXT_MISMATCH] = "Token %d text mismatch: expected '%s', got '%s'"
};

typedef enum {
	PARSER_INITIALIZATION_FAILURE,
	PARSER_UNEXPECTED_TOKEN,
	PARSER_EXPECTED,
	PARSER_FAILED_AST,
	PARSER_ALLOCATION_FAILURE,
	PARSER_TRAILING_COMMA,
	PARSER_NULL_TEST,
	PARSER_AST_MISMATCH
} ParserErrors;

static const char* ParserErrorMessages[] = {
	[PARSER_INITIALIZATION_FAILURE] = "Failed to initialize parser: could not fetch initial tokens",
	[PARSER_UNEXPECTED_TOKEN] = "Unexpected token '%s'",
	[PARSER_EXPECTED] = "Expected %s, got '%s'",
	[PARSER_FAILED_AST] = "Failed to create AST node for %s",
	[PARSER_ALLOCATION_FAILURE] = "Memory allocation failed for %s",
	[PARSER_TRAILING_COMMA] = "Trailing comma in parameter list is not allowed",
	[PARSER_NULL_TEST] = "Parser test failed: parse_statement returned NULL",
	[PARSER_AST_MISMATCH] = "AST mismatch: generated tree does not match expected structure"
};
 
typedef enum {
	SEMANTIC_DUPLICATE_SYMBOL,
	SEMANTIC_TYPE_MISMATCH,
	SEMANTIC_INVALID_ASSIGNMENT,
	SEMANTIC_RETURN_TYPE_MISMATCH,
	SEMANTIC_MISSING_RETURN,
	SEMANTIC_VOID_RETURN_WITH_VALUE,
	SEMANTIC_NON_VOID_RETURN_WITHOUT_VALUE,
	SEMANTIC_CONDITION_NOT_BOOLEAN,
	SEMANTIC_INVALID_OPERAND_TYPE,
	SEMANTIC_BINARY_OP_TYPE_MISMATCH,
	SEMANTIC_UNARY_OP_INVALID_TYPE,
	SEMANTIC_FUNCTION_NOT_FOUND,
	SEMANTIC_WRONG_ARGUMENT_COUNT,
	SEMANTIC_ARGUMENT_TYPE_MISMATCH,
	SEMANTIC_ARRAY_INDEX_NOT_INTEGER,
	SEMANTIC_INVALID_ARRAY_ACCESS,
	SEMANTIC_BREAK_OUTSIDE_LOOP,
	SEMANTIC_DIVISION_BY_ZERO,
	SEMANTIC_UNKNOWN_STATEMENT,
	SEMANTIC_RETURN_VALUE_TYPE_MISMATCH,
	SEMANTIC_UNKNOWN_VARIABLE,
	SEMANTIC_ARRAY_MIXED_TYPES,
	SEMANTIC_NO_IMPLICIT_CONVERSION,
	SEMANTIC_INVALID_CAST,
	SEMANTIC_INT_FLOAT_MISMATCH,
	SEMANTIC_STRING_NUMERIC_OPERATION,
	SEMANTIC_BOOLEAN_NUMERIC_OPERATION,
	SEMANTIC_INCOMPATIBLE_OPERAND_TYPES,
	SEMANTIC_VOID_IN_EXPRESSION,
	SEMANTIC_NULL_IN_ARITHMETIC,
	SEMANTIC_ARRAY_IN_ARITHMETIC,
	SEMANTIC_MISMATCHED_ARRAY_ELEMENT_TYPE
} SemanticErrors;

static const char* SemanticErrorMessages[] = {
	[SEMANTIC_DUPLICATE_SYMBOL] = "Symbol '%s' is already defined in this scope",
	[SEMANTIC_TYPE_MISMATCH] = "Cannot assign type '%s' to variable of type '%s'",
	[SEMANTIC_INVALID_ASSIGNMENT] = "Left-hand side of assignment must be a mutable variable",
	[SEMANTIC_RETURN_TYPE_MISMATCH] = "Function expects return type '%s', got '%s'",
	[SEMANTIC_MISSING_RETURN] = "Non-void function must return a value on all code paths",
	[SEMANTIC_VOID_RETURN_WITH_VALUE] = "Void function cannot return a value",
	[SEMANTIC_NON_VOID_RETURN_WITHOUT_VALUE] = "Function must return a value of type '%s'",
	[SEMANTIC_CONDITION_NOT_BOOLEAN] = "Condition must be of type 'boolean', got '%s'",
	[SEMANTIC_INVALID_OPERAND_TYPE] = "Operator '%s' cannot be applied to type '%s'",
	[SEMANTIC_BINARY_OP_TYPE_MISMATCH] = "Operator '%s' requires matching types, got '%s' and '%s'",
	[SEMANTIC_UNARY_OP_INVALID_TYPE] = "Operator '%s' cannot be applied to type '%s'",
	[SEMANTIC_FUNCTION_NOT_FOUND] = "Function '%s' is not defined",
	[SEMANTIC_WRONG_ARGUMENT_COUNT] = "Function expects %d argument(s), got %d",
	[SEMANTIC_ARGUMENT_TYPE_MISMATCH] = "Argument %d expects type '%s', got '%s'",
	[SEMANTIC_ARRAY_INDEX_NOT_INTEGER] = "Array index must be of type 'int', got '%s'",
	[SEMANTIC_INVALID_ARRAY_ACCESS] = "Cannot index into non-array type '%s'",
	[SEMANTIC_DIVISION_BY_ZERO] = "Division by zero will cause a runtime error",
	[SEMANTIC_UNKNOWN_STATEMENT] = "Unknown statement type: '%s'",
	[SEMANTIC_BREAK_OUTSIDE_LOOP] = "'break' can only be used inside loops",
	[SEMANTIC_RETURN_VALUE_TYPE_MISMATCH] = "Return type '%s' does not match function return type '%s'",
	[SEMANTIC_UNKNOWN_VARIABLE] = "Variable '%s' is not defined",
	[SEMANTIC_ARRAY_MIXED_TYPES] = "Array elements must all be the same type",
	[SEMANTIC_NO_IMPLICIT_CONVERSION] = "Cannot implicitly convert '%s' to '%s'",
	[SEMANTIC_INVALID_CAST] = "Cannot cast from type '%s' to type '%s'",
	[SEMANTIC_INT_FLOAT_MISMATCH] = "Cannot mix 'int' and 'float' without explicit conversion",
	[SEMANTIC_STRING_NUMERIC_OPERATION] = "Cannot perform arithmetic on type 'string'",
	[SEMANTIC_BOOLEAN_NUMERIC_OPERATION] = "Cannot perform arithmetic on type 'boolean'",
	[SEMANTIC_INCOMPATIBLE_OPERAND_TYPES] = "Operator '%s' cannot be applied to types '%s' and '%s'",
	[SEMANTIC_VOID_IN_EXPRESSION] = "Type 'void' cannot be used in expressions",
	[SEMANTIC_NULL_IN_ARITHMETIC] = "Type 'null' cannot be used in arithmetic operations",
	[SEMANTIC_ARRAY_IN_ARITHMETIC] = "Arrays cannot be used in arithmetic operations",
	[SEMANTIC_MISMATCHED_ARRAY_ELEMENT_TYPE] = "Array element at index %d has type '%s', expected '%s'"
};

typedef enum {
	SEMANTIC_UNUSED_VARIABLE,
	SEMANTIC_POSSIBLE_NULL_REFERENCE,
	SEMANTIC_DEPRECATED_FUNCTION,
	SEMANTIC_POTENTIAL_OVERFLOW,
	SEMANTIC_UNREACHABLE_CODE
} SemanticWarnings;

static const char* SemanticWarningMessages[] = {
	[SEMANTIC_UNUSED_VARIABLE] = "Variable '%s' is declared but never used",
	[SEMANTIC_POSSIBLE_NULL_REFERENCE] = "Possible null reference for '%s'",
	[SEMANTIC_DEPRECATED_FUNCTION] = "Function '%s' is deprecated",
	[SEMANTIC_POTENTIAL_OVERFLOW] = "Potential overflow in operation involving '%s'",
	[SEMANTIC_UNREACHABLE_CODE] = "Unreachable code after '%s'"
};

typedef enum {
	SEMANTIC_TIP_SIMPLIFY_EXPRESSION,
	SEMANTIC_TIP_REPLACE_LOOP_WITH_ITERATOR,
	SEMANTIC_TIP_PREFER_WHILE_LOOP
} SemanticTips;

static const char* SemanticTipMessages[] = {
	[SEMANTIC_TIP_SIMPLIFY_EXPRESSION] = "Expression '%s' can be simplified",
	[SEMANTIC_TIP_REPLACE_LOOP_WITH_ITERATOR] = "Consider using an iterator for '%s'",
	[SEMANTIC_TIP_PREFER_WHILE_LOOP] = "Consider using a 'while' loop instead of 'for' without increment"
};

static inline void log_error(const char* file, int line, int column, const char* fmt, ...) {
	fprintf(stderr, "\033[1;37m%s:%d:%d:\033[0m \033[1;31merror:\033[0m ", file ? file : "<unknown>", line, column);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

static inline void log_warning(const char* file, int line, int column, const char* fmt, ...) {
	fprintf(stdout, "\033[1;37m%s:%d:%d:\033[0m \033[1;33mwarning:\033[0m ", file ? file : "<unknown>", line, column);
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fprintf(stdout, "\n");
}

static inline void log_tip(const char* file, int line, int column, const char* fmt, ...) {
	fprintf(stdout, "\033[1;37m%s:%d:%d:\033[0m \033[1;36mtip:\033[0m ", file ? file : "<unknown>", line, column);
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fprintf(stdout, "\n");
}

static inline void log_lexer_error(int line, int column, const char* fmt, ...) {
	fprintf(stderr, "\033[1;37m<stdin>:%d:%d:\033[0m \033[1;31merror:\033[0m ", line, column);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

static inline void log_parser_error(Token* token, const char* fmt, ...) {
	fprintf(stderr, "\033[1;37m<stdin>:%d:%d:\033[0m \033[1;31merror:\033[0m ", token ? token->line : 0, token ? token->column : 0);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

static inline void log_semantic_error(ASTNode* node, const char* fmt, ...) {
	fprintf(stderr, "\033[1;37m<stdin>:%d:%d:\033[0m \033[1;31merror:\033[0m ", node ? node->token.line : 0, node ? node->token.column : 0);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}

static inline void log_semantic_warning(ASTNode* node, const char* fmt, ...) {
	fprintf(stdout, "\033[1;37m<stdin>:%d:%d:\033[0m \033[1;33mwarning:\033[0m ", node ? node->token.line : 0, node ? node->token.column : 0);
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fprintf(stdout, "\n");
}

static inline void log_semantic_tip(ASTNode* node, const char* fmt, ...) {
	fprintf(stdout, "\033[1;37m<stdin>:%d:%d:\033[0m \033[1;36mtip:\033[0m ", node ? node->token.line : 0, node ? node->token.column : 0);
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fprintf(stdout, "\n");
}

#endif

extern int VERBOSE;
// 
//  All errors, warnings, etc. for the Lexical Analysis
//   process
// 
typedef enum {
	LEXER_TYPE_MISMATCH,
	LEXER_TEXT_MISMATCH
} LexerErrors;

static const char* LexerErrorMessages[] = {
	[LEXER_TYPE_MISMATCH] = "Token %d type mismatch in lexer: found '%d', expected '%d'\n",
	[LEXER_TEXT_MISMATCH] = "Token %d text mismatch in lexer: found '%s', expected '%s'\n"
};

// 
//  Issues for the Parsing process
// 
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
	[PARSER_INITIALIZATION_FAILURE] = "Couldn't initialize the parser due to a failure when fetching the initial tokens.",
	[PARSER_UNEXPECTED_TOKEN] = "Couldn't parse statements because of an unexpected token: %s",
	[PARSER_EXPECTED] = "Expected %s, instead got %s",
	[PARSER_FAILED_AST] = "Couldn't create AST nodes for %s",
	[PARSER_ALLOCATION_FAILURE] = "Couldn't allocate memory for %s",
	[PARSER_TRAILING_COMMA] = "Found trailing comma in a parameter list, please remove it and try again.",
	[PARSER_NULL_TEST] = "The parser test failed because `parse_statement` function returned NULL",
	[PARSER_AST_MISMATCH] = "The AST generated and the AST to be expected are not identical!"
};

// 
//  Handling issues for Semantic Analysis
// 
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
	SEMANTIC_WRONG_ASSIGNMENT_COUNT,
	SEMANTIC_ARGUMENT_TYPE_MISMATCH,
	SEMANTIC_ARRAY_INDEX_NOT_INTEGER,
	SEMANTIC_INVALID_ARRAY_ACCESS,
	SEMANTIC_BREAK_OUTSIDE_LOOP,
	SEMANTIC_DIVISION_BY_ZERO,
	SEMANTIC_UNKNOWN_STATEMENT,
	SEMANTIC_RETURN_KEYWORD_TYPE_MISMATCH
} SemanticErrors;

static const char* SemanticErrorMessages[] = {
	[SEMANTIC_DUPLICATE_SYMBOL] = "Duplicate symbol detected: %s.",
	[SEMANTIC_TYPE_MISMATCH] = "Type mismatch: cannot assign %s to %s.",
	[SEMANTIC_INVALID_ASSIGNMENT] = "Invalid assignment: left-hand side must be a variable.",
	[SEMANTIC_RETURN_TYPE_MISMATCH] = "Return type mismatch: function returns %s but got %s.",
	[SEMANTIC_MISSING_RETURN] = "Missing return statement in non-void function.",
	[SEMANTIC_VOID_RETURN_WITH_VALUE] = "Cannot return a value from a void function.",
	[SEMANTIC_NON_VOID_RETURN_WITHOUT_VALUE] = "Non-void function must return a value.",
	[SEMANTIC_CONDITION_NOT_BOOLEAN] = "Condition expression must be boolean, got %s.",
	[SEMANTIC_INVALID_OPERAND_TYPE] = "Invalid operand type for operator %s: %s.",
	[SEMANTIC_BINARY_OP_TYPE_MISMATCH] = "Binary operator %s expects operands of type %s, but got %s and %s.",
	[SEMANTIC_UNARY_OP_INVALID_TYPE] = "Unary operator %s cannot be applied to type %s.",
	[SEMANTIC_FUNCTION_NOT_FOUND] = "Function not found: %s.",
	[SEMANTIC_WRONG_ASSIGNMENT_COUNT] = "Wrong number of values in assignment: expected %d, got %d.",
	[SEMANTIC_ARGUMENT_TYPE_MISMATCH] = "Function %s argument type mismatch at position %d: expected %s, got %s.",
	[SEMANTIC_ARRAY_INDEX_NOT_INTEGER] = "Array index must be an integer, got %s.",
	[SEMANTIC_INVALID_ARRAY_ACCESS] = "Invalid array access: %s is not an array or index out of bounds.",
	[SEMANTIC_DIVISION_BY_ZERO] = "Division by zero detected.",
	[SEMANTIC_UNKNOWN_STATEMENT] = "Unknown statement type: %s.",
	[SEMANTIC_BREAK_OUTSIDE_LOOP] = "Cannot use the 'break' keyword outside of loops.",
	[SEMANTIC_RETURN_KEYWORD_TYPE_MISMATCH] = "Types did not match when attempting a return."
};


typedef enum {
	SEMANTIC_UNUSED_VARIABLE,
	SEMANTIC_POSSIBLE_NULL_REFERENCE,
	SEMANTIC_DEPRECATED_FUNCTION,
	SEMANTIC_POTENTIAL_OVERFLOW,
	SEMANTIC_UNREACHABLE_CODE
} SemanticWarnings;

static const char* SemanticWarningMessages[] = {
	[SEMANTIC_UNUSED_VARIABLE] = "Warning: Variable %s is declared but never used.",
	[SEMANTIC_POSSIBLE_NULL_REFERENCE] = "Warning: Possible null reference detected for %s.",
	[SEMANTIC_DEPRECATED_FUNCTION] = "Warning: Function %s is deprecated and may be removed in future versions.",
	[SEMANTIC_POTENTIAL_OVERFLOW] = "Warning: Potential overflow detected in operation involving %s.",
	[SEMANTIC_UNREACHABLE_CODE] = "Warning: Unreachable code detected after statement %s."
};


typedef enum {
	SEMANTIC_TIP_SIMPLIFY_EXPRESSION,
	SEMANTIC_TIP_REPLACE_LOOP_WITH_ITERATOR,
	SEMANTIC_TIP_PREFER_WHILE_LOOP
} SemanticTips;

static const char* SemanticTipMessages[] = {
	[SEMANTIC_TIP_SIMPLIFY_EXPRESSION] = "Tip: Expression %s can be simplified for better performance or clarity.",
	[SEMANTIC_TIP_REPLACE_LOOP_WITH_ITERATOR] = "Tip: Consider using an iterator instead of a manual loop over %s.",
	[SEMANTIC_TIP_PREFER_WHILE_LOOP] = "Tip: You should prefer using a while loop instead of a for loop that doesn't increment."
};
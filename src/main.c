#include <stdio.h>
#include "lexer_tests.h"
#include "parser_tests.h"
#include "lexer.h"

// TODO: Add logic to compile code rather than test our own syntax
int main() {
	int lexer_failures = create_lexer_tests();
	int parser_failures = create_parser_tests();

	int total_failures = lexer_failures + parser_failures;

	if (total_failures == 0) {
		printf("All tests passed\n");
		return 0;
	} else {
		printf("Total failures: %d\n", total_failures);
		return total_failures;
	}
}
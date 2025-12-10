#include <stdio.h>
#include "lexer_tests.h"
#include "parser_tests.h"
#include "lexer.h"
#include "compiler_flags.h"
#include "compiler_handler.h"

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main(int argc, char **argv) {
	// Parse command-line flags
	CompilerFlags* flags = flags_init();
	if (flags == NULL) {
		fprintf(stderr, "Error: Failed to initialize compiler flags\n");
		return -1;
	}
	
	if (flags_parse(argc, argv, flags) != 0) {
		fprintf(stderr, "Error: Failed to parse command-line arguments\n");
		flags_free(flags);
		return -1;
	}
	
	// Handle help/version flags
	if (flags->show_help) {
		flags_print_help(argv[0]);
		flags_free(flags);
		return 0;
	}
	
	if (flags->show_version) {
		flags_print_version();
		flags_free(flags);
		return 0;
	}
	
	if (flags->test_compiler) {
		printf("Running tests...\n");
		
		int lexer_failures = create_lexer_tests();
		int parser_failures = create_parser_tests();

		int total_failures = lexer_failures + parser_failures;

		if (total_failures == 0) {
			printf("All tests passed\n");
			flags_free(flags);
			return 0;
		} else {
			printf("Total failures: %d\n", total_failures);
			flags_free(flags);
			return -1;
		}
	}

	// Validate flags
	if (flags_validate(flags) != 0) {
		fprintf(stderr, "Error: Invalid compiler flags\n");
		flags_free(flags);
		return -1;
	}
	
	// Print configuration if verbose
	if (flags->verbose) {
		printf("ADAN Compiler Configuration:\n");
		printf("  Input File: %s\n", flags->input_file ? flags->input_file : "stdin");
		printf("  Output File: %s\n", flags->output_file ? flags->output_file : "a.out");
		printf("  Target Architecture: %s\n", arch_to_string(flags->target_arch));
		printf("  Target OS: %s\n", os_to_string(flags->target_os));
		printf("  Optimization Level: %d\n", flags->opt_level);
		printf("  Cross-Compiling: %s\n", flags->is_cross_compile ? "Yes" : "No");
		if (flags->target_triple) {
			printf("  Target Triple: %s\n", flags->target_triple);
		}
		printf("\n");
	}
	
	// TODO: Implement actual compilation pipeline
	// This will call: lexer -> parser -> semantic -> IR -> codegen -> assemble -> link
	int res = compileHandler(flags);

	printf("The compiler is still in development and generated code may not be accurate.\n");
	
	flags_free(flags);
	return res;
}
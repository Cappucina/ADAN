#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "helper.h"
#include "stm.h"
#include "frontend/scanner/scanner.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/tree.h"
#include "frontend/semantics/semantic.h"
#include "backend/lower.h"
#include "backend/ir/ir.h"
#include "backend/backend.h"

bool has_valid_extension(const char* filename)
{
	size_t len = strlen(filename);
	if (len < 5)
		return false;

	if (strcmp(filename + len - 4, ".adn") == 0)
		return true;
	if (len >= 6 && strcmp(filename + len - 5, ".adan") == 0)
		return true;

	return false;
}

void print_usage(const char* program_name)
{
	printf("Usage: %s -f <file.adn> | --file <file.adn>\n", program_name);
	printf("File must have .adn or .adan extension\n");
}

int main(int argc, char* argv[])
{
	char* file_path = NULL;

	if (argc < 3)
	{
		print_usage(argv[0]);
		return 1;
	}

	for (int i = 1; i < argc - 1; i++)
	{
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			file_path = argv[i + 1];
			break;
		}
	}

	if (!file_path)
	{
		fprintf(stderr, "Error: No file specified\n");
		print_usage(argv[0]);
		return 1;
	}

	if (!has_valid_extension(file_path))
	{
		fprintf(stderr, "Error: File must have .adn or .adan extension\n");
		return 1;
	}

	char* source = read_file(file_path);
	if (!source)
	{
		fprintf(stderr, "Failed to read source file! (Error)\n");
		return 1;
	}

	SymbolTableStack* global_stack = sts_init();
	Scanner* scanner = scanner_init(source);
	Parser* parser = parser_init(scanner);
	if (parser)
	{
		parser->allow_undefined_symbols = true;
	}

	ASTNode* ast = parser_parse_program(parser);

	parser_free(parser);
	scanner_free(scanner);

	if (ast)
	{
		printf("AST created successfully! (Info)\n");
		// ast_print(ast, 0);

		SemanticAnalyzer* analyzer = semantic_init(ast, global_stack);
		if (analyzer)
		{
			if (!semantic_analyze(analyzer))
			{
				fprintf(stderr,
				        "Semantic analysis failed with %d error(s). (Error)\n",
				        analyzer->error_count);
			}
			else
			{
				printf("Semantic analysis completed successfully! (Info)\n");

				IRModule* ir = ir_module_create();
				if (!ir)
				{
					fprintf(stderr, "Failed to create IR module. (Error)\n");
				}
				else
				{
					Program program = {.ast_root = ast, .ir = ir};
					lower_program(&program);
					printf("IR generation completed! (Info)\n");
					// ir_print_module(ir, stdout);
					ir_print_module(ir, stdout);
					ir_module_destroy(ir);
				}
			}
			semantic_free(analyzer);
		}
		else
		{
			fprintf(stderr, "Failed to initialize semantic analyzer! (Error)\n");
		}

		ast_free(ast);
	}
	sts_free(global_stack);
	free(source);

	return 0;
}
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
#include "backend/linker/linker.h"
#include <limits.h>

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
	int do_link = 0;
	char* libs = NULL;
	char* out_path = NULL;

	if (argc < 3)
	{
		print_usage(argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if (i + 1 < argc)
				file_path = argv[i + 1];
		}
		else if (strcmp(argv[i], "--link") == 0)
		{
			do_link = 1;
		}
		else if (strcmp(argv[i], "--libs") == 0)
		{
			if (i + 1 < argc)
				libs = argv[i + 1];
		}
		else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
		{
			if (i + 1 < argc)
				out_path = argv[i + 1];
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

				char* ll_path = NULL;
				if (file_path)
				{
					size_t flen = strlen(file_path);
					const char* dot = strrchr(file_path, '.');
					size_t base_len = dot ? (size_t)(dot - file_path) : flen;
					ll_path = (char*)malloc(base_len + 4 + 1);
					if (ll_path)
					{
						memcpy(ll_path, file_path, base_len);
						strcpy(ll_path + base_len, ".ll");
					}
				}

				int emit_res = -1;
				if (ll_path)
				{
					emit_res = backend_compile_ast_to_llvm_file(ast, ll_path);
					if (emit_res == 0)
						printf("LLVM IR emitted to %s\n", ll_path);
					else
						fprintf(stderr, "Failed to emit LLVM IR to %s\n", ll_path);
				}

				if (do_link && ll_path && emit_res == 0)
				{
					const char* outp = out_path ? out_path : "a.out";
					int lres = linker_link_with_clang(ll_path, outp, libs ? libs : "");
					if (lres == 0)
						printf("Linked executable: %s\n", outp);
					else
						fprintf(stderr, "Linking failed (code=%d)\n", lres);
				}

				if (ll_path)
					free(ll_path);
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
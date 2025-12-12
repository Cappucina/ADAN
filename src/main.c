#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer_tests.h"
#include "parser_tests.h"
#include "codegen_tests.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "liveness.h"
#include "codegen.h"
#include "library.h"
#include <signal.h>
#include <unistd.h>

LibraryRegistry* global_library_registry = NULL;

static void handle_timeout(int signum) {
	(void)signum;
	fprintf(stderr, "error: compilation timed out\n");
	exit(1);
}

char* read_file_source(const char* file_path) {
	FILE* fp = fopen(file_path, "rb");
	if (!fp) return NULL;

	unsigned char bom[3];
	size_t n = fread(bom, 1, 3, fp);
	if (!(n == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
		fseek(fp, 0, SEEK_SET);
	}

	size_t cap = 1024, len = 0;
	char* out = malloc(cap);
	if (!out) {
		fclose(fp);
	
		return NULL;
	}

	char buf[256];
	while (fgets(buf, sizeof(buf), fp)) {
		size_t blen = strlen(buf);
		if (len + blen + 1 > cap) {
			cap *= 2;
			char* tmp = realloc(out, cap);
			
			if (!tmp) {
				free(out);
				fclose(fp);
				
				return NULL;
			}
			
			out = tmp;
		}

		memcpy(out + len, buf, blen);
		len += blen;
	}

	out[len] = '\0';
	fclose(fp);
	
	return out;
}

int main(int argc, char** argv) {
	global_library_registry = init_library_registry("lib");

	create_lexer_tests();
	create_parser_tests();
	create_codegen_tests();

	signal(SIGALRM, handle_timeout);
	alarm(10);

	if (argc < 2) {
		printf("No input file provided\n");
		return 1;
	}

	char* file_source = read_file_source(argv[1]);
	if (!file_source) {
		printf("Failed to read source file\n");
		return 1;
	}

	Lexer* lexer = create_lexer(file_source);
	if (!lexer) {
		free(file_source);
		return 1;
	}

	Parser parser;
	init_parser(&parser, lexer);

	if (parser.error) {
		if (parser.error_message) printf("%s\n", parser.error_message);
		free_parser(&parser);
		free(lexer);
		free(file_source);
		return 1;
	}

	ASTNode* ast = parse_file(&parser);
	if (!ast || parser.error) {
		if (parser.error_message) printf("%s\n", parser.error_message);
		free_parser(&parser);
		free(lexer);
		free(file_source);
		return 1;
	}

	SymbolTable* symbols = init_symbol_table();
	if (!symbols) {
		free_ast(ast);
		free_parser(&parser);
		free(lexer);
		free(file_source);
		return 1;
	}


	analyze_file(ast, symbols);
	check_entry_point(symbols);
	analyze_variable_usage(symbols);

	if (semantic_get_error_count() > 0) {
		free_symbol_table(symbols);
		free_ast(ast);
		free_parser(&parser);
		free(lexer);
		free(file_source);
		free_library_registry(global_library_registry);
		return 1;
	}

	char* register_names[] = {"rbx", "r10", "r11", "r12"};
	int caller_saved[] = {0, 1};
	TargetConfig config;
	init_target_config(&config, 4, register_names, 2, caller_saved, 8);
	
	FILE* asm_file = fopen("compiled/assembled.s", "w");
	if (!asm_file) {
		free_symbol_table(symbols);
		free_ast(ast);
		free_parser(&parser);
		free(lexer);
		free(file_source);
		return 1;
	}
	
	init_ir_full();
	
	for (int i = 0; i < ast->child_count; i++) {
		ASTNode* child = ast->children[i];
		if (child && child->type == AST_PROGRAM && child->child_count > 3) {
			generate_ir(child);
		}
	}
	
	if (global_library_registry) {
		Library* lib = global_library_registry->libraries;
		while (lib) {
			if (lib->ast && lib->ast->child_count > 0) {
				for (int i = 0; i < lib->ast->child_count; i++) {
					ASTNode* func_node = lib->ast->children[i];
					if (func_node && func_node->type == AST_PROGRAM && func_node->child_count > 3) {
						generate_ir(func_node);
					}
				}
			}
			lib = lib->next;
		}
	}
	
	StringLiteral* strings = get_string_literals();
	if (strings) {
		fprintf(asm_file, ".section .rodata\n");
		StringLiteral* current = strings;
		while (current) {
			fprintf(asm_file, "%s:\n", current->label);
			fprintf(asm_file, "  .string \"%s\"\n", current->value);
			current = current->next;
		}
		fprintf(asm_file, "\n");
	}
	
	fprintf(asm_file, ".text\n");
	
	IRInstruction* all_ir = get_ir_head();
	if (all_ir) {
		number_instructions(all_ir);
		LiveInterval* intervals = compute_liveness(all_ir);
		assign_stack_offsets(intervals, &config);
		int frame_size = compute_spill_frame_size(intervals, &config);
		
		fprintf(asm_file, ".globl main\n");
		int stack_bytes = frame_size;
		if (stack_bytes < 0) stack_bytes = 0;
		// Align to 16 bytes to keep stack ABI compliant when calling
		stack_bytes = (stack_bytes + 15) & ~15;
		generate_asm(all_ir, intervals, &config, asm_file, stack_bytes);
	}
	
	fclose(asm_file);
	free_symbol_table(symbols);
	free_ast(ast);
	free_parser(&parser);
	free(lexer);
	free(file_source);
	free_library_registry(global_library_registry);

	// print_ir();

	return 0;
}

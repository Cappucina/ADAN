#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "lexer_tests.h"
#include "parser_tests.h"
#include "codegen_tests.h"
#include "lexer.h"
#include "logs.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "liveness.h"
#include "codegen.h"
#include "library.h"
#include <signal.h>
#include <unistd.h>

int VERBOSE = 0;

static char* escape_for_asm(const char* s) {
	if (!s) return strdup("");
	int cap = 64, len = 0;
	char* out = malloc(cap);
	if (!out) return NULL;
	for (int i = 0; s[i] != '\0'; i++) {
		char c = s[i];
		const char* esc = NULL;
		char tmp[3] = {0};
		switch (c) {
			case '\n': esc = "\\n"; break;
			case '\t': esc = "\\t"; break;
			case '\r': esc = "\\r"; break;
			case '"': esc = "\\\""; break;
			case '\\': esc = "\\\\"; break;
			default:
				tmp[0] = c;
				esc = tmp;
				break;
		}
		int add = strlen(esc);
		if (len + add + 1 >= cap) {
			cap *= 2;
			char* tmp2 = realloc(out, cap);
			if (!tmp2) { free(out); return NULL; }
			out = tmp2;
		}
		for (int j = 0; j < add; j++) out[len++] = esc[j];
	}
	out[len] = '\0';
	return out;
}

LibraryRegistry* global_library_registry = NULL;

static void handle_timeout(int signum) {
	(void)signum;
	fprintf(stderr, "error: compilation timed out\n");
	exit(1);
}

static void runtime_signal_handler(int signum, siginfo_t* info, void* ctx) {
	(void)ctx;
	void* addr = info ? info->si_addr : NULL;

	switch (signum) {
		case SIGSEGV:
			fprintf(stderr, "error: Segmentation fault at address %p. This is often caused by a stack overflow (infinite recursion) or an invalid memory access. Try limiting recursion or inspecting stack allocations.\n", addr);
			break;
		case SIGBUS:
			fprintf(stderr, "error: Bus error (SIGBUS) at address %p: possible unaligned or invalid memory access.\n", addr);
			break;
		case SIGFPE:
			fprintf(stderr, "error: Floating point exception (SIGFPE): arithmetic error (e.g., division by zero or overflow).\n");
			break;
		default:
			fprintf(stderr, "error: Unhandled signal %d received.\n", signum);
	}

	_exit(128 + signum);
}

static char* compiler_read_file_source(const char* file_path) {
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
	int EMIT_BINARY = 1;
	global_library_registry = init_library_registry("lib");
	if (!global_library_registry) {
		fprintf(stderr, "Failed to initialize library registry\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
			VERBOSE = 1;
		}
		if (strcmp(argv[i], "--emit-assembly") == 0 || strcmp(argv[i], "--no-emit-binary") == 0) {
			EMIT_BINARY = 0;
		}
		if (strcmp(argv[i], "--emit-binary") == 0 || strcmp(argv[i], "-b") == 0) {
			EMIT_BINARY = 1;
		}
	}

	create_lexer_tests();
	create_parser_tests();
	create_codegen_tests();

	signal(SIGALRM, handle_timeout);
	{
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_sigaction = runtime_signal_handler;
		sa.sa_flags = SA_SIGINFO;
		sigaction(SIGSEGV, &sa, NULL);
		sigaction(SIGBUS, &sa, NULL);
		sigaction(SIGFPE, &sa, NULL);
	}
	alarm(10);

	int src_index = -1;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') { src_index = i; break; }
	}
	if (src_index < 0) {
		fprintf(stderr, "No input file provided\n");
		return 1;
	}

	char* file_source = compiler_read_file_source(argv[src_index]);
	if (!file_source) {
		fprintf(stderr, "Failed to read source file: %s\n", argv[src_index]);
		fprintf(stderr, "Please check that the file exists and is readable\n");
		return 1;
	}

	Lexer* lexer = create_lexer(file_source);
	if (!lexer) {
		free(file_source);
		fprintf(stderr, "Failed to create lexer\n");
		return 1;
	}

	Parser parser;
	init_parser(&parser, lexer);

	if (parser.error) {
		if (parser.error_message) fprintf(stderr, "%s\n", parser.error_message);
		free_parser(&parser);
		free(lexer);
		free(file_source);
		return 1;
	}

	ASTNode* ast = parse_file(&parser);
	if (!ast || parser.error) {
		if (parser.error_message) fprintf(stderr, "%s\n", parser.error_message);
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
		fprintf(stderr, "Failed to create symbol table\n");
		return 1;
	}


	analyze_file(ast, symbols);
	check_entry_point(symbols);
	analyze_variable_usage(symbols);

	if (semantic_get_error_count() > 0) {
		fprintf(stderr, "Compilation failed: %d semantic errors found\n", semantic_get_error_count());
		free_symbol_table(symbols);
		free_ast(ast);
		free_parser(&parser);
		free(lexer);
		free(file_source);
		free_library_registry(global_library_registry);
		return 1;
	}

	#ifdef __aarch64__
		#define ARCH_ARM64 1
	#elif defined(__x86_64__) || defined(_M_X64)
		#define ARCH_X86_64 1
	#else
		#define ARCH_X86_64 1  // Default to x86_64
	#endif
	
	#ifdef ARCH_ARM64
		char* register_names[] = {"x19", "x20", "x21", "x22"};
	#else
		char* register_names[] = {"rbx", "r10", "r11", "r12"};
	#endif
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
		if (child) {
			if (child->type == AST_DECLARATION) {
				generate_ir(child);
			} else if (child->type == AST_PROGRAM && child->child_count > 3) {
				generate_ir(child);
			}
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
		#ifdef __APPLE__
			fprintf(asm_file, ".section __TEXT,__cstring,cstring_literals\n");
		#else
			fprintf(asm_file, ".section .rodata\n");
		#endif
		StringLiteral* current = strings;
		while (current) {
			if (current->value && current->label) {
				char* esc = escape_for_asm(current->value);
				if (esc) {
					fprintf(asm_file, "%s:\n", current->label);
					fprintf(asm_file, "  .string \"%s\"\n", esc);
					free(esc);
				}
			}
			current = current->next;
		}
		fprintf(asm_file, "\n");
	}
	
	fprintf(asm_file, ".text\n");
	
	GlobalVariable* gvars = get_global_variables();
	if (gvars) {
		#ifdef __APPLE__
			fprintf(asm_file, ".data\n");
		#else
			fprintf(asm_file, ".section .data\n");
		#endif
		GlobalVariable* cur = gvars;
		while (cur) {
			if (cur->is_string && cur->initial) {
				fprintf(asm_file, "%s: \n", cur->label);
				fprintf(asm_file, "    .quad %s\n", cur->initial);
			} else {
				const char* val = cur->initial ? cur->initial : "0";
				fprintf(asm_file, "%s: \n", cur->label);
				fprintf(asm_file, "    .quad %s\n", val);
			}
			cur = cur->next;
		}
		fprintf(asm_file, "\n");
	}

	IRInstruction* all_ir = get_ir_head();
	if (all_ir) {
		number_instructions(all_ir);
		LiveInterval* intervals = compute_liveness(all_ir);
		assign_stack_offsets(intervals, &config);

		if (VERBOSE) {
			print_ir();
			print_liveness(intervals);
			LiveInterval* it = intervals;
			fprintf(stderr, "Stack layout:\n");
			while (it) {
				fprintf(stderr, "  %s -> reg=%d spilled=%d offset=%d\n", it->variable_name, it->registry, it->spilled, it->stack_offset);
				it = it->next;
			}
		}

		int frame_size = compute_spill_frame_size(intervals, &config);
		#ifdef __APPLE__
			fprintf(asm_file, ".globl _main\n");
		#else
			fprintf(asm_file, ".globl main\n");
		#endif
		int stack_bytes = frame_size;
		if (stack_bytes < 0) stack_bytes = 0;
		stack_bytes = (stack_bytes + 15) & ~15;
		generate_asm(all_ir, intervals, &config, asm_file, stack_bytes);
	}
	
	fclose(asm_file);
	fprintf(stderr, "Assembly written to compiled/assembled.s\n");
	free_symbol_table(symbols);
	if (VERBOSE) {
		print_ast(ast, NODE_ACTUAL, 0);
	}

	free_ast(ast);
	free_parser(&parser);
	free(lexer);
	free(file_source);
	free_library_registry(global_library_registry);

	if (EMIT_BINARY) {
		fprintf(stderr, "Assembling and linking...\n");
		int status = 1;
		char arch[64] = "";
		{
			#include <sys/utsname.h>
			struct utsname u;
			if (uname(&u) == 0) strncpy(arch, u.machine, sizeof(arch)-1);
		}

		if (strstr(arch, "arm64") || strstr(arch, "aarch64")) {
			status = system("clang -I include -arch x86_64 compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1") ;
			if (status != 0) {
				status = system("gcc -I include -m64 -no-pie compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1");
			}
		} else {
			status = system("gcc -I include -no-pie compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1");
			if (status != 0) {
				status = system("clang -I include compiled/assembled.s lib/adan/*.c -o compiled/program 2>&1");
			}
		}

		if (status != 0) {
			fprintf(stderr, "error: failed to assemble/link compiled/assembled.s into compiled/program\n");
			return 1;
		}

		// I removed this as I tested running program.bin without the wrapper script on Apple Silicon.
		// It should be noted I have modified my Mac quite a bit and I may have things like custom Rosetta that my make it diffrent on my mac.
		// Someone with a stock Apple Silicon mac should test this.

		// Also just creating arm code is the best thing to do although focusing on or architecture first makes sense for now.

		
		// if (strstr(arch, "arm") || strstr(arch, "aarch64")) {
		// 	remove("compiled/program.bin");
		// 	if (rename("compiled/program", "compiled/program.bin") != 0) {
		// 		perror("warning: failed to rename compiled/program");
		// 	} else {
		// 		FILE* w = fopen("compiled/program", "w");
		// 		if (w) {
		// 			fprintf(w, "#!/bin/sh\nif command -v arch >/dev/null 2>&1; then\n  exec arch -x86_64 \"$(dirname \"$0\")/program.bin\" \"$@\"\nelse\n  echo \"error: this program is x86_64; on Apple Silicon run with Rosetta: arch -x86_64 ./compiled/program.bin\" >&2\n  exit 1\nfi\n");
		// 			fclose(w);
		// 			chmod("compiled/program", 0755);
		// 		}
		// 	}
		// }

		fprintf(stderr, "Compilation successful: compiled/program\n");
	} else {
		fprintf(stderr, "Assembly-only mode: compiled/assembled.s (not linking to binary)\n");
	}

	system("clear");
	return 0;
}


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
#include "flags.h"

static char *escape_for_asm(const char *s)
{
	if (!s)
		return strdup("");
	int cap = 64, len = 0;
	char *out = malloc(cap);
	if (!out)
		return NULL;
	for (int i = 0; s[i] != '\0'; i++)
	{
		char c = s[i];
		const char *esc = NULL;
		char tmp[3] = {0};
		switch (c)
		{
		case '\n':
			esc = "\\n";
			break;
		case '\t':
			esc = "\\t";
			break;
		case '\r':
			esc = "\\r";
			break;
		case '"':
			esc = "\\\"";
			break;
		case '\\':
			esc = "\\\\";
			break;
		default:
			tmp[0] = c;
			esc = tmp;
			break;
		}
		int add = strlen(esc);
		if (len + add + 1 >= cap)
		{
			cap *= 2;
			char *tmp2 = realloc(out, cap);
			if (!tmp2)
			{
				free(out);
				return NULL;
			}
			out = tmp2;
		}
		for (int j = 0; j < add; j++)
			out[len++] = esc[j];
	}
	out[len] = '\0';
	return out;
}

LibraryRegistry *global_library_registry = NULL;

static void handle_timeout(int signum)
{
	(void)signum;
	fprintf(stderr, "error: compilation timed out\n");
	exit(1);
}

static void runtime_signal_handler(int signum, siginfo_t *info, void *ctx)
{
	(void)ctx;
	void *addr = info ? info->si_addr : NULL;

	switch (signum)
	{
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

static char *compiler_read_file_source(const char *file_path)
{
	FILE *fp = fopen(file_path, "rb");
	if (!fp)
		return NULL;

	unsigned char bom[3];
	size_t n = fread(bom, 1, 3, fp);
	if (!(n == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF))
	{
		fseek(fp, 0, SEEK_SET);
	}

	size_t cap = 1024, len = 0;
	char *out = malloc(cap);
	if (!out)
	{
		fclose(fp);

		return NULL;
	}

	char buf[256];
	while (fgets(buf, sizeof(buf), fp))
	{
		size_t blen = strlen(buf);
		if (len + blen + 1 > cap)
		{
			cap *= 2;
			char *tmp = realloc(out, cap);

			if (!tmp)
			{
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

bool handleSpecialFlags(compiler_flags *flags)
{
	if (flags->help)
	{
		printf("Usage: adan-compile [options] <input_file>\n");
		printf("Options:\n  -h, --help              Show this help message\n  -v, --verbose           Enable verbose output\n  -k, --keep-asm          Keep intermediate ASM\n  -w, --warnings-as-errors Treat warnings as errors\n  -o, --output <file>     Specify output file\n  -i, --input <file>      Specify input file\n  -c, --compile-time      Enable compile-time evaluation\n  -e, --execute-after-run  Run after compilation\n");

		return true;
	}

out:
	return false;
}

int main(int argc, char **argv)
{
	int res = 0;

	compiler_flags *flags = NULL;
	flags = flags_init();

	if (flags == NULL)
	{
		fprintf(stderr, "Error: Failed to initialize compiler flags\n");
		res = -1;
		goto out;
	}

	if (parse_flags(argc, argv, flags) != 0)
	{
		res = -1;
		goto out;
	}

	if (handleSpecialFlags(flags))
		goto out;

	global_library_registry = init_library_registry("lib");
	if (!global_library_registry)
	{
		fprintf(stderr, "Failed to initialize library registry\n");
		return 1;
	}

	create_lexer_tests(flags);
	create_parser_tests();
	create_codegen_tests();

	signal(SIGALRM, handle_timeout);
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = runtime_signal_handler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	sigaction(SIGFPE, &sa, NULL);
	alarm(10);

	if (!flags->input || flags->input[0] == '\0')
	{
		fprintf(stderr, "No input file provided\n");
		res = -1;
		goto out;
	}

	char *file_source = compiler_read_file_source(flags->input);
	if (!file_source)
	{
		fprintf(stderr, "Failed to read source file: %s\n", flags->input);
		res = -1;
		goto out;
	}

	Lexer *lexer = create_lexer(file_source);
	if (!lexer)
	{
		fprintf(stderr, "Failed to create lexer\n");
		res = -1;
		goto out;
	}

	Parser parser;
	init_parser(&parser, lexer);
	if (parser.error)
	{
		if (parser.error_message)
			fprintf(stderr, "%s\n", parser.error_message);
		res = -1;
		goto out;
	}

	ASTNode *ast = parse_file(&parser);
	if (!ast || parser.error)
	{
		if (parser.error_message)
			fprintf(stderr, "%s\n", parser.error_message);
		res = -1;
		goto out;
	}

	SymbolTable *symbols = init_symbol_table();
	if (!symbols)
	{
		fprintf(stderr, "Failed to create symbol table\n");
		res = -1;
		goto out;
	}

	analyze_file(ast, symbols);
	check_entry_point(symbols);
	analyze_variable_usage(symbols);

	if (semantic_get_error_count() > 0)
	{
		fprintf(stderr, "Compilation failed: %d semantic errors\n", semantic_get_error_count());
		res = -1;
		goto out;
	}

#ifdef __aarch64__
	char *register_names[] = {"x19", "x20", "x21", "x22"};
#else
	char *register_names[] = {"rbx", "r10", "r11", "r12"};
#endif
	int caller_saved[] = {0, 1};
	TargetConfig config;
	init_target_config(&config, 4, register_names, 2, caller_saved, 8);

	FILE *asm_file = fopen("compiled/assembled.s", "w");
	if (!asm_file)
	{
		fprintf(stderr, "Failed to open ASM output\n");
		res = -1;
		goto out;
	}

	init_ir_full();
	for (int i = 0; i < ast->child_count; i++)
	{
		ASTNode *child = ast->children[i];
		if (!child)
			continue;
		if (child->type == AST_DECLARATION || (child->type == AST_PROGRAM && child->child_count > 3))
			generate_ir(child);
	}

	if (global_library_registry)
	{
		Library *lib = global_library_registry->libraries;
		while (lib)
		{
			if (lib->ast && lib->ast->child_count > 0)
			{
				for (int i = 0; i < lib->ast->child_count; i++)
				{
					ASTNode *func_node = lib->ast->children[i];
					if (func_node && func_node->type == AST_PROGRAM && func_node->child_count > 3)
						generate_ir(func_node);
				}
			}
			lib = lib->next;
		}
	}

	StringLiteral *strings = get_string_literals();
	if (strings)
	{
#ifdef __APPLE__
		fprintf(asm_file, ".section __TEXT,__cstring,cstring_literals\n");
#else
		fprintf(asm_file, ".section .rodata\n");
#endif
		for (StringLiteral *cur = strings; cur; cur = cur->next)
		{
			if (!cur->value || !cur->label)
				continue;
			char *esc = escape_for_asm(cur->value);
			if (esc)
			{
				fprintf(asm_file, "%s:\n  .string \"%s\"\n", cur->label, esc);
				free(esc);
			}
		}
	}

	fprintf(asm_file, ".text\n");

	GlobalVariable *gvars = get_global_variables();
	if (gvars)
	{
#ifdef __APPLE__
		fprintf(asm_file, ".data\n");
#else
		fprintf(asm_file, ".section .data\n");
#endif
		for (GlobalVariable *cur = gvars; cur; cur = cur->next)
		{
			const char *val = cur->initial ? cur->initial : "0";
			if (cur->is_string && cur->initial)
			{
				fprintf(asm_file, "%s:\n    .quad %s\n", cur->label, cur->initial);
			}
			else
			{
				fprintf(asm_file, "%s:\n    .quad %s\n", cur->label, val);
			}
		}
	}

	IRInstruction *all_ir = get_ir_head();
	if (all_ir)
	{
		number_instructions(all_ir);
		LiveInterval *intervals = compute_liveness(all_ir);
		assign_stack_offsets(intervals, &config);
		int frame_size = compute_spill_frame_size(intervals, &config);
		frame_size = (frame_size + 15) & ~15;
#ifdef __APPLE__
		fprintf(asm_file, ".globl _main\n");
#else
		fprintf(asm_file, ".globl main\n");
#endif
		generate_asm(all_ir, intervals, &config, asm_file, frame_size);
	}

	fclose(asm_file);
	fprintf(stderr, "Assembly written to compiled/assembled.s\n");

	fprintf(stderr, "Assembling and linking...\n");
	int status = 1;
	char arch[64] = "";
	{
#include <sys/utsname.h>
		struct utsname u;
		if (uname(&u) == 0)
			strncpy(arch, u.machine, sizeof(arch) - 1);
	}
	char cmd[512];

	if (strstr(arch, "arm64") || strstr(arch, "aarch64"))
	{
		snprintf(cmd, sizeof(cmd),
				 "clang -I include -arch x86_64 compiled/assembled.s lib/adan/*.c -lm -o %s 2>&1",
				 flags->output);
		status = system(cmd);
		if (status != 0)
		{
			snprintf(cmd, sizeof(cmd),
					 "gcc -I include -m64 -no-pie compiled/assembled.s lib/adan/*.c -lm -o %s 2>&1",
					 flags->output);
			status = system(cmd);
		}
	}
	else
	{
		snprintf(cmd, sizeof(cmd),
				 "gcc -I include -no-pie compiled/assembled.s lib/adan/*.c -lm -o %s 2>&1",
				 flags->output);
		status = system(cmd);
		if (status != 0)
		{
			snprintf(cmd, sizeof(cmd),
					 "clang -I include compiled/assembled.s lib/adan/*.c -lm -o %s 2>&1",
					 flags->output);
			status = system(cmd);
		}
	}

	if (status != 0)
	{
		fprintf(stderr, "error: failed to assemble/link compiled/assembled.s into %s\n", flags->output);
		return 1;
	}

	fprintf(stderr, "Compilation successful: %s\n", flags->output);

out:
	if (file_source)
		free(file_source);
	if (lexer)
		free_lexer(lexer);
	if (ast)
		free_ast(ast);
	if (symbols)
		free_symbol_table(symbols);
	if (global_library_registry)
		free_library_registry(global_library_registry);

	if (flags)
	{
		if (flags->input)
		{
			free(flags->input);
			flags->input = NULL;
		}
		if (flags->output)
		{
			free(flags->output);
			flags->output = NULL;
		}
		free(flags);
	}

	system("cls || clear");

	return res;
}
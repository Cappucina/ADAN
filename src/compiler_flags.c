#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler_flags.h"

/*
 * Compiler Flags Implementation
 * 
 * Handles parsing and management of command-line compilation flags
 */

// ============================================================================
// INITIALIZATION
// ============================================================================

CompilerFlags* flags_init(void) {
	CompilerFlags* flags = malloc(sizeof(CompilerFlags));
	if (flags == NULL) {
		fprintf(stderr, "Error: Failed to allocate memory for flags\n");
		return NULL;
	}
	// Initialize all fields to defaults
	flags->target_arch = TARGET_UNKNOWN;
	flags->target_os = OS_UNKNOWN;
	flags->host_arch = TARGET_UNKNOWN;
	flags->host_os = OS_UNKNOWN;
	flags->input_file = NULL;
	flags->output_file = NULL;
	flags->opt_level = OPT_NONE;
	flags->emit_ir = 0;
	flags->emit_asm = 0;
	flags->verbose = 0;
	flags->debug = 0;
	flags->is_cross_compile = 0;
	flags->target_triple = NULL;
	flags->no_link = 0;
	flags->static_link = 0;
	flags->position_independent = 0;
	flags->show_help = 0;
	flags->show_version = 0;
    flags->test_compiler = 0;

	// Try to set sensible defaults based on host system
#ifdef __APPLE__
# ifdef __aarch64__
	flags->target_arch = TARGET_ARM64;
# else
	flags->target_arch = TARGET_X86_64;
# endif
	flags->target_os = OS_MACOS;
	flags->host_arch = flags->target_arch;
	flags->host_os = flags->target_os;
#elif __linux__
	flags->target_os = OS_LINUX;
# ifdef __aarch64__
	flags->target_arch = TARGET_AARCH64;
# else
	flags->target_arch = TARGET_X86_64;
# endif
	flags->host_arch = flags->target_arch;
	flags->host_os = flags->target_os;
#elif _WIN32
	flags->target_os = OS_WINDOWS;
	flags->target_arch = TARGET_X86_64;
	flags->host_arch = flags->target_arch;
	flags->host_os = flags->target_os;
#else
	flags->target_arch = TARGET_UNKNOWN;
	flags->target_os = OS_UNKNOWN;
#endif

	return flags;
}

// ============================================================================
// FLAG PARSING
// ============================================================================

int flags_parse(int argc, char** argv, CompilerFlags* flags) {
	// If no arguments supplied, keep defaults and return success.
	if (argc < 2) {
		return 0;
	}
	
	// Simple parsing loop for common options. This is intentionally
	// lightweight so the compiler can run with no args during tests.
	for (int i = 1; i < argc; ++i) {
		const char* a = argv[i];

		if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0 || argc == 1) {
			flags->show_help = 1;
			return 0;
		}

		if (strcmp(a, "--version") == 0) {
			flags->show_version = 1;
			return 0;
		}

		if (strcmp(a, "-v") == 0 || strcmp(a, "--verbose") == 0) {
			flags->verbose = 1;
			continue;
		}

		if (strcmp(a, "-g") == 0 || strcmp(a, "--debug") == 0) {
			flags->debug = 1;
			continue;
		}

		if (strncmp(a, "--target=", 9) == 0) {
			const char* val = a + 9;
			flags->target_arch = string_to_arch(val);
			continue;
		}

		if (strncmp(a, "--os=", 5) == 0) {
			const char* val = a + 5;
			flags->target_os = string_to_os(val);
			continue;
		}

		if (strncmp(a, "--target-triple=", 16) == 0) {
			const char* val = a + 16;
			flags->target_triple = strdup(val);
			flags->is_cross_compile = 1;
			continue;
		}

		if (strcmp(a, "--emit-ir") == 0) {
			flags->emit_ir = 1;
			continue;
		}

		if (strcmp(a, "--emit-asm") == 0) {
			flags->emit_asm = 1;
			continue;
		}

		if (strcmp(a, "-o") == 0 && i + 1 < argc) {
			flags->output_file = strdup(argv[++i]);
			continue;
		}

        if (strcmp(a, "-t") == 0 || strcmp(a, "--test") == 0 || strcmp(a, "--tests") == 0) {
            flags->test_compiler = 1;
            continue;
        }

		// positional argument: input file
		if (flags->input_file == NULL) {
			flags->input_file = strdup(a);
			continue;
		}

	// If the user did not explicitly set a target arch/os, use host values
	if (flags->target_arch == TARGET_UNKNOWN) flags->target_arch = flags->host_arch;
	if (flags->target_os == OS_UNKNOWN) flags->target_os = flags->host_os;

	// Mark cross-compile if target differs from detected host
	if (flags->target_arch != flags->host_arch || flags->target_os != flags->host_os) {
		flags->is_cross_compile = 1;
	}

	// If no explicit target triple, generate a sensible default
	if (flags->target_triple == NULL) {
		char* triple = get_target_triple(flags);
		if (triple) {
			flags->target_triple = triple; // ownership transferred
		}
	}

	return 0;
		fprintf(stderr, "Unknown argument: %s\n", a);
		flags_print_help(argv[0]);
		return -1;
	}
	//   --target=<arch>        or -t <arch>       (arm64, x86-64, aarch64, wasm)
	//   --os=<os>              or -o <os>         (macos, linux, windows, web)
	//   --optimize=<level>     or -O<level>       (0, s, 2, 3)
	//   --output=<file>        or -o <file>       (output filename)
	//   --emit-ir                                 (output IR only)
	//   --emit-asm                                (output assembly, don't assemble)
	//   --verbose              or -v              (verbose output)
	//   --debug                or -g              (include debug symbols)
	//   --no-link                                 (compile but don't link)
	//   --static                                  (static linking)
	//   --pic                  or -fPIC           (position-independent code)
	//   --help                 or -h              (show help)
	//   --version                                 (show version)
	//   --target-triple=<triple>                  (explicit target triple)
	return 0;
}

// ============================================================================
// VALIDATION
// ============================================================================

int flags_validate(CompilerFlags* flags) {
	// TODO: Check that input_file is set
	// TODO: Check that target_arch is not UNKNOWN
	// TODO: Check that target_os is not UNKNOWN
	// TODO: Ensure output_file is set (or generate default)
	// TODO: Warn if cross-compiling without explicit --target-triple
	// TODO: Return 0 if valid, -1 if invalid
	
	return 0;
}

// ============================================================================
// ENUM CONVERSIONS
// ============================================================================

const char* arch_to_string(TargetArch arch) {
	switch (arch) {
		case TARGET_ARM64: return "arm64";
		case TARGET_X86_64: return "x86-64";
		case TARGET_AARCH64: return "aarch64";
		case TARGET_WASM: return "wasm";
		default: return "unknown";
	}
}

const char* os_to_string(TargetOS os) {
	switch (os) {
		case OS_MACOS: return "macos";
		case OS_LINUX: return "linux";
		case OS_WINDOWS: return "windows";
		case OS_WEB: return "web";
		default: return "unknown";
	}
}

TargetArch string_to_arch(const char* str) {
	if (str == NULL) return TARGET_UNKNOWN;
	if (strcmp(str, "arm64") == 0 || strcmp(str, "aarch64") == 0) return TARGET_ARM64;
	if (strcmp(str, "x86-64") == 0 || strcmp(str, "x86_64") == 0 || strcmp(str, "x86") == 0) return TARGET_X86_64;
	if (strcmp(str, "aarch64") == 0) return TARGET_AARCH64;
	if (strcmp(str, "wasm") == 0 || strcmp(str, "wasm32") == 0) return TARGET_WASM;
	return TARGET_UNKNOWN;
}

TargetOS string_to_os(const char* str) {
	if (str == NULL) return OS_UNKNOWN;
	if (strcmp(str, "macos") == 0 || strcmp(str, "darwin") == 0) return OS_MACOS;
	if (strcmp(str, "linux") == 0) return OS_LINUX;
	if (strcmp(str, "windows") == 0 || strcmp(str, "win32") == 0) return OS_WINDOWS;
	if (strcmp(str, "web") == 0 || strcmp(str, "wasm") == 0) return OS_WEB;
	return OS_UNKNOWN;
}

// ============================================================================
// TARGET TRIPLE GENERATION
// ============================================================================

char* get_target_triple(CompilerFlags* flags) {
	if (flags == NULL) return NULL;
	if (flags->target_triple) return strdup(flags->target_triple);

	const char* arch_part = "";
	const char* vendor = "unknown";
	const char* os_part = "unknown";

	switch (flags->target_arch) {
		case TARGET_ARM64:
		case TARGET_AARCH64:
			arch_part = "aarch64";
			break;
		case TARGET_X86_64:
			arch_part = "x86_64";
			break;
		case TARGET_WASM:
			arch_part = "wasm32";
			break;
		default:
			arch_part = "unknown";
	}

	switch (flags->target_os) {
		case OS_MACOS:
			vendor = "apple";
			os_part = "darwin";
			break;
		case OS_LINUX:
			vendor = "unknown";
			os_part = "linux-gnu";
			break;
		case OS_WINDOWS:
			vendor = "pc";
			os_part = "windows-msvc";
			break;
		case OS_WEB:
			vendor = "unknown";
			os_part = "unknown";
			break;
		default:
			vendor = "unknown";
			os_part = "unknown";
	}

	size_t len = strlen(arch_part) + 1 + strlen(vendor) + 1 + strlen(os_part) + 1;
	char* triple = malloc(len);
	if (!triple) return NULL;
	snprintf(triple, len, "%s-%s-%s", arch_part, vendor, os_part);
	return triple;
}

// ============================================================================
// HELP AND VERSION
// ============================================================================

void flags_print_help(const char* program_name) {
	printf("Usage: %s [OPTIONS] <input-file>\n\n", program_name);
	printf("OPTIONS:\n");
	printf("  Target Configuration:\n");
	printf("    --target=<arch>         Target architecture (arm64, x86-64, aarch64, wasm)\n");
	printf("    --os=<os>               Target OS (macos, linux, windows, web)\n");
	printf("    --target-triple=<triple> Explicit target triple\n\n");
	printf("  Compilation:\n");
	printf("    -O0, -Os, -O2, -O3      Optimization level\n");
	printf("    --emit-ir               Emit intermediate representation only\n");
	printf("    --emit-asm              Emit assembly code (don't assemble)\n");
	printf("    -o <file>               Output file name\n\n");
	printf("  Linking:\n");
	printf("    --no-link               Compile but don't link\n");
	printf("    --static                Static linking\n");
	printf("    --pic                   Position-independent code\n\n");
	printf("  Debugging:\n");
	printf("    -g, --debug             Include debug symbols\n");
	printf("    -v, --verbose           Verbose output\n\n");
	printf("  Information:\n");
	printf("    -h, --help              Show this help message\n");
	printf("    --version               Show version information\n");
	printf("    -t, --test              Run compiler tests\n");
}

void flags_print_version(void) {
	printf("ADAN Compiler v0.X.X\n"); // TODO: For someone who manages versioning to update with current version or make a automated system
	printf("Written in C with love\n");
}

// ============================================================================
// CLEANUP
// ============================================================================

void flags_free(CompilerFlags* flags) {
	if (flags == NULL) return;
	
	if (flags->input_file) free(flags->input_file);
	if (flags->output_file) free(flags->output_file);
	if (flags->target_triple) free(flags->target_triple);
	
	free(flags);
}

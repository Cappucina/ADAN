#ifndef COMPILER_FLAGS_H
#define COMPILER_FLAGS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Compiler Configuration Flags
 * 
 * This module handles command-line flags for code generation targets,
 * optimization levels, and debugging options.
 */

// ============================================================================
// TARGET ARCHITECTURE ENUMERATION
// ============================================================================

typedef enum {
	TARGET_ARM64,      // Apple Silicon M1, M2, M3, etc. As well as other devices like Raspberry Pi, etc. with manufactures like Qualcomm, etc.
	TARGET_X86_64,     // Intel/AMD 64-bit
	TARGET_WASM,       // WebAssembly (future)
	TARGET_UNKNOWN
} TargetArch;

// ============================================================================
// TARGET OS ENUMERATION
// ============================================================================

typedef enum {
	OS_MACOS,
	OS_LINUX,
	OS_WINDOWS,
	OS_WEB,
	OS_UNKNOWN
} TargetOS;

// ============================================================================
// OPTIMIZATION LEVELS
// ============================================================================

typedef enum {
	OPT_NONE = 0,  // -O0: No optimization
	OPT_SIZE = 1,  // -Os: Optimize for code size
	OPT_SPEED = 2, // -O2: Optimize for speed
	OPT_MAX = 3    // -O3: Maximum optimization
} OptimizationLevel;

// ============================================================================
// COMPILER FLAGS STRUCTURE
// ============================================================================

typedef struct {
	// Target Configuration
	TargetArch target_arch;
	TargetOS target_os;
	// Host (detected) configuration
	TargetArch host_arch;
	TargetOS host_os;
	
	// Input/Output
	char* input_file;
	char* output_file;
	
	// Code Generation Options
	OptimizationLevel opt_level;
	int emit_ir;              // Emit IR intermediate representation
	int emit_asm;             // Emit assembly code (don't assemble)
	int verbose;              // Print detailed compilation messages
	int debug;                // Include debug symbols
	
	// Cross-Compilation
	int is_cross_compile;     // Flag if cross-compiling
	char* target_triple;      // Target triple (e.g., "aarch64-linux-gnu")
	
	// Linking
	int no_link;              // Don't link, just compile
	int static_link;          // Static linking
	int position_independent; // Generate position-independent code
	
	// Misc
	int show_help;
	int show_version;
    int test_compiler;
    int show_system_info;
} CompilerFlags;

// ============================================================================
// FLAG PARSING FUNCTIONS
// ============================================================================

/**
 * Initialize default compiler flags
 */
CompilerFlags* flags_init(void);

/**
 * Parse command-line arguments and set flags
 * Returns 0 on success, -1 on error
 */
int flags_parse(int argc, char** argv, CompilerFlags* flags);

/**
 * Print usage/help message
 */
void flags_print_help(const char* program_name);

/**
 * Print version information
 */
void flags_print_version(void);

/**
 * Validate flags for consistency
 * Returns 0 if valid, -1 if invalid
 */
int flags_validate(CompilerFlags* flags);

/**
 * Convert TargetArch enum to string
 */
const char* arch_to_string(TargetArch arch);

/**
 * Convert TargetOS enum to string
 */
const char* os_to_string(TargetOS os);

/**
 * Convert string to TargetArch enum
 */
TargetArch string_to_arch(const char* str);

/**
 * Convert string to TargetOS enum
 */
TargetOS string_to_os(const char* str);

/**
 * Get the target triple string (e.g., "aarch64-apple-darwin")
 */
char* get_target_triple(CompilerFlags* flags);

/**
 * Show system information based on flags
 */
void show_system_info(CompilerFlags* flags);

/**
 * Free allocated memory in flags
 */
void flags_free(CompilerFlags* flags);

#endif // COMPILER_FLAGS_H

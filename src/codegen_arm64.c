#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen_arm64.h"

/*
 * ARM64 Code Generation Implementation for M-series Macs
 * 
 * This file contains stub implementations for ARM64 code generation.
 * You will fill in the logic to convert IR instructions to ARM64 assembly.
 */

// ============================================================================
// SECTION 1: REGISTER MANAGEMENT STUBS
// ============================================================================

ARM64RegAllocator* arm64_init_allocator(void) {
	// TODO: Allocate and initialize all ARM64 registers (x0-x31)
	// TODO: Mark caller-saved registers (x0-x17, x30)
	// TODO: Mark callee-saved registers (x19-x28)
	// TODO: Return the initialized allocator
	return NULL;
}

char* arm64_allocate_register(ARM64RegAllocator* alloc) {
	// TODO: Find the next available register
	// TODO: Mark it as unavailable
	// TODO: Return its name (e.g., "x0", "x1")
	// TODO: If no registers available, implement register spilling
	return NULL;
}

void arm64_free_register(ARM64RegAllocator* alloc, char* reg_name) {
	// TODO: Find the register by name
	// TODO: Mark it as available again
}

// ============================================================================
// SECTION 2: INSTRUCTION EMISSION STUBS
// ============================================================================

void arm64_emit_mov(FILE* output, char* dest, char* src) {
	// TODO: Emit: mov dest, src
	// Example format: fprintf(output, "\tmov %s, %s\n", dest, src);
}

void arm64_emit_add(FILE* output, char* dest, char* src1, char* src2) {
	// TODO: Emit: add dest, src1, src2
}

void arm64_emit_sub(FILE* output, char* dest, char* src1, char* src2) {
	// TODO: Emit: sub dest, src1, src2
}

void arm64_emit_mul(FILE* output, char* dest, char* src1, char* src2) {
	// TODO: Emit: mul dest, src1, src2
}

void arm64_emit_div(FILE* output, char* dest, char* src1, char* src2) {
	// TODO: Emit: sdiv dest, src1, src2 (signed divide for integers)
}

void arm64_emit_cmp_and_branch(FILE* output, char* cond, char* label, char* arg1, char* arg2) {
	// TODO: Emit comparison instruction (cmp arg1, arg2)
	// TODO: Emit conditional branch (b.eq, b.ne, b.lt, etc. based on cond)
	// Example: fprintf(output, "\tcmp %s, %s\n\tb.%s %s\n", arg1, arg2, cond, label);
}

void arm64_emit_jmp(FILE* output, char* label) {
	// TODO: Emit unconditional jump: b label
}

void arm64_emit_label(FILE* output, char* label) {
	// TODO: Emit a label for jump targets
	// Format: label:
}

// ============================================================================
// SECTION 3: STACK MANAGEMENT STUBS
// ============================================================================

ARM64StackFrame* arm64_init_stack_frame(void) {
	// TODO: Allocate a new stack frame structure
	// TODO: Initialize current_offset to 0
	// TODO: Return the frame
	return NULL;
}

int arm64_allocate_stack_space(ARM64StackFrame* frame, int size) {
	// TODO: Calculate the offset for a new variable
	// TODO: Update current_offset
	// TODO: Return the offset (useful for accessing the variable later)
	return 0;
}

// ============================================================================
// SECTION 4: PROLOGUE/EPILOGUE STUBS
// ============================================================================

void arm64_emit_prologue(FILE* output, int stack_size) {
	// TODO: Emit prologue instructions:
	//   - Save callee-saved registers (stp x29, x30, [sp, #-16]!)
	//   - Adjust stack pointer if needed
	//   - Set up frame pointer
}

void arm64_emit_epilogue(FILE* output) {
	// TODO: Emit epilogue instructions:
	//   - Restore callee-saved registers
	//   - Return from function (ret)
}

// ============================================================================
// SECTION 5: MAIN CODE GENERATION FUNCTION
// ============================================================================

void arm64_generate_code(IRInstruction* ir_head, const char* output_filename) {
	// TODO: Open output file for writing
	
	// TODO: Emit assembly header (section declarations, etc.)
	
	// TODO: Iterate through IR instructions and emit ARM64 code:
	//   for each ir_head...next:
	//     if (ir->op == IR_ADD) emit_add(...)
	//     if (ir->op == IR_SUB) emit_sub(...)
	//     ... handle all IR operations
	
	// TODO: Emit assembly footer
	
	// TODO: Close output file
}

void arm64_cleanup(ARM64RegAllocator* alloc, ARM64StackFrame* frame) {
	// TODO: Free allocated memory for registers
	// TODO: Free allocated memory for stack frame
}

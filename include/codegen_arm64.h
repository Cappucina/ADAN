#ifndef CODEGEN_ARM64_H
#define CODEGEN_ARM64_H

#include "ir.h"
#include "codegen.h"

/*
 * ARM64 (Apple Silicon M-series) Code Generation Module
 * 
 * This module is responsible for translating IR instructions into ARM64 assembly
 * for Apple Silicon Macs (M1, M2, M3, etc.)
 */

// ============================================================================
// SECTION 1: REGISTER MANAGEMENT
// ============================================================================
// You'll need to:
// - Define ARM64 general-purpose registers (x0-x31)
// - Define caller-saved vs callee-saved registers
// - Track register allocation/deallocation
// - Handle register spilling (when we run out of registers)

typedef struct {
	char* name;           // Register name (e.g., "x0", "x1")
	int is_available;     // Whether this register is free to use
	int is_caller_saved;  // Whether caller must preserve this register
} ARM64Register;

typedef struct {
	ARM64Register* registers;
	int num_registers;
	int next_free_register;
} ARM64RegAllocator;

// Initialize the register allocator
ARM64RegAllocator* arm64_init_allocator(void);

// Request an available register for temporary storage
char* arm64_allocate_register(ARM64RegAllocator* alloc);

// Free a register when done using it
void arm64_free_register(ARM64RegAllocator* alloc, char* reg_name);

// ============================================================================
// SECTION 2: INSTRUCTION EMISSION
// ============================================================================
// You'll need to:
// - Convert each IR operation (IR_ADD, IR_MUL, etc.) to ARM64 instructions
// - Handle different addressing modes (immediate, register, memory)
// - Track offsets for stack memory

// Emit a basic MOV instruction (move data)
void arm64_emit_mov(FILE* output, char* dest, char* src);

// Emit an ADD instruction
void arm64_emit_add(FILE* output, char* dest, char* src1, char* src2);

// Emit a SUB instruction
void arm64_emit_sub(FILE* output, char* dest, char* src1, char* src2);

// Emit a MUL instruction
void arm64_emit_mul(FILE* output, char* dest, char* src1, char* src2);

// Emit a DIV instruction
void arm64_emit_div(FILE* output, char* dest, char* src1, char* src2);

// Emit a comparison and conditional jump
void arm64_emit_cmp_and_branch(FILE* output, char* cond, char* label, char* arg1, char* arg2);

// Emit an unconditional jump
void arm64_emit_jmp(FILE* output, char* label);

// Emit a label (jump target)
void arm64_emit_label(FILE* output, char* label);

// ============================================================================
// SECTION 3: STACK MANAGEMENT
// ============================================================================
// You'll need to:
// - Track the stack pointer (sp)
// - Allocate space for local variables
// - Handle function prologue/epilogue

typedef struct {
	int current_offset;  // Current stack offset from sp
	int max_offset;      // Maximum offset used (for prologue)
} ARM64StackFrame;

// Initialize stack frame for a function
ARM64StackFrame* arm64_init_stack_frame(void);

// Allocate space on the stack for a variable
int arm64_allocate_stack_space(ARM64StackFrame* frame, int size);

// ============================================================================
// SECTION 4: FUNCTION PROLOGUES AND EPILOGUES
// ============================================================================
// You'll need to:
// - Save/restore callee-saved registers (x19-x28)
// - Adjust stack pointer for local variables
// - Follow ARM64 ABI (Application Binary Interface)

// Emit function prologue (entry point setup)
void arm64_emit_prologue(FILE* output, int stack_size);

// Emit function epilogue (exit sequence)
void arm64_emit_epilogue(FILE* output);

// ============================================================================
// SECTION 5: MAIN CODE GENERATION FUNCTION
// ============================================================================

// Convert IR to ARM64 assembly
void arm64_generate_code(IRInstruction* ir_head, const char* output_filename);

// Free all allocated resources
void arm64_cleanup(ARM64RegAllocator* alloc, ARM64StackFrame* frame);

#endif // CODEGEN_ARM64_H

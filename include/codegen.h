#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "liveness.h"
#include "ir.h"
#include <stdbool.h>

// 
//  We're doing this simply because we can't just "ask"
//   how many registers are available on the CPU.
// 
typedef struct TargetConfig {
	int available_registers;      // Total available general-purpose registers.
	char** register_names;        // The names (e.g., "rax", "rbx")
	int* caller_saved_indices;    // Which registers are safe to remove?
	int caller_saved_count;       // Number of caller-saved entries.
	int spill_slot_size;          // Size (bytes) per spilled slot on stack.
} TargetConfig;

// 
//  Initialize target descriptor. `register_names` and `caller_saved_indices`
//   are copied (caller may free theirs after init). `spill_slot_size` in bytes.
// 
bool init_target_config(TargetConfig* cfg, int available_registers, char** register_names, int caller_saved_count, int* caller_saved_indices, int spill_slot_size);

// 
//  Release any resources owned by cfg.
// 
void free_target_config(TargetConfig* cfg);

// 
//  Return the textual register name for a given index, or NULL if out of range.
// 
const char* get_register_name(const TargetConfig* cfg, int index);

// 
//  High level entry: generate assembly from IR and the computed liveness/intervals.
//   `out` may be stdout or a FILE* you open.
// 
void get_location(char* result_buffer, char* variable_name, LiveInterval* intervals, const TargetConfig* cfg);
void generate_asm(IRInstruction* ir_head, LiveInterval* intervals, const TargetConfig* cfg, FILE* out, int stack_bytes);

// 
//  Emit function prologue/epilogue helpers (useful when assembling functions).
// 
void emit_prologue(FILE* out, const TargetConfig* cfg, int stack_bytes);
void emit_epilogue(FILE *out, const TargetConfig *cfg, int stack_bytes);

// 
//  Assign stack offsets (in bytes) to spilled intervals; uses cfg->spill_slot_size.
// 
void assign_stack_offsets(LiveInterval* intervals, const TargetConfig* cfg);

// 
//  A small helper to compute total spill stack frame size (returns bytes).
// 
int compute_spill_frame_size(LiveInterval* intervals, const TargetConfig* cfg);

// 
//  Debug helper to print config to provided FILE*.
// 
void print_target_config(const TargetConfig* cfg, FILE* out);

#endif
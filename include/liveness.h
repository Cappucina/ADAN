#ifndef LIVENESS_H
#define LIVENESS_H

#include "ir.h"
#include <stdbool.h>

typedef struct LiveInterval {
	char* variable_name;          // The variable name. (e.g., "_t0")
	int start_point;              // "Birth": Instruction index where defined.
	int end_point;                // "Death": Instruction index where last used.
	int registry;                 // The specific CPU register we assign. (e.g., 0 for RAX)
	bool spilled;                 // True if we run out of registers and put it on stack.
	int stack_offset;             // If spilled, where is it on the stack? (e.g., -8)
	struct LiveInterval* next;    // Linked list pointer.
} LiveInterval;

bool is_variable(char* name);

// 
//  Walks the IR list and numbers instructions. (0, 2, 4...)
// 
void number_instructions(IRInstruction* ir_head);

// 
//  Searches through and either returns or creates a new
//   interval.
// 
LiveInterval* get_or_return_interval(char* name, int current_index);

// 
//  Scans the IR, finds when variables are created and used,
//   and returns a list of intervals.
// 
LiveInterval* compute_liveness(IRInstruction* ir_head);

// 
//  Prints the table: "Variable | Start | End"
// 
void print_liveness(LiveInterval* interval_head);

// 
//  Search the existing list for a variable name. (to update its end_point)
// 
LiveInterval* find_interval(LiveInterval* head, char* name);

void sort_intervals(LiveInterval** head_reference);

#endif
#include "codegen.h"
#include <stdlib.h>

bool init_target_config(TargetConfig* cfg, int available_registers, char** register_names, int caller_saved_count, int* caller_saved_indices, int spill_slot_size) {
    cfg->available_registers = available_registers;
    cfg->register_names = register_names;
    cfg->caller_saved_indices = caller_saved_indices;
    cfg->caller_saved_count = caller_saved_count;
    cfg->spill_slot_size = spill_slot_size;
}

void free_target_config(TargetConfig* cfg) {
    if (cfg == NULL) return;
    if (cfg->register_names != NULL) {
        for (int i = 0; i < cfg->available_registers; i++) {
            if (cfg->register_names[i] != NULL) free(cfg->register_names[i]);
        }

        free(cfg->register_names);
    }

    if (cfg->caller_saved_indices != NULL) free(cfg->caller_saved_indices);
    free(cfg);
}

const char* get_register_name(const TargetConfig* cfg, int index) {
    
}

void generate_asm(IRInstruction* ir_head, LiveInterval* intervals, const TargetConfig* cfg, FILE* out) {
    
}

void emit_prologue(FILE* out, const TargetConfig* cfg, int stack_bytes) {

}

void emit_epilogue(FILE* out, const TargetConfig* cfg) {
    
}

void assign_stack_offsets(LiveInterval* intervals, const TargetConfig* cfg) {

}

int compute_spill_frame_size(LiveInterval* intervals, const TargetConfig* cfg) {
    
}

void print_target_config(const TargetConfig* cfg, FILE* out) {
    
}
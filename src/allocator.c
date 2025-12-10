#include "liveness.h"
#include "codegen.h"

#define REGISTRY_POOL_SIZE 4

// 
//  We map internal IDs (0-3) to physical registers
// 
//   0 -> rbx
//   1 -> r10
//   2 -> r11
//   3 -> r12
// 
//  We're using these specifically as they're just the
//   safest to mess around with and modify.
// 

void allocate_registers(LiveInterval* head_interval) {
	sort_intervals(&head_interval);

	bool registries_in_use[REGISTRY_POOL_SIZE];
	for(int i = 0; i < REGISTRY_POOL_SIZE; i++) {
		registries_in_use[i] = false;
	}

	LiveInterval* active_intervals[REGISTRY_POOL_SIZE];
	for (int i = 0; i < REGISTRY_POOL_SIZE; i++) {
		active_intervals[i] = NULL;
	}

	LiveInterval* current = head_interval;
	while (current != NULL) {
		// 
		//  Step 1:
		//        - Look at all the registers. If the variable holding one
		//           has "died", free the register.
		// 
		for (int i = 0; i < REGISTRY_POOL_SIZE; i++) {
			if (active_intervals[i] != NULL) {
				if (active_intervals[i]->end_point < current->start_point) {
					registries_in_use[i] = false;
					active_intervals[i] = NULL;
				}
			}
		}

		// 
		//  Step 2:
		//        - Attempt to allocate object to registry.
		// 
		int found_registry = -1;
		for (int i = 0; i < REGISTRY_POOL_SIZE; i++) {
			if (!registries_in_use[i]) {
				found_registry = i;
				break;
			}
		}

		// 
		//  Step 3:
		//        - Either assign or spill the data.
		// 
		if (found_registry != -1) {
			current->registry = found_registry;
			current->spilled = false;
			registries_in_use[found_registry] = true;
			active_intervals[found_registry] = current;

			// printf("Allocated %s to Registry %d\n", current->variable_name, found_registry);
		} else {
			// FAILURE: No registers left. We must SPILL!!

			current->registry = -1;
			current->spilled = true;

			// Calculation of the exact stack offset after spillage will
			//  be done during code generation.
		}

		current = current->next;
	}
}
#include "liveness.h"
#include "ir.h"
#include <string.h>
#include <ctype.h>

static LiveInterval* interval_list = NULL;

bool is_variable(char* name) {
	if (name == NULL) return false;
	if (isdigit(name[0])) return false;
	if (name[0] == '-' && isdigit(name[1])) return false;

	return true;
}

void number_instructions(IRInstruction* ir_head) {
	int counter = 0;
	IRInstruction* current = ir_head;
	
	while (current != NULL) {
		current->index = counter;
		counter += 2;
		current = current->next;
	}
}

LiveInterval* get_or_return_interval(char* name, int current_index) {
	LiveInterval* current = interval_list;

	while (current != NULL) {
		if (strcmp(current->variable_name, name) == 0) {
			if (current_index > current->end_point) {
				current->end_point = current_index;
			}
			return current;
		}
		current = current->next;
	}

	LiveInterval* new_node = malloc(sizeof(LiveInterval));
	
	new_node->variable_name = name;
	new_node->start_point = current_index;
	new_node->end_point = current_index;
	new_node->registry = -1;
	new_node->spilled = false;
	new_node->next = interval_list;

	interval_list = new_node;

	return new_node;
}

LiveInterval* compute_liveness(IRInstruction* ir_head) {
	interval_list = NULL;
	IRInstruction* current = ir_head;

	while (current != NULL) {
		if (is_variable(current->arg1)) get_or_return_interval(current->arg1, current->index);
		if (is_variable(current->arg2)) get_or_return_interval(current->arg2, current->index);
		if (is_variable(current->result)) get_or_return_interval(current->result, current->index);

		current = current->next;
	}

	return interval_list;
}

void print_liveness(LiveInterval* interval_head) {
	printf("%-10s | %-5s | %-5s\n", "Variable", "Start", "End");

	LiveInterval* current = interval_head;

	while (current != NULL) {
		printf("%-10s | %-5d | %-5d\n", current->variable_name, current->start_point, current->end_point);
		current = current->next; // Advance to avoid infinite loop when printing intervals
	}
}

LiveInterval* find_interval(LiveInterval* head, char* name) {
	LiveInterval* current = head;

	while (current != NULL) {
		if (strcmp(current->variable_name, name) == 0) {
			return current;
		}
		current = current->next;
	}

	return NULL;
}

void sort_intervals(LiveInterval** head_reference) {
	int swapped;

	LiveInterval *ptr1;
	LiveInterval *lptr = NULL;

	if (*head_reference == NULL) return;
	do {
		swapped = 0;
		ptr1 = *head_reference;

		while (ptr1->next != lptr) {
			if (ptr1->start_point > ptr1->next->start_point) {
				// SWAP DATA (It's easier than swapping nodes!!)

				int temp_start = ptr1->start_point;
				int temp_end = ptr1->end_point;
				char* temp_name = ptr1->variable_name;

				ptr1->start_point = ptr1->next->start_point;
				ptr1->end_point = ptr1->next->end_point;
				ptr1->variable_name = ptr1->next->variable_name;

				ptr1->next->start_point = temp_start;
				ptr1->next->end_point = temp_end;
				ptr1->next->variable_name = temp_name;

				swapped = 1;
			}

			ptr1 = ptr1->next;
		}

		lptr = ptr1;
	} while (swapped);
}
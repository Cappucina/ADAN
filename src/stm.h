#ifndef STM_H
#define STM_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TABLE_SIZE 1009

typedef struct SymbolEntry {
	char* name;
	char* type;
	unsigned int size;
	unsigned int dimension;
	char* decl_line;
	char* usage_line;
	char* address;

	struct SymbolEntry* next;
} SymbolEntry;

typedef struct SymbolTableManager {
	SymbolEntry* buckets[TABLE_SIZE];
	struct SymbolTableManager* parent;
	int scope_level;
} SymbolTableManager;

typedef struct SymbolTableStack {
	struct SymbolTableManager* current_scope;  // Each manager refers to its own scope.
} SymbolTableStack;

SymbolTableManager* stm_init();

SymbolTableStack* sts_init();

void stm_free(SymbolTableManager* manager);

void sts_free(SymbolTableStack* stack);

void sts_pop_scope(SymbolTableStack* stack);

void sts_push_scope(SymbolTableStack* stack);

SymbolEntry* stm_lookup_local(SymbolTableManager* manager, const char* name);

SymbolEntry* stm_lookup(SymbolTableManager* manager, const char* name);

void stm_insert(SymbolTableManager* manager, char* name, char* type, unsigned int size,
                unsigned int dimension, char* decl_line, char* usage_line, char* address);

#endif
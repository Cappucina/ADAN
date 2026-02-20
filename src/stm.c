#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stm.h"
#include "helper.h"

SymbolTableManager* stm_init()
{
	SymbolTableManager* manager = (SymbolTableManager*)calloc(1, sizeof(SymbolTableManager));
	if (!manager)
	{
		printf("No memory left to create a SymbolTableManager! (Error)");
		return NULL;
	}
	return manager;
}

SymbolTableStack* sts_init()
{
	SymbolTableStack* stack = (SymbolTableStack*)calloc(1, sizeof(SymbolTableStack));
	if (!stack)
	{
		printf("No memory left to create a SymbolTableStack! (Error)");
		return NULL;
	}
	SymbolTableManager* manager = stm_init();
	manager->scope_level = 0;
	manager->parent = NULL;
	stack->current_scope = manager;
	return stack;
}

void stm_free(SymbolTableManager* manager)
{
	if (!manager)
		return;
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		SymbolEntry* entry = manager->buckets[i];
		while (entry != NULL)
		{
			SymbolEntry* next_entry = entry->next;
			free(entry->name);
			free(entry->type);
			free(entry->decl_line);
			free(entry->usage_line);
			free(entry->address);
			free(entry);
			entry = next_entry;
		}
	}
	free(manager);
}

void sts_free(SymbolTableStack* stack)
{
	if (!stack)
		return;
	SymbolTableManager* iter = stack->current_scope;
	while (iter != NULL)
	{
		SymbolTableManager* parent = iter->parent;
		stm_free(iter);
		iter = parent;
	}
	free(stack);
}

void sts_pop_scope(SymbolTableStack* stack)
{
	if (!stack || !stack->current_scope)
		return;
	SymbolTableManager* old_scope = stack->current_scope;
	stack->current_scope = old_scope->parent;
	stm_free(old_scope);
}

void sts_push_scope(SymbolTableStack* stack)
{
	if (!stack)
		return;
	SymbolTableManager* new_scope = stm_init();
	new_scope->parent = stack->current_scope;
	new_scope->scope_level = stack->current_scope->scope_level + 1;
	stack->current_scope = new_scope;
	printf("Entered new scope (Level %d)\n", new_scope->scope_level);
}

SymbolEntry* search_buckets(SymbolEntry* buckets[], const char* name)
{
	unsigned int bucket = hash(name) % TABLE_SIZE;
	SymbolEntry* entry = buckets[bucket];
	while (entry != NULL)
	{
		if (strcmp(entry->name, name) == 0)
		{
			return entry;
		}
		entry = entry->next;
	}
	return NULL;
}

SymbolEntry* stm_lookup_local(SymbolTableManager* manager, const char* name)
{
	SymbolEntry* local_entry = search_buckets(manager->buckets, name);
	return local_entry;
}

SymbolEntry* stm_lookup(SymbolTableManager* manager, const char* name)
{
	SymbolEntry* entry = search_buckets(manager->buckets, name);
	if (entry == NULL && manager->parent != NULL)
	{
		return stm_lookup(manager->parent, name);
	}
	return entry;
}

void stm_insert(SymbolTableManager* manager, char* name, char* type, unsigned int size,
                unsigned int dimension, char* decl_line, char* usage_line, char* address)
{
	if (search_buckets(manager->buckets, name) != NULL)
	{
		// @todo Possibly write an error handler to throw custom error messages
		printf("\"%s\" was already found in the SymbolTable! (Error)", name);
		return;
	}

	unsigned int bucket = hash(name) % TABLE_SIZE;
	SymbolEntry* node = (SymbolEntry*)malloc(sizeof(SymbolEntry));
	if (!node)
	{
		printf("No memory left to allocate for a symbol entry! (Error)");
		return;
	}

	node->name = clone_string(name);
	node->type = clone_string(type);
	node->size = size;
	node->dimension = dimension;
	node->decl_line = clone_string(decl_line);
	node->usage_line = clone_string(usage_line);
	node->address = clone_string(address);
	node->next = manager->buckets[bucket];
	manager->buckets[bucket] = node;
}
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
		fprintf(stderr, "No memory left to create a SymbolTableManager! (Error)\n");
		return NULL;
	}
	return manager;
}

SymbolTableStack* sts_init()
{
	SymbolTableStack* stack = (SymbolTableStack*)calloc(1, sizeof(SymbolTableStack));
	if (!stack)
	{
		fprintf(stderr, "No memory left to create a SymbolTableStack! (Error)\n");
		return NULL;
	}
	SymbolTableManager* manager = stm_init();
	if (!manager)
	{
		fprintf(stderr,
		        "No memory left to create a SymbolTableManager for the stack! (Error)\n");
		free(stack);
		return NULL;
	}
	manager->scope_level = 0;
	manager->parent = NULL;
	stack->current_scope = manager;
	return stack;
}

void stm_free(SymbolTableManager* manager)
{
	if (!manager)
	{
		return;
	}
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
	{
		return;
	}
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
	{
		return;
	}
	SymbolTableManager* old_scope = stack->current_scope;
	stack->current_scope = old_scope->parent;
	stm_free(old_scope);
}

void sts_push_scope(SymbolTableStack* stack)
{
	if (!stack)
	{
		return;
	}
	SymbolTableManager* new_scope = stm_init();
	if (!new_scope)
	{
		fprintf(stderr, "Failed to create new scope! (Error)\n");
		return;
	}

	if (stack->current_scope)
	{
		new_scope->parent = stack->current_scope;
		new_scope->scope_level = stack->current_scope->scope_level + 1;
	}
	else
	{
		new_scope->parent = NULL;
		new_scope->scope_level = 0;
	}
	stack->current_scope = new_scope;
	printf("Entered new scope (Level %d)\n", new_scope->scope_level);
}

SymbolEntry* search_buckets(SymbolEntry* buckets[], const char* name)
{
	if (!name)
	{
		return NULL;
	}

	unsigned int bucket = hash(name) % TABLE_SIZE;
	SymbolEntry* entry = buckets[bucket];
	while (entry != NULL)
	{
		if (entry->name && strcmp(entry->name, name) == 0)
		{
			return entry;
		}
		entry = entry->next;
	}
	return NULL;
}

SymbolEntry* stm_lookup_local(SymbolTableManager* manager, const char* name)
{
	if (!manager)
	{
		fprintf(stderr, "stm_lookup_local called with NULL manager. (Warning)\n");
		return NULL;
	}
	return search_buckets(manager->buckets, name);
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
                char* decl_line, char* usage_line, char* address)
{
	if (search_buckets(manager->buckets, name) != NULL)
	{
		fprintf(stderr, "\"%s\" was already found in the SymbolTable! (Error)\n", name);
		return;
	}

	unsigned int bucket = hash(name) % TABLE_SIZE;
	SymbolEntry* node = (SymbolEntry*)malloc(sizeof(SymbolEntry));
	if (!node)
	{
		fprintf(stderr, "No memory left to allocate for a symbol entry! (Error)\n");
		return;
	}

	node->name = name ? clone_string(name, strlen(name)) : NULL;
	if (!node->name)
	{
		goto cleanup_node;
	}
	node->type = type ? clone_string(type, strlen(type)) : NULL;
	if (!node->type)
	{
		goto cleanup_node;
	}
	node->size = size;
	node->decl_line = decl_line ? clone_string(decl_line, strlen(decl_line)) : NULL;
	if (!node->decl_line)
	{
		goto cleanup_node;
	}
	node->usage_line = usage_line ? clone_string(usage_line, strlen(usage_line)) : NULL;
	if (usage_line && !node->usage_line)
	{
		goto cleanup_node;
	}
	node->address = address ? clone_string(address, strlen(address)) : NULL;
	if (address && !node->address)
	{
		goto cleanup_node;
	}

	node->next = manager->buckets[bucket];
	manager->buckets[bucket] = node;
	return;

cleanup_node:
	if (node->name)
	{
		free(node->name);
	}
	if (node->type)
	{
		free(node->type);
	}
	if (node->decl_line)
	{
		free(node->decl_line);
	}
	if (node->usage_line)
	{
		free(node->usage_line);
	}
	if (node->address)
	{
		free(node->address);
	}
	free(node);
}
#include <string.h>

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
} SymbolTableManager;

SymbolTableManager* stm_init() {
    SymbolTableManager* manager = (SymbolTableManager*)calloc(1, sizeof(SymbolTableManager));
    if (!manager) {
        printf("No memory left to create a SymbolTableManager! (Error)");
        return NULL;
    }
    return manager;
}

void stm_free(SymbolTableManager* manager) {
    if (!manager) return;
    for (int i = 0; i < TABLE_SIZE; i++) {
        SymbolEntry* entry = manager->buckets[i];
        while (entry != NULL) {
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

SymbolEntry* stm_lookup(SymbolTableManager* manager, char* name) {
    unsigned int bucket = hash(name) % TABLE_SIZE;
    SymbolEntry* entry = manager->buckets[bucket];

    while (entry != NULL) {
        if (strcmp(name, entry->name) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}

void stm_insert(SymbolTableManager* manager, char* name, char* type, unsigned int size,
                                             unsigned int dimension, char* decl_line, char* usage_line,
                                             char* address) {
    
    // @important When changing from a single-level symbol table, remove and add support for various scopes.
    if (stm_lookup(manager, name) != NULL) {
        // @todo Possibly write an error handler to throw custom error messages
        printf("\"%s\" was already found in the SymbolTable! (Error)");
        return;
    }

    unsigned int bucket = hash(name) % TABLE_SIZE;
    SymbolEntry* node = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    if (!node) {
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
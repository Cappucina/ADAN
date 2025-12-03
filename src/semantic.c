#include "semantic.h"
#include "util.h"

SymbolTable* init_symbol_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));

    table->bucket_count = 64;
    table->buckets = calloc(table->bucket_count, sizeof(Symbol*));
    table->parent = NULL;

    return table;
}

void enter_scope(SymbolTable* table) {
    SymbolTable* new_table = init_symbol_table();
    new_table->parent = table;
    *table = *new_table;
    free(new_table);
}

void exit_scope(SymbolTable* table) {
    if (table->parent) {
        SymbolTable* parent = table->parent;
        for (int i = 0; i < table->bucket_count; i++) {
            Symbol* sym = table->buckets[i];
            while (sym) {
                Symbol* next = sym->next;
                free(sym->name);
                free(sym);
                sym = next;
            }
        }

        free(table->buckets);
        *table = *parent;
    }
}

bool add_symbol(SymbolTable* table, const char* name, Type type, ASTNode* node) {
    if (!table || !name) return false;

    unsigned long hash = hash_string(name);
    int index = (int)(hash % table->bucket_count);

    //
    //  Check for duplicates in the current scope
    //
    Symbol* current_symbol = table->buckets[index];
    while (current_symbol) {
        if (strcmp(current_symbol->name, name) == 0) return false;
        current_symbol = current_symbol->next;
    }

    // 
    //  Allocate a new symbol
    // 
    Symbol* new_symbol = malloc(sizeof(Symbol));
    
    if (!new_symbol) return false;
    if (!new_symbol->name) {
        free(new_symbol);
        return false;
    }

    new_symbol->type = type;
    new_symbol->node = node;
    new_symbol->next = table->buckets[index];

    table->buckets[index] = new_symbol;
    return true;
}

Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    if (!table || !name) return NULL;

    while (table) {
        if (!table->buckets || table->bucket_count <= 0) {
            table = table->parent;
            continue;
        }

        unsigned long hash = hash_string(name);
        int index = (int)(hash % table->bucket_count);
        
        Symbol* current_symbol = table->buckets[index];
        while (current_symbol) {
            if (strcmp(current_symbol->name, name) == 0) {
                return current_symbol;
            }
            current_symbol = current_symbol->next;
        }
        table = table->parent;
    }
    
    return NULL;
}

bool symbol_in_scope(SymbolTable* table, const char* name) {
    if (!table || !name) return false;

    unsigned long hash = hash_string(name);
    int index = (int)(hash % table->bucket_count);

    Symbol* current_symbol = table->buckets[index];
    while (current_symbol) {
        if (strcmp(current_symbol->name, name) == 0) return true;
        current_symbol = current_symbol->next;
    }

    return false;
}

// 
//  AST Traversal / Semantic Checks
// 
void analyze_program(ASTNode* root, SymbolTable* table) {

}

void analyze_block(ASTNode* block, SymbolTable* table) {

}

void analyze_statement(ASTNode* statement, SymbolTable* table) {

}

void analyze_for(ASTNode* for_node, SymbolTable* table) {

}

void analyze_if(ASTNode* if_node, SymbolTable* table) {

}

void analyze_while(ASTNode* while_node, SymbolTable* table) {

}

void analyze_return(ASTNode* return_node, SymbolTable* table) {

}

void analyze_assignment(ASTNode* assignment_node, SymbolTable* table) {

}

Type analyze_expression(ASTNode* expr_node, SymbolTable* table) {

}

Type analyze_binary_op(ASTNode* binary_node, SymbolTable* table) {

}

Type analyze_unary_op(ASTNode* unary_node, SymbolTable* table) {

}

// 
//  Type System / Helper functions
// 
Type get_expression_type(ASTNode* expr_node, SymbolTable* table) {
    if (!expr_node || !table) return TYPE_UNKNOWN;

    return expr_node->annotated_type;
}

bool check_type_compatibility(Type expected, Type actual) {
    return expected == actual;
}

bool is_numeric_type(Type type) {

}

bool is_boolean_type(Type type) {

}

//
//  Error Handling (Errors, Warnings, Tips, etc.)
// 
void semantic_error(ASTNode* node, const char* fmt, ...) {

}

void semantic_warning(ASTNode* node, const char* fmt, ...) {

}

void semantic_tip(ASTNode* node, const char* fmt, ...) {

}

// 
//  AST Annotation and Utilities
// 
void annotate_node_type(ASTNode* node, Type type) {

}

void print_symbol_table(SymbolTable* table) {

}

void free_symbol_table(SymbolTable* table) {

}
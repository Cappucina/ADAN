#include "semantic.h"

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

}

Symbol* lookup_symbol(SymbolTable* table, const char* name) {

}

bool symbol_in_scope(SymbolTable* table, const char* name) {

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

}

bool check_type_compatibility(Type expected, Type actual) {

}

bool is_numeric_type(Type type) {

}

bool is_boolean_type(Type type) {

}

//
//  Error Handling (Errors, Warnings, Tips, etc.)
// 
void semantic_error(const char* format, ...) {

}

void semantic_warning(const char* format, ...) {

}

void semantic_tip(const char* format, ...) {

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
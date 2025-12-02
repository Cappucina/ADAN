#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOLEAN,
    TYPE_CHAR,
    TYPE_NULL,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_UNKNOWN
} Type;

typedef struct Symbol {
    char* name;
    Type type;
    ASTNode* node;
    struct Symbol* next;
} Symbol;

typedef struct SymbolTable {
    Symbol** buckets;
    int bucket_count;
    struct SymbolTable* parent;
} SymbolTable;

SymbolTable* init_symbol_table();

void enter_scope(SymbolTable* table);

void exit_scope(SymbolTable* table);

bool add_symbol(SymbolTable* table, const char* name, Type type, ASTNode* node);

Symbol* lookup_symbol(SymbolTable* table, const char* name);

bool symbol_in_scope(SymbolTable* table, const char* name);

// 
//  AST Traversal / Semantic Checks
// 
void analyze_program(ASTNode* root, SymbolTable* table);

void analyze_block(ASTNode* block, SymbolTable* table);

void analyze_statement(ASTNode* statement, SymbolTable* table);

void analyze_for(ASTNode* for_node, SymbolTable* table);

void analyze_if(ASTNode* if_node, SymbolTable* table);

void analyze_while(ASTNode* while_node, SymbolTable* table);

void analyze_return(ASTNode* return_node, SymbolTable* table);

void analyze_assignment(ASTNode* assignment_node, SymbolTable* table);

Type analyze_expression(ASTNode* expr_node, SymbolTable* table);

Type analyze_binary_op(ASTNode* binary_node, SymbolTable* table);

Type analyze_unary_op(ASTNode* unary_node, SymbolTable* table);

// 
//  Type System / Helper functions
// 
Type get_expression_type(ASTNode* expr_node, SymbolTable* table);

bool check_type_compatibility(Type expected, Type actual);

bool is_numeric_type(Type type);

bool is_boolean_type(Type type);

//
//  Error Handling (Errors, Warnings, Tips, etc.)
// 
void semantic_error(ASTNode* node, const char* fmt, ...);

void semantic_warning(ASTNode* node, const char* fmt, ...);

void semantic_tip(ASTNode* node, const char* fmt, ...);

// 
//  AST Annotation and Utilities
// 
void annotate_node_type(ASTNode* node, Type type);

void print_symbol_table(SymbolTable* table);

void free_symbol_table(SymbolTable* table);

#endif
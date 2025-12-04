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
    if (root == NULL && table == NULL) return;
    if (root->type != AST_PROGRAM) return;

    
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
    if (!expr_node) return TYPE_UNKNOWN;
    if (expr_node->type == AST_LITERAL) {
        TokenType tt = expr_node->token.type;
        Type inferred = TYPE_UNKNOWN;

        switch (tt) {
            case TOKEN_INT_LITERAL:
                inferred = TYPE_INT;
                break;
            case TOKEN_FLOAT_LITERAL:
                inferred = TYPE_FLOAT;
                break;

            case TOKEN_STRING:
                inferred = TYPE_STRING;
                break;

            case TOKEN_TRUE:
            case TOKEN_FALSE:
            case TOKEN_BOOLEAN:
                inferred = TYPE_BOOLEAN;
                break;

            case TOKEN_CHAR:
                inferred = TYPE_CHAR;
                break;
            
                case TOKEN_NULL:
                inferred = TYPE_NULL;
                break;
        }

        annotate_node_type(expr_node, inferred);
        return inferred;
    }

    if (expr_node->type == AST_IDENTIFIER) {
        if (expr_node->token.text && table) {
            Symbol* s = lookup_symbol(table, expr_node->token.text);
            Type t = s ? s->type : TYPE_UNKNOWN;
            annotate_node_type(expr_node, t);
            return t;
        }

        annotate_node_type(expr_node, TYPE_UNKNOWN);
        return TYPE_UNKNOWN;
    }

    if (expr_node->type == AST_BINARY_OP || expr_node->type == AST_BINARY_EXPR ||
        expr_node->type == AST_COMPARISON || expr_node->type == AST_LOGICAL_OP) {
        if (expr_node->child_count < 2) {
            annotate_node_type(expr_node, TYPE_UNKNOWN);
            return TYPE_UNKNOWN;
        }
    
        Type L = get_expression_type(expr_node->children[0], table);
        Type R = get_expression_type(expr_node->children[1], table);

        if (is_numeric_type(L) && is_numeric_type(R)) {
            Type res = (L == TYPE_FLOAT || R == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
            annotate_node_type(expr_node, res);
            return res;
        }

        if (expr_node->type == AST_COMPARISON || expr_node->type == AST_LOGICAL_OP) {
            annotate_node_type(expr_node, TYPE_BOOLEAN);
            return TYPE_BOOLEAN;
        }

        if (L == R && L != TYPE_UNKNOWN) {
            annotate_node_type(expr_node, L);
            return L;
        }

        annotate_node_type(expr_node, TYPE_UNKNOWN);
        return TYPE_UNKNOWN;
    }

    if (expr_node->type == AST_UNARY_OP || expr_node->type == AST_UNARY_EXPR) {
        if (expr_node->child_count < 1) {
            annotate_node_type(expr_node, TYPE_UNKNOWN);
            return TYPE_UNKNOWN;
        }
        
        if (expr_node->token.type == TOKEN_NOT) {
            annotate_node_type(expr_node, TYPE_BOOLEAN);
            return TYPE_BOOLEAN;
        }
        
        Type inner = get_expression_type(expr_node->children[0], table);
        annotate_node_type(expr_node, inner);
        
        return inner;
    }

    if (expr_node->type == AST_ARRAY_LITERAL) {
        annotate_node_type(expr_node, TYPE_ARRAY);
        return TYPE_ARRAY;
    }

    if (expr_node->child_count > 0) {
        Type t = get_expression_type(expr_node->children[0], table);
        annotate_node_type(expr_node, t);
        return t;
    }
 
    annotate_node_type(expr_node, TYPE_UNKNOWN);
    return TYPE_UNKNOWN;
}

bool check_type_compatibility(Type expected, Type actual) {
    if (expected == TYPE_UNKNOWN || actual == TYPE_UNKNOWN) return false;
    if (expected == actual) return true;
    if (is_numeric_type(expected) && is_numeric_type(actual)) return true;
    if (is_numeric_type(expected) && actual == TYPE_CHAR) return true;
    if (actual == TYPE_NULL && (expected == TYPE_STRING || expected == TYPE_ARRAY || expected == TYPE_NULL)) return true;

    return false;
}

bool is_numeric_type(Type type) {
    return type == TYPE_INT || type == TYPE_FLOAT;
}

bool is_boolean_type(Type type) {
    return type == TYPE_BOOLEAN;
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
    if (!node) return;
    node->annotated_type = type;
}

void free_symbol_table(SymbolTable* table) {
    if (!table) return;
    if (table->buckets) {
        for (int index = 0; index < table->bucket_count; index++) {
            Symbol* current_symbol = table->buckets[index];

            while (current_symbol) {
                Symbol* next_symbol = current_symbol->next;
                if (current_symbol->name) free(current_symbol->name);
                free(current_symbol);
                current_symbol = next_symbol;
            }
        }

        free(table->buckets);
        table->buckets = NULL;
    }

    free(table);
}
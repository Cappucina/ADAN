#ifndef LIBRARY_H
#define LIBRARY_H

#include "ast.h"
#include "semantic.h"
#include "parser.h"

typedef struct LibraryFunction {
    char* name;
    CompleteType return_type;
    int param_count;
    CompleteType* param_types;
    char** param_names;
    struct LibraryFunction* next;
} LibraryFunction;

typedef struct Library {
	char* publisher;
	char* package;
	LibraryFunction* functions;
	ASTNode* ast;
	struct Library* next;
} Library;

typedef struct LibraryRegistry {
	Library* libraries;
	char* search_path;
} LibraryRegistry;

LibraryRegistry* init_library_registry(const char* search_path);

void free_library_function(LibraryFunction* func);

void free_library(Library* lib);

void free_library_registry(LibraryRegistry* registry);

char* resolve_library_path(LibraryRegistry* registry, const char* publisher, const char* package);

Library* load_library(LibraryRegistry* registry, const char* publisher, const char* package);

bool import_library_symbols(Library* lib, SymbolTable* table);

#endif
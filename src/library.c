#include "library.h"
#include "logs.h"
#include "util.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "flags.h"

static char *trim(char *s)
{
	if (!s)
		return s;
	while (*s && isspace((unsigned char)*s))
		s++;
	char *end = s + strlen(s);
	while (end > s && isspace((unsigned char)*(end - 1)))
		end--;
	*end = '\0';
	return s;
}

static CompleteType map_c_type(const char *t)
{
    CompleteType type_unknown = { TYPE_UNKNOWN, NULL };

    if (!t) return type_unknown;

    char buf[128];
    int j = 0;
    for (int i = 0; t[i] && j < (int)sizeof(buf) - 1; i++) {
        if (!isspace((unsigned char)t[i]))
            buf[j++] = t[i];
    }
    buf[j] = '\0';

    char *p = buf;
    while (!strncmp(p, "const", 5)) p += 5;
    while (!strncmp(p, "volatile", 8)) p += 8;

    int depth = 0;
    for (char *q = p; *q; q++) {
        if (*q == '*') depth++;
    }

    char base[128];
    j = 0;
    for (int i = 0; p[i]; i++) {
        if (p[i] != '*')
            base[j++] = p[i];
    }
    base[j] = '\0';

    if (strcmp(base, "char") == 0) {
        if (depth == 0) {
            return (CompleteType){ TYPE_CHAR, NULL };
        }

        CompleteType t = { TYPE_STRING, NULL };

        for (int i = 1; i < depth; i++) {
            CompleteType wrap = { TYPE_ARRAY, malloc(sizeof(CompleteType)) };
            if (!wrap.points_to) return type_unknown;
            *wrap.points_to = t;
            t = wrap;
        }

        return t;
    }

    CompleteType base_type;

    if (!strcmp(base, "int") ||
        !strcmp(base, "long") ||
        !strcmp(base, "size_t") ||
        !strcmp(base, "uid_t")) {
        base_type = (CompleteType){ TYPE_INT, NULL };
    }
    else if (!strcmp(base, "float") || !strcmp(base, "double")) {
        base_type = (CompleteType){ TYPE_FLOAT, NULL };
    }
    else if (!strcmp(base, "bool") || !strcmp(base, "_Bool")) {
        base_type = (CompleteType){ TYPE_BOOLEAN, NULL };
    }
    else if (!strcmp(base, "void")) {
        base_type = (CompleteType){ TYPE_VOID, NULL };
    }
    else {
        return type_unknown;
    }

    CompleteType tcur = base_type;
    for (int i = 0; i < depth; i++) {
        CompleteType wrap = { TYPE_POINTER, malloc(sizeof(CompleteType)) };
        if (!wrap.points_to) return type_unknown;
        *wrap.points_to = tcur;
        tcur = wrap;
    }

    return tcur;
}

static LibraryFunction *parse_c_prototype(char *line)
{
    if (!line)
        return NULL;
    char *l = trim(line);

    if (*l == '\0' || *l == '/' || *l == '#' || !strchr(l, '(') || !strchr(l, ')'))
        return NULL;

    char *lp = strchr(l, '(');
    char *rp = strchr(lp, ')');
    *lp = '\0';
    *rp = '\0';

    char *ret_and_name = trim(l);
    char *params_str = trim(lp + 1);
    char *last_space = strrchr(ret_and_name, ' ');
    if (!last_space)
        return NULL;

    const char *name = trim(last_space + 1);
    char saved = *last_space;
    *last_space = '\0';
    char *type_only = trim(ret_and_name);

    CompleteType ret_type = map_c_type(type_only);
    *last_space = saved;

    if (ret_type.type == TYPE_UNKNOWN)
        return NULL;

    LibraryFunction *func = malloc(sizeof(LibraryFunction));
    if (!func)
        return NULL;
    func->name = strdup(name);
    if (!func->name) {
        free(func);
        return NULL;
    }

    func->return_type = ret_type;
    func->param_count = 0;
    func->param_types = NULL;
    func->param_names = NULL;
    func->next = NULL;

    if (params_str && *params_str && strcmp(params_str, "void") != 0) {
        int count = 0;
        for (char *p = params_str; *p; p++)
            if (*p == ',')
                count++;
        count++;

        func->param_types = malloc(sizeof(Type) * count);
        func->param_names = malloc(sizeof(char *) * count);
        if (!func->param_types || !func->param_names) {
            free(func->param_types);
            free(func->param_names);
            free(func->name);
            free(func);
            return NULL;
        }

        func->param_count = 0;
        char *saveptr = NULL;
        for (char *tok = strtok_r(params_str, ",", &saveptr); tok; tok = strtok_r(NULL, ",", &saveptr)) {
            char *part = trim(tok);
            char *last = strrchr(part, ' ');
            const char *pname = last ? trim(last + 1) : part;
            if (last)
                *last = '\0';
            char *type_part = trim(part);
            char *end = type_part;
            while (*end && !isspace((unsigned char)*end) && *end != '*' && *end != '[')
                end++;
            *end = '\0';

            CompleteType pt = map_c_type(type_part);
            if (last)
                *last = ' ';

            func->param_types[func->param_count] = pt;
            func->param_names[func->param_count] = strdup(pname);
            func->param_count++;
        }
    }

    return func;
}

LibraryRegistry *init_library_registry(CompilerFlags *flags)
{
	LibraryRegistry *registry = malloc(sizeof(LibraryRegistry));
	if (!registry)
		return NULL;

	registry->libraries = NULL;

	if (flags->lib_count > 0)
	{
		registry->search_paths = malloc((flags->lib_count + 1) * sizeof(char *));
		if (!registry->search_paths)
		{
			free(registry);
			return NULL;
		}

		for (int i = 0; i < flags->lib_count; i++)
		{
			registry->search_paths[i] = strdup(flags->libs[i]);
		}
		registry->search_paths[flags->lib_count] = NULL;
	}
	else
	{
		registry->search_paths = malloc(2 * sizeof(char *));
		if (!registry->search_paths)
		{
			free(registry);
			return NULL;
		}
		registry->search_paths[0] = strdup("../lib");
		registry->search_paths[1] = NULL;
	}

	return registry;
}

void free_library_function(LibraryFunction *func)
{
	if (!func)
		return;
	free(func->name);
	free(func->param_types);

	for (int i = 0; i < func->param_count; i++)
	{
		free(func->param_names[i]);
	}

	free(func->param_names);
	free(func);
}

void free_library(Library *lib)
{
	if (!lib)
		return;
	free(lib->package);
	free(lib->publisher);

	LibraryFunction *func = lib->functions;
	while (func)
	{
		LibraryFunction *next = func->next;
		free_library_function(func);
		func = next;
	}
	if (lib->ast)
	{
		free_ast(lib->ast);
	}
	free(lib);
}

void free_library_registry(LibraryRegistry *registry)
{
	if (!registry)
		return;

	Library *lib = registry->libraries;
	while (lib)
	{
		Library *next = lib->next;
		free_library(lib);
		lib = next;
	}

	free(registry->search_paths);
	free(registry);
}

char *resolve_library_path(LibraryRegistry *registry, const char *publisher, const char *package)
{
	if (!registry || !publisher || !package || !registry->search_paths)
		return NULL;

	for (int i = 0; registry->search_paths[i]; i++)
	{
		size_t path_length = strlen(registry->search_paths[i]) + strlen(publisher) + strlen(package) + 20;
		char *lib_path = malloc(path_length);
		if (!lib_path)
			continue;

		snprintf(lib_path, path_length, "%s/%s/%s.adn", registry->search_paths[i], publisher, package);
		if (access(lib_path, F_OK) == 0)
			return lib_path;

		snprintf(lib_path, path_length, "%s/%s/%s.c", registry->search_paths[i], publisher, package);
		if (access(lib_path, F_OK) == 0)
			return lib_path;

		free(lib_path);
	}

	return NULL;
}

Library *load_library(LibraryRegistry *registry, const char *publisher, const char *package)
{
	if (!registry || !publisher || !package)
		return NULL;

	Library *existing = registry->libraries;
	while (existing)
	{
		if (existing->publisher && existing->package &&
			strcmp(existing->publisher, publisher) == 0 &&
			strcmp(existing->package, package) == 0)
		{
			return existing;
		}

		existing = existing->next;
	}

	char *lib_path = resolve_library_path(registry, publisher, package);
	if (!lib_path)
	{
		fprintf(stderr, "error: Failed to resolve library path for '%s.%s'\n", publisher, package);
		return NULL;
	}

	size_t len = strlen(lib_path);
	int is_c_file = strstr(lib_path, ".c") != NULL;

	Library *lib = malloc(sizeof(Library));
	if (!lib)
	{
		free(lib_path);
		return NULL;
	}

	lib->publisher = strdup(publisher);
	lib->package = strdup(package);
	if (!lib->publisher || !lib->package)
	{
		free(lib->publisher);
		free(lib->package);
		free(lib_path);
		free(lib);
		return NULL;
	}
	lib->functions = NULL;
	lib->ast = NULL;
	lib->next = registry->libraries;

	FILE *lib_file = fopen(lib_path, "r");
	if (!lib_file)
	{
		fprintf(stderr, "error: Library file not found: %s\n", lib_path);
		free(lib_path);
		free(lib);
		return NULL;
	}

	if (is_c_file)
	{
		char line[256];
		while (fgets(line, sizeof(line), lib_file))
		{
			LibraryFunction *func = parse_c_prototype(line);
			if (!func)
				continue;
			func->next = lib->functions;
			lib->functions = func;
		}
		fclose(lib_file);
		registry->libraries = lib;
		free(lib_path);
		return lib;
	}

	fseek(lib_file, 0, SEEK_END);
	long file_size = ftell(lib_file);
	fseek(lib_file, 0, SEEK_SET);

	char *source = malloc(file_size + 1);
	if (!source)
	{
		fclose(lib_file);
		free(lib_path);
		free(lib);
		return NULL;
	}

	fread(source, 1, file_size, lib_file);
	source[file_size] = '\0';
	fclose(lib_file);

	Lexer *lexer = create_lexer(source);
	if (!lexer)
	{
		free(source);
		free(lib_path);
		free(lib);
		return NULL;
	}

	Parser parser;
	init_parser(&parser, lexer);

	ASTNode *file_ast = create_ast_node(AST_FILE, (Token){0});
	ASTNode **programs = NULL;
	int program_count = 0;

	while (parser.current_token.type != TOKEN_EOF)
	{
		if (parser.current_token.type == TOKEN_PROGRAM)
		{
			ASTNode *func_ast = parse_program(&parser);
			if (!func_ast)
			{
				fprintf(stderr, "error: Failed to parse function in library '%s.%s'\n", publisher, package);
				continue;
			}

			if (func_ast->child_count < 3 || !func_ast->children[0] || !func_ast->children[1] || !func_ast->children[2])
			{
				fprintf(stderr, "error: Invalid function AST structure in library '%s.%s'\n", publisher, package);
				free_ast(func_ast);
				continue;
			}

			ASTNode **tmp = realloc(programs, sizeof(ASTNode *) * (program_count + 1));
			if (!tmp)
			{
				free_ast(func_ast);
				continue;
			}
			programs = tmp;
			programs[program_count++] = func_ast;

			LibraryFunction *func = malloc(sizeof(LibraryFunction));
			if (!func)
			{
				free_ast(func_ast);
				continue;
			}

			if (!func_ast->children[1]->token.text)
			{
				free(func);
				free_ast(func_ast);
				continue;
			}

			func->name = strdup(func_ast->children[1]->token.text);
			if (!func->name)
			{
				free(func);
				free_ast(func_ast);
				continue;
			}

			Token type_token = func_ast->children[0]->token;
			switch (type_token.type)
			{
			case TOKEN_INT:
				func->return_type.type = TYPE_INT;
				break;
			case TOKEN_FLOAT:
				func->return_type.type = TYPE_FLOAT;
				break;
			case TOKEN_STRING:
				func->return_type.type = TYPE_STRING;
				break;
			case TOKEN_BOOLEAN:
				func->return_type.type = TYPE_BOOLEAN;
				break;
			case TOKEN_CHAR:
				func->return_type.type = TYPE_CHAR;
				break;
			case TOKEN_VOID:
				func->return_type.type = TYPE_VOID;
				break;
			default:
				func->return_type.type = TYPE_UNKNOWN;
				break;
			}

			ASTNode *params = func_ast->children[2];
			func->param_count = params ? params->child_count : 0;
			func->param_types = malloc(sizeof(Type) * func->param_count);
			func->param_names = malloc(sizeof(char *) * func->param_count);
			if (!func->param_types || !func->param_names)
			{
				free(func->param_types);
				free(func->param_names);
				free(func->name);
				free(func);
				free_ast(func_ast);
				continue;
			}

			for (int j = 0; j < func->param_count; j++)
			{
				ASTNode *param = params->children[j];
				if (!param || param->child_count < 2 || !param->children[0] || !param->children[1])
				{
					func->param_names[j] = NULL;
					func->param_types[j].type = TYPE_UNKNOWN;
					continue;
				}
				if (!param->children[0]->token.text)
				{
					func->param_names[j] = NULL;
					func->param_types[j].type = TYPE_UNKNOWN;
					continue;
				}
				func->param_names[j] = strdup(param->children[0]->token.text);
				if (!func->param_names[j])
				{
					func->param_types[j].type = TYPE_UNKNOWN;
					continue;
				}

				Token param_type_token = param->children[1]->token;
				switch (param_type_token.type)
				{
				case TOKEN_INT:
					func->param_types[j].type = TYPE_INT;
					break;
				case TOKEN_FLOAT:
					func->param_types[j].type = TYPE_FLOAT;
					break;
				case TOKEN_STRING:
					func->param_types[j].type = TYPE_STRING;
					break;
				case TOKEN_BOOLEAN:
					func->param_types[j].type = TYPE_BOOLEAN;
					break;
				case TOKEN_CHAR:
					func->param_types[j].type = TYPE_CHAR;
					break;
				case TOKEN_VOID:
					func->param_types[j].type = TYPE_VOID;
					break;
				default:
					func->param_types[j].type = TYPE_UNKNOWN;
					break;
				}
			}

			func->next = lib->functions;
			lib->functions = func;
		}
		else
		{
			match(&parser, parser.current_token.type);
		}
	}

	if (program_count > 0)
	{
		file_ast->children = programs;
		file_ast->child_count = program_count;
		lib->ast = file_ast;
	}
	else
	{
		free(programs);
		free_ast(file_ast);
	}

	registry->libraries = lib;

	free(source);
	free(lib_path);
	free_parser(&parser);
	free(lexer);

	return lib;
}

bool import_library_symbols(Library *lib, SymbolTable *table)
{
	if (!lib || !table)
		return false;

	LibraryFunction *func = lib->functions;
	while (func)
	{
		if (!func->name)
		{
			func = func->next;
			continue;
		}

		if (!add_symbol(table, func->name, func->return_type, NULL))
			return false;

		func = func->next;
	}

	return true;
}
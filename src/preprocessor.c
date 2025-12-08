#include "preprocessor.h"

Preprocessor* create_preprocessor() {
    Preprocessor* pp = (Preprocessor*)malloc(sizeof(Preprocessor));
    if (!pp) return NULL;

    pp->included_count = 0;
    pp->included_files = NULL;

    return pp;
}

char* preprocess_file(Preprocessor* pp, const char* path) {
    if (is_file_included(pp, path)) {
        // Throw Error: File already included
        return strdup("");
    }

    mark_as_included(pp, path);

    char* content = load_file(path);
    if (!content) {
        // Throw Error: Unable to load file
        return NULL;
    }

    char* result = strdup("");
    char* cur_pos = content;

    while (1) {
        char* include_keyword = strstr(cur_pos, "include");
        if (!include_keyword) {
            result = (char*)realloc(result, (strlen(result) + strlen(cur_pos) + 1));
            strcat(result, cur_pos);
            break;
        }

        result = (char*)realloc(result, strlen(result) + (include_keyword - cur_pos) + 1);
        strncat(result, cur_pos, include_keyword - cur_pos);

        char* start = include_keyword + strlen("include");
        while (*start == ' ' || *start == '\t') start++;
        char* end = strchr(start, ';');
        if (!end) {
            // Throw Error: Couldn't find semicolon
            free(content);
            free(result);
            return NULL;
        }

        char* actual_include = (char*)malloc(end - start + 1);
        strncpy(actual_include, start, end - start);
        actual_include[end - start] = '\0';

        char* path = get_package_path(actual_include);
        free(actual_include);
        if (!path) {
            // Throw Error: Unable to resolve package path
            free(content);
            free(result);
            return NULL;
        }

        char* included_content = preprocess_file(pp, path);
        free(path);
        
        if (!included_content) {
            // Throw Error: Unable to preprocess included file
            free(content);
            free(result);
            return NULL;
        }
        result = (char*)realloc(result, strlen(result) + strlen(included_content) + 1);
        strcat(result, included_content);
        free(included_content);

        cur_pos = end + 1;
    }
    free(content);
    return result;
}

char* load_file(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        // Throw Error: Unable to open file
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc(size + 1);

    fread(content, 1, size, file);
    content[size] = '\0';

    fclose(file);

    return content;
}

bool is_file_included(Preprocessor* pp, const char* path) {
    for (int i = 0; i < pp->included_count; i++) {
        if (strcmp(pp->included_files[i], path) == 0) {
            return true;
        }
    }
    return false;
}

void mark_as_included(Preprocessor* pp, const char* path) {
    pp->included_count++;
    pp->included_files = (char**)realloc(pp->included_files, pp->included_count * sizeof(char*));
    pp->included_files[pp->included_count - 1] = strdup(path);
}

void free_preprocessor(Preprocessor* pp) {
    if (!pp) return;
    for (int i = 0; i < pp->included_count; i++) {
        free(pp->included_files[i]);
    }
    free(pp->included_files);
    free(pp);
}

char* get_package_path(const char* package_name) {
    // Takes in package name like "adan.io" and returns a real path
    // I'm not doing ts; nvm ig I'm doing ts FUCKJ

    char* env = getenv("ADAN_PACKAGE_PATH");
    if (!env) {
        // Throw Error: ADAN_PACKAGE_PATH not set
        return NULL;
    }

    char* full_path = (char*)malloc(strlen(env) + strlen(package_name) + 6); // 6 for '/', ".adn", and '\0'

    strcpy(full_path, env);
    strcat(full_path, "/");

    char* name_copy = strdup(package_name);
    if (!name_copy) {
        free(full_path);
        return NULL;
    }

    char* dot_pos = strchr(name_copy, '.');
    if (!dot_pos) {
        free(name_copy);
        free(full_path);
        // Throw Error: Invalid package name (no dot)
        return NULL;
    }

    if (strchr(dot_pos + 1, '.')) {
        free(name_copy);
        free(full_path);
        // Throw Error: Invalid package name (multiple dots)
        return NULL;
    }

    *dot_pos = '/';

    strcat(full_path, name_copy);
    strcat(full_path, ".adn");
    free(name_copy);
    return full_path;
}
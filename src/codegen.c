// For utils to go alongside codegen for specific architectures and OSes

#include <stdio.h>
#include <stdlib.h>
#include "codegen.h"

// Returns a heap-allocated array of Token* and writes its length to out_token_count.
// On failure, returns NULL and sets *out_token_count = 0.
Token** convertToLogic(const char *input_file) {
    FILE   *file        = NULL;
    char   *source_code = NULL;
    Lexer  *lexer       = NULL;
    Token **tokens      = NULL;

    size_t file_size    = 0;
    size_t capacity     = 0;
    size_t count        = 0;

    printf("Converting %s to logic representation...\n", input_file);

    // 1. Open file
    file = fopen(input_file, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", input_file);
        return NULL;
    }

    // 2. Determine file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error: fseek failed for %s\n", input_file);
        goto error;
    }

    long fsize = ftell(file);
    if (fsize < 0) {
        fprintf(stderr, "Error: ftell failed for %s\n", input_file);
        goto error;
    }
    file_size = (size_t)fsize;

    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Error: fseek (rewind) failed for %s\n", input_file);
        goto error;
    }

    // 3. Read file into buffer and null-terminate
    source_code = (char *)malloc(file_size + 1);
    if (!source_code) {
        fprintf(stderr, "Error: Memory allocation failed for source_code\n");
        goto error;
    }

    if (file_size > 0) {
        size_t read_bytes = fread(source_code, 1, file_size, file);
        if (read_bytes != file_size) {
            fprintf(stderr, "Error: fread failed for %s (expected %zu, got %zu)\n",
                    input_file, file_size, read_bytes);
            goto error;
        }
    }
    source_code[file_size] = '\0';

    // 4. Create lexer on the source buffer
    lexer = create_lexer(source_code);
    if (!lexer) {
        fprintf(stderr, "Error: create_lexer failed for %s\n", input_file);
        goto error;
    }

    // 5. Dynamic array of Token*
    capacity = 16;  // initial capacity
    tokens = (Token **)malloc(capacity * sizeof(Token *));
    if (!tokens) {
        fprintf(stderr, "Error: Memory allocation failed for tokens array\n");
        goto error;
    }

    // 6. Lex tokens into the dynamic array
    while (1) {
        Token *token = next_token(lexer);
        if (!token) {
            fprintf(stderr, "Error: next_token returned NULL\n");
            goto error;
        }

        if (token->type == TOKEN_EOF) {
            // We don't store the EOF token; just free it and stop.
            free_token(token);
            break;
        }

        if (count == capacity) {
            size_t new_capacity = capacity * 2;
            Token **new_tokens = (Token **)realloc(tokens, new_capacity * sizeof(Token *));
            if (!new_tokens) {
                fprintf(stderr, "Error: realloc failed while growing tokens array\n");
                free_token(token);
                goto error;
            }
            tokens   = new_tokens;
            capacity = new_capacity;
        }

        tokens[count++] = token;
    }

    printf("Total tokens generated: %zu\n", count);

    // 7. Optionally shrink to fit
    if (count > 0) {
        Token **shrunk = (Token **)realloc(tokens, count * sizeof(Token *));
        if (shrunk) {
            tokens = shrunk;
        }
    }

    if (file) {
        fclose(file);
    }
    // If your lexer keeps a pointer to source_code but does NOT own it,
    // you should destroy the lexer before freeing source_code.
    // Example (if such a function exists):
    // destroy_lexer(lexer);

    free(source_code);

    return tokens;

error:
    // Clean up on failure
    if (tokens) {
        for (size_t i = 0; i < count; ++i) {
            if (tokens[i]) {
                free_token(tokens[i]);
            }
        }
        free(tokens);
    }

    if (lexer) {
        // destroy_lexer(lexer); // if available
    }

    if (source_code) {
        free(source_code);
    }

    if (file) {
        fclose(file);
    }

    return NULL;
}

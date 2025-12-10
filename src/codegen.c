// For utils to go alongside codgen for specific architectures and OSes

#include "codegen.h"

Lexer *convertToLogic(char* input_file) {
    Lexer *lexer = NULL;
    printf("Converting %s to logic representation...\n", input_file);
    
    // read file
    FILE *file = fopen(input_file, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", input_file);
        goto out;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *source_code = malloc(file_size + 1);

    if (source_code == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        goto out;
    }

    lexer = create_lexer(source_code);

out:
    fclose(file);
    free(source_code);
    return lexer;
}
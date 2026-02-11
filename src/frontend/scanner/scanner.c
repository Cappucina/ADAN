#include <stdlib.h>
#include <string.h>

#include "scanner.h"

Scanner* scanner_init(char* source) {
    Scanner* scanner = (Scanner*)calloc(1, sizeof(Scanner));
    if (!scanner) {
        printf("No memory left to allocate for a new scanner! (Error)");
        return NULL;
    }
    scanner->position = 0;
    scanner->source = source;
    scanner->length = strlen(source);
    scanner->start = 0;
    scanner->column = 1;
    scanner->line = 1;
    return scanner;
}

void scanner_free(Scanner* scanner) {
    if (!scanner) {
        printf("No scanner provided; nothing to free! (Error)");
        return;
    }
    free(scanner);
}

// Inline functions and stuff



// Actual scanning logic

Token* scanner_scan(Scanner* scanner) {

}
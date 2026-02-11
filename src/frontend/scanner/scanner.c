#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "scanner.h"

Scanner *scanner_init(char *source)
{
    Scanner *scanner = (Scanner *)calloc(1, sizeof(Scanner));
    if (!scanner)
    {
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

void scanner_free(Scanner *scanner)
{
    if (!scanner)
    {
        printf("No scanner provided; nothing to free! (Error)");
        return;
    }
    free(scanner);
}

// Inline functions and stuff

char peek(Scanner *scanner)
{
    if (scanner->position >= scanner->length)
        return '\0';
    return scanner->source[scanner->position];
}

char peek_next(Scanner *scanner)
{
    if (scanner->position + 1 >= scanner->length)
        return '\0';
    return scanner->source[scanner->position + 1];
}

char advance(Scanner *scanner)
{
    if (scanner->position >= scanner->length)
        return '\0';
    char curr = scanner->source[scanner->position++];
    if (curr == '\n')
    {
        scanner->line++;
        scanner->column = 1;
    }
    else
    {
        scanner->column++;
    }
    return curr;
}

bool is_at_end(Scanner *scanner)
{
    return scanner->position >= scanner->length;
}

// @todo Implement `is_alpha`, `is_digit`, `is_alphanumeric`, `is_whitespace`, `make_token`, `scan_next_token`, `is_keyword`, `check_keyword`, `scan_next_token`

void skip_spaces(Scanner *scanner)
{
    while (1)
    {
        char curr = peek(scanner);
        if (curr == ' ' || curr == '\t' || curr == '\r' || curr == '\n')
        {
            advance(scanner);
        }
        else
        {
            break;
        }
    }
}

// Actual scanning logic

Token *scanner_scan(Scanner *scanner)
{
}
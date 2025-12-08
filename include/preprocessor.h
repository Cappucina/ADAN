#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

typedef struct {
    char** included_files;
    size_t included_count;
    
} Preprocessor;

//
//  Init Preprocessor
//
Preprocessor* create_preprocessor();

//
//  Process includes recursively
//
char* preprocess_file(Preprocessor* pp, const char* path);

//
//  Load file content to string
//
char* load_file(const char* path);

//
//  Check if a file has already been included
//
bool is_file_included(Preprocessor* pp, const char* path);

//
//  Mark a file as included
//
void mark_as_included(Preprocessor* pp, const char* path);

//
//  Free Preprocessor resources
//
void free_preprocessor(Preprocessor* pp);

// 
char* get_package_path(const char* package_name);

#endif
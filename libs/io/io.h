#ifndef ADAN_IO_H
#define ADAN_IO_H

#include <stddef.h>

void adn_println(const char* message);

void adn_flush(void);

void adn_write_file(const char* path, const char* content);

char* adn_read_file(const char* path);

char* adn_input(const char* prompt);

void adn_errorln(const char* fmt, ...);

#endif

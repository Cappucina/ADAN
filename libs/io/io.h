#ifndef ADAN_IO_H
#define ADAN_IO_H

#include <stddef.h>

void adn_println(const char* message);

char* adn_input(const char* prompt);

void adn_errorln(const char* fmt, ...);

#endif

#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

int64_t adn_powi(int64_t base, int64_t exp);

char* adn_strconcat(const char* s1, const char* s2);

char* adn_i32_to_string(int64_t val);

int64_t adn_string_to_i32(const char* s);

#endif

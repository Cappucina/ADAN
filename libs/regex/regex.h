#ifndef ADAN_REGEX_H
#define ADAN_REGEX_H

#include <stdint.h>

int64_t adn_regex_valid(const char* pattern);

char* adn_regex_escape(const char* text);

char* adn_regex_compile(const char* pattern);

int64_t adn_regex_matches(const char* pattern, const char* text);

int64_t adn_regex_find(const char* pattern, const char* text);

int64_t adn_regex_contains(const char* pattern, const char* text);

int64_t adn_regex_starts_with(const char* pattern, const char* text);

int64_t adn_regex_ends_with(const char* pattern, const char* text);

char* adn_regex_replace(const char* pattern, const char* text, const char* replacement);

char* adn_regex_replace_all(const char* pattern, const char* text,
	                        const char* replacement);

void* adn_regex_split(const char* pattern, const char* text);

void* adn_array_create(void);

void adn_array_push_string(void* array, const char* value);

#endif
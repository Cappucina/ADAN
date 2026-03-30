#ifndef ADAN_COLLECTIONS_OBJECT_H
#define ADAN_COLLECTIONS_OBJECT_H

#include <stdint.h>

int64_t adn_collections_object_has(void* object, const char* key);

int64_t adn_collections_object_get_i64(void* object, const char* key);

double adn_collections_object_get_f64(void* object, const char* key);

char* adn_collections_object_get_string(void* object, const char* key);

void* adn_collections_object_get_ptr(void* object, const char* key);

void adn_collections_object_set_i64(void* object, const char* key, int64_t value);

void adn_collections_object_set_f64(void* object, const char* key, double value);

void adn_collections_object_set_string(void* object, const char* key, const char* value);

void adn_collections_object_set_ptr(void* object, const char* key, void* value);

#endif
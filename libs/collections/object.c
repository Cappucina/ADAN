#include "object.h"

int64_t adn_object_has(void* object, const char* key);

int64_t adn_object_get_i64(void* object, const char* key);

double adn_object_get_f64(void* object, const char* key);

char* adn_object_get_string(void* object, const char* key);

void* adn_object_get_ptr(void* object, const char* key);

void adn_object_set_i64(void* object, const char* key, int64_t value);

void adn_object_set_f64(void* object, const char* key, double value);

void adn_object_set_string(void* object, const char* key, const char* value);

void adn_object_set_ptr(void* object, const char* key, void* value);

int64_t adn_collections_object_has(void* object, const char* key)
{
	return adn_object_has(object, key);
}

int64_t adn_collections_object_get_i64(void* object, const char* key)
{
	return adn_object_get_i64(object, key);
}

double adn_collections_object_get_f64(void* object, const char* key)
{
	return adn_object_get_f64(object, key);
}

char* adn_collections_object_get_string(void* object, const char* key)
{
	return adn_object_get_string(object, key);
}

void* adn_collections_object_get_ptr(void* object, const char* key)
{
	return adn_object_get_ptr(object, key);
}

void adn_collections_object_set_i64(void* object, const char* key, int64_t value)
{
	adn_object_set_i64(object, key, value);
}

void adn_collections_object_set_f64(void* object, const char* key, double value)
{
	adn_object_set_f64(object, key, value);
}

void adn_collections_object_set_string(void* object, const char* key, const char* value)
{
	adn_object_set_string(object, key, value);
}

void adn_collections_object_set_ptr(void* object, const char* key, void* value)
{
	adn_object_set_ptr(object, key, value);
}
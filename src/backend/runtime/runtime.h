#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

int64_t adn_powi(int64_t base, int64_t exp);

char* adn_strconcat(const char* s1, const char* s2);

char* adn_i32_to_string(int64_t val);

int64_t adn_string_to_i32(const char* s);

char* adn_f64_to_string(double val);

double adn_string_to_f64(const char* s);

int64_t adn_string_length(const char* s);

char* adn_string_char_at(const char* s, int64_t index);

int64_t adn_string_code_at(const char* s, int64_t index);

char* adn_string_from_code(int64_t code);

char* adn_string_format(const char* format, void* args);

void* adn_object_create(void);

void adn_object_set_i64(void* object, const char* key, int64_t value);

void adn_object_set_f64(void* object, const char* key, double value);

void adn_object_set_string(void* object, const char* key, const char* value);

void adn_object_set_ptr(void* object, const char* key, void* value);

int64_t adn_object_get_i64(void* object, const char* key);

double adn_object_get_f64(void* object, const char* key);

char* adn_object_get_string(void* object, const char* key);

void* adn_object_get_ptr(void* object, const char* key);

void* adn_array_create(void);

void adn_array_push_i64(void* array, int64_t value);

int64_t adn_array_pop_i64(void* array);

void adn_array_push_f64(void* array, double value);

double adn_array_pop_f64(void* array);

void adn_array_push_string(void* array, const char* value);

char* adn_array_pop_string(void* array);

void adn_array_push_ptr(void* array, void* value);

void* adn_array_pop_ptr(void* array);

void adn_array_insert_i64(void* array, int64_t index, int64_t value);

void adn_array_insert_f64(void* array, int64_t index, double value);

void adn_array_insert_string(void* array, int64_t index, const char* value);

void adn_array_insert_ptr(void* array, int64_t index, void* value);

void adn_array_clear(void* array);

int64_t adn_array_length(void* array);

void* adn_array_slice(void* array, int64_t start, int64_t end);

int64_t adn_array_get_i64(void* array, int64_t index);

int64_t adn_array_remove_i64(void* array, int64_t index);

double adn_array_get_f64(void* array, int64_t index);

double adn_array_remove_f64(void* array, int64_t index);

char* adn_array_get_string(void* array, int64_t index);

char* adn_array_remove_string(void* array, int64_t index);

void* adn_array_get_ptr(void* array, int64_t index);

void* adn_array_remove_ptr(void* array, int64_t index);

#endif

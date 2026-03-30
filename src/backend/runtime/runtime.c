#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "runtime.h"

int64_t adn_powi(int64_t base, int64_t exp)
{
	if (exp < 0)
	{
		return 0;
	}
	if (exp == 0)
	{
		return 1;
	}

	int64_t result = 1;
	int64_t current_base = base;
	int64_t current_exp = exp;

	while (current_exp > 0)
	{
		if (current_exp & 1)
		{
			result *= current_base;
		}
		current_base *= current_base;
		current_exp >>= 1;
	}

	return result;
}

char* adn_strconcat(const char* s1, const char* s2)
{
	if (!s1 || !s2)
	{
		return NULL;
	}

	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	char* result = (char*)malloc(len1 + len2 + 1);

	if (!result)
	{
		return NULL;
	}

	strcpy(result, s1);
	strcat(result, s2);

	return result;
}

char* adn_i32_to_string(int64_t val)
{
	char buf[32];
	int len = snprintf(buf, sizeof(buf), "%lld", (long long)val);
	if (len < 0)
	{
		return NULL;
	}
	char* result = (char*)malloc((size_t)len + 1);
	if (!result)
	{
		return NULL;
	}
	memcpy(result, buf, (size_t)len + 1);
	return result;
}

int64_t adn_string_to_i32(const char* s)
{
	if (!s)
	{
		return 0;
	}
	return (int64_t)strtoll(s, NULL, 10);
}

char* adn_f64_to_string(double val)
{
	char buf[64];
	int len = snprintf(buf, sizeof(buf), "%g", val);
	if (len < 0)
	{
		return NULL;
	}
	char* result = (char*)malloc((size_t)len + 1);
	if (!result)
	{
		return NULL;
	}
	memcpy(result, buf, (size_t)len + 1);
	return result;
}

double adn_string_to_f64(const char* s)
{
	if (!s)
	{
		return 0.0;
	}
	return strtod(s, NULL);
}

int64_t adn_string_length(const char* s)
{
	if (!s)
	{
		return 0;
	}
	return (int64_t)strlen(s);
}

char* adn_string_char_at(const char* s, int64_t index)
{
	if (!s || index < 0)
	{
		return strdup("");
	}

	size_t length = strlen(s);
	if ((size_t)index >= length)
	{
		return strdup("");
	}

	char* result = (char*)malloc(2);
	if (!result)
	{
		return NULL;
	}
	result[0] = s[index];
	result[1] = '\0';
	return result;
}

int64_t adn_string_code_at(const char* s, int64_t index)
{
	if (!s || index < 0)
	{
		return -1;
	}

	size_t length = strlen(s);
	if ((size_t)index >= length)
	{
		return -1;
	}

	return (unsigned char)s[index];
}

char* adn_string_from_code(int64_t code)
{
	if (code < 0 || code > 255)
	{
		return strdup("");
	}

	char* result = (char*)malloc(2);
	if (!result)
	{
		return NULL;
	}
	result[0] = (char)code;
	result[1] = '\0';
	return result;
}

typedef struct
{
	char* data;
	size_t length;
	size_t capacity;
} AdnStringBuilder;

static void adn_builder_init(AdnStringBuilder* builder)
{
	if (!builder)
	{
		return;
	}
	builder->capacity = 64;
	builder->length = 0;
	builder->data = calloc(builder->capacity, 1);
}

static void adn_builder_reserve(AdnStringBuilder* builder, size_t additional)
{
	if (!builder)
	{
		return;
	}
	if (builder->length + additional + 1 <= builder->capacity)
	{
		return;
	}
	size_t next_capacity = builder->capacity == 0 ? 64 : builder->capacity;
	while (builder->length + additional + 1 > next_capacity)
	{
		next_capacity *= 2;
	}
	char* resized = realloc(builder->data, next_capacity);
	if (!resized)
	{
		return;
	}
	builder->data = resized;
	builder->capacity = next_capacity;
}

static void adn_builder_append_n(AdnStringBuilder* builder, const char* text, size_t length)
{
	if (!builder || !text)
	{
		return;
	}
	adn_builder_reserve(builder, length);
	memcpy(builder->data + builder->length, text, length);
	builder->length += length;
	builder->data[builder->length] = '\0';
}

static void adn_builder_append(AdnStringBuilder* builder, const char* text)
{
	if (!text)
	{
		return;
	}
	adn_builder_append_n(builder, text, strlen(text));
}

static void adn_builder_append_char(AdnStringBuilder* builder, char ch)
{
	adn_builder_reserve(builder, 1);
	builder->data[builder->length++] = ch;
	builder->data[builder->length] = '\0';
}

static char* adn_builder_finish(AdnStringBuilder* builder)
{
	if (!builder)
	{
		return NULL;
	}
	if (!builder->data)
	{
		return strdup("");
	}
	char* result = builder->data;
	builder->data = NULL;
	builder->length = 0;
	builder->capacity = 0;
	return result;
}

typedef enum
{
	ADN_VALUE_I64,
	ADN_VALUE_F64,
	ADN_VALUE_STRING,
	ADN_VALUE_PTR
} AdnValueKind;

typedef struct
{
	AdnValueKind kind;
	union
	{
		int64_t i64;
		double f64;
		char* string;
		void* ptr;
	} data;
} AdnValue;

typedef struct
{
	char* key;
	AdnValue value;
} AdnObjectEntry;

typedef struct
{
	AdnObjectEntry* entries;
	size_t count;
	size_t capacity;
} AdnObject;

typedef struct
{
	AdnValue* items;
	size_t count;
	size_t capacity;
} AdnArray;

static void adn_value_release(AdnValue* value)
{
	if (!value)
	{
		return;
	}
	if (value->kind == ADN_VALUE_STRING)
	{
		free(value->data.string);
		value->data.string = NULL;
	}
}

static AdnValue adn_value_from_i64(int64_t value)
{
	AdnValue wrapped = {0};
	wrapped.kind = ADN_VALUE_I64;
	wrapped.data.i64 = value;
	return wrapped;
}

static AdnValue adn_value_from_f64(double value)
{
	AdnValue wrapped = {0};
	wrapped.kind = ADN_VALUE_F64;
	wrapped.data.f64 = value;
	return wrapped;
}

static AdnValue adn_value_from_string(const char* value)
{
	AdnValue wrapped = {0};
	wrapped.kind = ADN_VALUE_STRING;
	wrapped.data.string = value ? strdup(value) : strdup("");
	return wrapped;
}

static AdnValue adn_value_from_ptr(void* value)
{
	AdnValue wrapped = {0};
	wrapped.kind = ADN_VALUE_PTR;
	wrapped.data.ptr = value;
	return wrapped;
}

static AdnValue adn_value_clone(const AdnValue* value)
{
	if (!value)
	{
		return adn_value_from_i64(0);
	}
	if (value->kind == ADN_VALUE_STRING)
	{
		return adn_value_from_string(value->data.string);
	}
	if (value->kind == ADN_VALUE_F64)
	{
		return adn_value_from_f64(value->data.f64);
	}
	if (value->kind == ADN_VALUE_PTR)
	{
		return adn_value_from_ptr(value->data.ptr);
	}
	return adn_value_from_i64(value->data.i64);
}

static void adn_object_reserve(AdnObject* object, size_t needed)
{
	if (!object || object->capacity >= needed)
	{
		return;
	}
	size_t next_capacity = object->capacity == 0 ? 4 : object->capacity;
	while (next_capacity < needed)
	{
		next_capacity *= 2;
	}
	AdnObjectEntry* resized = realloc(object->entries, sizeof(AdnObjectEntry) * next_capacity);
	if (!resized)
	{
		return;
	}
	object->entries = resized;
	object->capacity = next_capacity;
}

static AdnArray* adn_array_cast(void* array)
{
	return (AdnArray*)array;
}

static char* adn_value_to_string_spec(const AdnValue* value, char spec)
{
	char buffer[128];
	if (!value)
	{
		return strdup("");
	}
	switch (spec)
	{
		case 'd':
		case 'i':
		{
			long long integer_value = 0;
			if (value->kind == ADN_VALUE_F64)
			{
				integer_value = (long long)value->data.f64;
			}
			else if (value->kind == ADN_VALUE_STRING)
			{
				integer_value =
				    value->data.string ? strtoll(value->data.string, NULL, 10) : 0;
			}
			else if (value->kind == ADN_VALUE_PTR)
			{
				integer_value = (long long)(intptr_t)value->data.ptr;
			}
			else
			{
				integer_value = (long long)value->data.i64;
			}
			snprintf(buffer, sizeof(buffer), "%lld", integer_value);
			return strdup(buffer);
		}
		case 'u':
		{
			unsigned long long integer_value = 0;
			if (value->kind == ADN_VALUE_F64)
			{
				integer_value = (unsigned long long)value->data.f64;
			}
			else if (value->kind == ADN_VALUE_STRING)
			{
				integer_value =
				    value->data.string ? strtoull(value->data.string, NULL, 10) : 0;
			}
			else if (value->kind == ADN_VALUE_PTR)
			{
				integer_value = (unsigned long long)(uintptr_t)value->data.ptr;
			}
			else
			{
				integer_value = (unsigned long long)value->data.i64;
			}
			snprintf(buffer, sizeof(buffer), "%llu", integer_value);
			return strdup(buffer);
		}
		case 'f':
		{
			double float_value = 0.0;
			if (value->kind == ADN_VALUE_I64)
			{
				float_value = (double)value->data.i64;
			}
			else if (value->kind == ADN_VALUE_STRING)
			{
				float_value =
				    value->data.string ? strtod(value->data.string, NULL) : 0.0;
			}
			else if (value->kind == ADN_VALUE_F64)
			{
				float_value = value->data.f64;
			}
			snprintf(buffer, sizeof(buffer), "%g", float_value);
			return strdup(buffer);
		}
		case 'c':
		{
			char out[2] = {0, 0};
			if (value->kind == ADN_VALUE_STRING && value->data.string &&
			    value->data.string[0] != '\0')
			{
				out[0] = value->data.string[0];
			}
			else if (value->kind == ADN_VALUE_F64)
			{
				out[0] = (char)((int)value->data.f64);
			}
			else if (value->kind == ADN_VALUE_PTR)
			{
				out[0] = (char)(intptr_t)value->data.ptr;
			}
			else
			{
				out[0] = (char)value->data.i64;
			}
			return strdup(out);
		}
		case 'p':
		{
			void* pointer_value = NULL;
			if (value->kind == ADN_VALUE_PTR)
			{
				pointer_value = value->data.ptr;
			}
			else if (value->kind == ADN_VALUE_I64)
			{
				pointer_value = (void*)(intptr_t)value->data.i64;
			}
			snprintf(buffer, sizeof(buffer), "%p", pointer_value);
			return strdup(buffer);
		}
		case 's':
		default:
			if (value->kind == ADN_VALUE_STRING)
			{
				return strdup(value->data.string ? value->data.string : "");
			}
			if (value->kind == ADN_VALUE_F64)
			{
				return adn_f64_to_string(value->data.f64);
			}
			if (value->kind == ADN_VALUE_PTR)
			{
				snprintf(buffer, sizeof(buffer), "%p", value->data.ptr);
				return strdup(buffer);
			}
			return adn_i32_to_string(value->data.i64);
	}
}

char* adn_string_format(const char* format, void* args)
{
	AdnArray* values = adn_array_cast(args);
	AdnStringBuilder builder = {0};
	adn_builder_init(&builder);
	if (!format)
	{
		return adn_builder_finish(&builder);
	}
	size_t arg_index = 0;
	for (size_t i = 0; format[i] != '\0'; i++)
	{
		if (format[i] != '%')
		{
			adn_builder_append_char(&builder, format[i]);
			continue;
		}
		char spec = format[i + 1];
		if (spec == '\0')
		{
			adn_builder_append_char(&builder, '%');
			break;
		}
		if (spec == '%')
		{
			adn_builder_append_char(&builder, '%');
			i++;
			continue;
		}
		AdnValue* value =
		    (values && arg_index < values->count) ? &values->items[arg_index++] : NULL;
		char* replacement = adn_value_to_string_spec(value, spec);
		if (replacement)
		{
			adn_builder_append(&builder, replacement);
			free(replacement);
		}
		i++;
	}
	return adn_builder_finish(&builder);
}

static AdnObject* adn_object_cast(void* object)
{
	return (AdnObject*)object;
}

static AdnObjectEntry* adn_object_find_entry(AdnObject* object, const char* key)
{
	if (!object || !key)
	{
		return NULL;
	}
	for (size_t i = 0; i < object->count; i++)
	{
		if (object->entries[i].key && strcmp(object->entries[i].key, key) == 0)
		{
			return &object->entries[i];
		}
	}
	return NULL;
}

static void adn_object_store_value(AdnObject* object, const char* key, AdnValue value)
{
	if (!object || !key)
	{
		adn_value_release(&value);
		return;
	}
	AdnObjectEntry* entry = adn_object_find_entry(object, key);
	if (entry)
	{
		adn_value_release(&entry->value);
		entry->value = value;
		return;
	}
	adn_object_reserve(object, object->count + 1);
	if (object->count >= object->capacity)
	{
		adn_value_release(&value);
		return;
	}
	object->entries[object->count].key = strdup(key);
	object->entries[object->count].value = value;
	object->count++;
}

static void adn_array_reserve(AdnArray* array, size_t needed)
{
	if (!array || array->capacity >= needed)
	{
		return;
	}
	size_t next_capacity = array->capacity == 0 ? 4 : array->capacity;
	while (next_capacity < needed)
	{
		next_capacity *= 2;
	}
	AdnValue* resized = realloc(array->items, sizeof(AdnValue) * next_capacity);
	if (!resized)
	{
		return;
	}
	array->items = resized;
	array->capacity = next_capacity;
}

static void adn_array_push_value(AdnArray* array, AdnValue value)
{
	if (!array)
	{
		adn_value_release(&value);
		return;
	}
	adn_array_reserve(array, array->count + 1);
	if (array->count >= array->capacity)
	{
		adn_value_release(&value);
		return;
	}
	array->items[array->count++] = value;
}

static AdnValue* adn_array_get_value(AdnArray* array, int64_t index)
{
	if (!array || index < 0 || (size_t)index >= array->count)
	{
		return NULL;
	}
	return &array->items[index];
}

static int64_t adn_array_insert_index(AdnArray* array, int64_t index)
{
	if (!array)
	{
		return 0;
	}
	if (index < 0)
	{
		return 0;
	}
	if ((size_t)index > array->count)
	{
		return (int64_t)array->count;
	}
	return index;
}

static int64_t adn_array_remove_index(AdnArray* array, int64_t index)
{
	if (!array || index < 0 || (size_t)index >= array->count)
	{
		return -1;
	}
	return index;
}

static void adn_array_insert_value(AdnArray* array, int64_t index, AdnValue value)
{
	if (!array)
	{
		adn_value_release(&value);
		return;
	}
	index = adn_array_insert_index(array, index);
	adn_array_reserve(array, array->count + 1);
	if (array->count >= array->capacity)
	{
		adn_value_release(&value);
		return;
	}
	size_t offset = (size_t)index;
	memmove(&array->items[offset + 1], &array->items[offset],
	        sizeof(AdnValue) * (array->count - offset));
	array->items[offset] = value;
	array->count++;
}

static AdnValue adn_array_take_value(AdnArray* array, int64_t index)
{
	AdnValue empty = adn_value_from_i64(0);
	index = adn_array_remove_index(array, index);
	if (!array || index < 0)
	{
		return empty;
	}
	size_t offset = (size_t)index;
	AdnValue removed = array->items[offset];
	if (offset + 1 < array->count)
	{
		memmove(&array->items[offset], &array->items[offset + 1],
		        sizeof(AdnValue) * (array->count - offset - 1));
	}
	array->count--;
	return removed;
}

void* adn_object_create(void)
{
	return calloc(1, sizeof(AdnObject));
}

void adn_object_set_i64(void* object, const char* key, int64_t value)
{
	adn_object_store_value(adn_object_cast(object), key, adn_value_from_i64(value));
}

void adn_object_set_f64(void* object, const char* key, double value)
{
	adn_object_store_value(adn_object_cast(object), key, adn_value_from_f64(value));
}

void adn_object_set_string(void* object, const char* key, const char* value)
{
	adn_object_store_value(adn_object_cast(object), key, adn_value_from_string(value));
}

void adn_object_set_ptr(void* object, const char* key, void* value)
{
	adn_object_store_value(adn_object_cast(object), key, adn_value_from_ptr(value));
}

int64_t adn_object_has(void* object, const char* key)
{
	return adn_object_find_entry(adn_object_cast(object), key) != NULL;
}

int64_t adn_object_get_i64(void* object, const char* key)
{
	AdnObjectEntry* entry = adn_object_find_entry(adn_object_cast(object), key);
	if (!entry)
	{
		return 0;
	}
	if (entry->value.kind == ADN_VALUE_F64)
	{
		return (int64_t)entry->value.data.f64;
	}
	if (entry->value.kind == ADN_VALUE_STRING)
	{
		return entry->value.data.string ? strtoll(entry->value.data.string, NULL, 10) : 0;
	}
	if (entry->value.kind == ADN_VALUE_PTR)
	{
		return (int64_t)(intptr_t)entry->value.data.ptr;
	}
	return entry->value.data.i64;
}

double adn_object_get_f64(void* object, const char* key)
{
	AdnObjectEntry* entry = adn_object_find_entry(adn_object_cast(object), key);
	if (!entry)
	{
		return 0.0;
	}
	if (entry->value.kind == ADN_VALUE_I64)
	{
		return (double)entry->value.data.i64;
	}
	if (entry->value.kind == ADN_VALUE_STRING)
	{
		return entry->value.data.string ? strtod(entry->value.data.string, NULL) : 0.0;
	}
	return entry->value.data.f64;
}

char* adn_object_get_string(void* object, const char* key)
{
	AdnObjectEntry* entry = adn_object_find_entry(adn_object_cast(object), key);
	if (!entry)
	{
		return strdup("");
	}
	if (entry->value.kind == ADN_VALUE_STRING)
	{
		return entry->value.data.string ? entry->value.data.string : strdup("");
	}
	if (entry->value.kind == ADN_VALUE_I64)
	{
		return adn_i32_to_string(entry->value.data.i64);
	}
	if (entry->value.kind == ADN_VALUE_F64)
	{
		return adn_f64_to_string(entry->value.data.f64);
	}
	return strdup("[object]");
}

void* adn_object_get_ptr(void* object, const char* key)
{
	AdnObjectEntry* entry = adn_object_find_entry(adn_object_cast(object), key);
	if (!entry)
	{
		return NULL;
	}
	if (entry->value.kind == ADN_VALUE_PTR)
	{
		return entry->value.data.ptr;
	}
	return NULL;
}

void* adn_array_create(void)
{
	return calloc(1, sizeof(AdnArray));
}

void adn_array_push_i64(void* array, int64_t value)
{
	adn_array_push_value(adn_array_cast(array), adn_value_from_i64(value));
}

int64_t adn_array_pop_i64(void* array)
{
	AdnValue value = adn_array_take_value(adn_array_cast(array), adn_array_length(array) - 1);
	if (value.kind == ADN_VALUE_F64)
	{
		return (int64_t)value.data.f64;
	}
	if (value.kind == ADN_VALUE_STRING)
	{
		return value.data.string ? strtoll(value.data.string, NULL, 10) : 0;
	}
	if (value.kind == ADN_VALUE_PTR)
	{
		return (int64_t)(intptr_t)value.data.ptr;
	}
	return value.data.i64;
}

void adn_array_push_f64(void* array, double value)
{
	adn_array_push_value(adn_array_cast(array), adn_value_from_f64(value));
}

double adn_array_pop_f64(void* array)
{
	AdnValue value = adn_array_take_value(adn_array_cast(array), adn_array_length(array) - 1);
	if (value.kind == ADN_VALUE_I64)
	{
		return (double)value.data.i64;
	}
	if (value.kind == ADN_VALUE_STRING)
	{
		return value.data.string ? strtod(value.data.string, NULL) : 0.0;
	}
	return value.data.f64;
}

void adn_array_push_string(void* array, const char* value)
{
	adn_array_push_value(adn_array_cast(array), adn_value_from_string(value));
}

char* adn_array_pop_string(void* array)
{
	AdnValue value = adn_array_take_value(adn_array_cast(array), adn_array_length(array) - 1);
	if (value.kind == ADN_VALUE_STRING)
	{
		return value.data.string ? value.data.string : strdup("");
	}
	if (value.kind == ADN_VALUE_I64)
	{
		return adn_i32_to_string(value.data.i64);
	}
	if (value.kind == ADN_VALUE_F64)
	{
		return adn_f64_to_string(value.data.f64);
	}
	return strdup("[array]");
}

void adn_array_push_ptr(void* array, void* value)
{
	adn_array_push_value(adn_array_cast(array), adn_value_from_ptr(value));
}

void* adn_array_pop_ptr(void* array)
{
	AdnValue value = adn_array_take_value(adn_array_cast(array), adn_array_length(array) - 1);
	return value.kind == ADN_VALUE_PTR ? value.data.ptr : NULL;
}

void adn_array_insert_i64(void* array, int64_t index, int64_t value)
{
	adn_array_insert_value(adn_array_cast(array), index, adn_value_from_i64(value));
}

void adn_array_insert_f64(void* array, int64_t index, double value)
{
	adn_array_insert_value(adn_array_cast(array), index, adn_value_from_f64(value));
}

void adn_array_insert_string(void* array, int64_t index, const char* value)
{
	adn_array_insert_value(adn_array_cast(array), index, adn_value_from_string(value));
}

void adn_array_insert_ptr(void* array, int64_t index, void* value)
{
	adn_array_insert_value(adn_array_cast(array), index, adn_value_from_ptr(value));
}

void adn_array_clear(void* array)
{
	AdnArray* inner = adn_array_cast(array);
	if (!inner)
	{
		return;
	}
	for (size_t i = 0; i < inner->count; i++)
	{
		adn_value_release(&inner->items[i]);
	}
	inner->count = 0;
}

int64_t adn_array_length(void* array)
{
	AdnArray* inner = adn_array_cast(array);
	return inner ? (int64_t)inner->count : 0;
}

void* adn_array_slice(void* array, int64_t start, int64_t end)
{
	AdnArray* inner = adn_array_cast(array);
	AdnArray* slice = calloc(1, sizeof(AdnArray));
	if (!slice)
	{
		return NULL;
	}
	if (!inner)
	{
		return slice;
	}
	if (start < 0)
	{
		start = 0;
	}
	if (end < 0)
	{
		end = 0;
	}
	if ((size_t)start > inner->count)
	{
		start = (int64_t)inner->count;
	}
	if ((size_t)end > inner->count)
	{
		end = (int64_t)inner->count;
	}
	if (end < start)
	{
		end = start;
	}
	for (int64_t i = start; i < end; i++)
	{
		adn_array_push_value(slice, adn_value_clone(&inner->items[i]));
	}
	return slice;
}

int64_t adn_array_get_i64(void* array, int64_t index)
{
	AdnValue* value = adn_array_get_value(adn_array_cast(array), index);
	if (!value)
	{
		return 0;
	}
	if (value->kind == ADN_VALUE_F64)
	{
		return (int64_t)value->data.f64;
	}
	if (value->kind == ADN_VALUE_STRING)
	{
		return value->data.string ? strtoll(value->data.string, NULL, 10) : 0;
	}
	if (value->kind == ADN_VALUE_PTR)
	{
		return (int64_t)(intptr_t)value->data.ptr;
	}
	return value->data.i64;
}

int64_t adn_array_remove_i64(void* array, int64_t index)
{
	AdnValue value = adn_array_take_value(adn_array_cast(array), index);
	if (value.kind == ADN_VALUE_F64)
	{
		return (int64_t)value.data.f64;
	}
	if (value.kind == ADN_VALUE_STRING)
	{
		return value.data.string ? strtoll(value.data.string, NULL, 10) : 0;
	}
	if (value.kind == ADN_VALUE_PTR)
	{
		return (int64_t)(intptr_t)value.data.ptr;
	}
	return value.data.i64;
}

double adn_array_get_f64(void* array, int64_t index)
{
	AdnValue* value = adn_array_get_value(adn_array_cast(array), index);
	if (!value)
	{
		return 0.0;
	}
	if (value->kind == ADN_VALUE_I64)
	{
		return (double)value->data.i64;
	}
	if (value->kind == ADN_VALUE_STRING)
	{
		return value->data.string ? strtod(value->data.string, NULL) : 0.0;
	}
	return value->data.f64;
}

double adn_array_remove_f64(void* array, int64_t index)
{
	AdnValue value = adn_array_take_value(adn_array_cast(array), index);
	if (value.kind == ADN_VALUE_I64)
	{
		return (double)value.data.i64;
	}
	if (value.kind == ADN_VALUE_STRING)
	{
		return value.data.string ? strtod(value.data.string, NULL) : 0.0;
	}
	return value.data.f64;
}

char* adn_array_get_string(void* array, int64_t index)
{
	AdnValue* value = adn_array_get_value(adn_array_cast(array), index);
	if (!value)
	{
		return strdup("");
	}
	if (value->kind == ADN_VALUE_STRING)
	{
		return value->data.string ? value->data.string : strdup("");
	}
	if (value->kind == ADN_VALUE_I64)
	{
		return adn_i32_to_string(value->data.i64);
	}
	if (value->kind == ADN_VALUE_F64)
	{
		return adn_f64_to_string(value->data.f64);
	}
	return strdup("[array]");
}

char* adn_array_remove_string(void* array, int64_t index)
{
	AdnValue value = adn_array_take_value(adn_array_cast(array), index);
	if (value.kind == ADN_VALUE_STRING)
	{
		return value.data.string ? value.data.string : strdup("");
	}
	if (value.kind == ADN_VALUE_I64)
	{
		return adn_i32_to_string(value.data.i64);
	}
	if (value.kind == ADN_VALUE_F64)
	{
		return adn_f64_to_string(value.data.f64);
	}
	return strdup("[array]");
}

void* adn_array_get_ptr(void* array, int64_t index)
{
	AdnValue* value = adn_array_get_value(adn_array_cast(array), index);
	if (!value || value->kind != ADN_VALUE_PTR)
	{
		return NULL;
	}
	return value->data.ptr;
}

void* adn_array_remove_ptr(void* array, int64_t index)
{
	AdnValue value = adn_array_take_value(adn_array_cast(array), index);
	return value.kind == ADN_VALUE_PTR ? value.data.ptr : NULL;
}
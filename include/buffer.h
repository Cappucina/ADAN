#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    void* data;
    size_t count;
    size_t capacity;
    size_t element_size;
} Buffer;

Buffer* buffer_create(size_t element_size);

void buffer_push(Buffer* buf, const void* element);

void* buffer_get(Buffer* buf, size_t index);

void buffer_free(Buffer* buf);

#endif
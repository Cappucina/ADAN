#include "../../include/buffer.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

Buffer* buffer_create(size_t element_size)
{
    Buffer* buf = (Buffer*)malloc(sizeof(Buffer));
    if (!buf) return NULL;

    buf->element_size = element_size;
    buf->capacity = 1024;
    buf->count = 0;
    buf->data = malloc(buf->capacity * element_size);

    if (!buf->data)
    {
        free(buf);
        return NULL;
    }

    return buf;
}

void buffer_push(Buffer* buf, const void* element)
{
    if (!buf || !element) return;

    if (buf->count == buf->capacity)
    {
        buf->capacity *= 2;
        void* new_data = realloc(buf->data, buf->capacity * buf->element_size);
        if (!new_data) return;
        buf->data = new_data;
    }

    memcpy((char*)buf->data + (buf->count * buf->element_size), element, buf->element_size);
    buf->count++;
}

void* buffer_get(Buffer* buf, size_t index)
{
    if (!buf || index >= buf->count) return NULL;
    return (char*)buf->data + (index * buf->element_size);
}

void buffer_free(Buffer* buf)
{
    if (!buf) return;
    free(buf->data);
    free(buf);
}

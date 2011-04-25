#ifndef MTN_ARRAY_HEAP_H
#define MTN_ARRAY_HEAP_H

#include <stdbool.h>

#include "utarray.h"

typedef struct
{
    UT_array* array;
    bool (*is_greater)(void* a, void* b);
} array_heap;

void heap_build(array_heap heap);
void heap_push(array_heap heap, void* node);
void heap_pop(array_heap heap);

void* heap_first(array_heap heap);

bool heap_is_empty(array_heap heap);

#endif

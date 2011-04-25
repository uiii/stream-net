#include "array_heap.h"

#include <string.h>
#include <err.h>

#include <stdio.h> // TODO smazat

static
void
_swap_ptr(void* a, void* b, size_t ptr_size)
{
    void* tmp = malloc(ptr_size);

    if(tmp == NULL)
    {
        err(1, "Fatal error: cannot allocate memory");
    }

    memcpy(tmp, a, ptr_size);
    memcpy(a, b, ptr_size);
    memcpy(b, tmp, ptr_size);
    free(tmp);
}

static
void
heapify(array_heap heap, int index)
{
    //printf("heapify [%d] = %d\n", index, *(int*) utarray_eltptr(heap.array, index));

    unsigned int array_len = utarray_len(heap.array);
    if(index < 0 || index > array_len - 1)
    {
        return;
    }

    int largest_index = index;

    int left_index = 2 * index + 1;
    int right_index = 2 * index + 2;

    if(left_index < array_len)
    {
        void* largest = utarray_eltptr(heap.array, largest_index);
        void* left = utarray_eltptr(heap.array, left_index);

        if(heap.is_greater(left, largest))
        {
            largest_index = left_index;
        }
    }

    if(right_index < array_len)
    {
        void* largest = utarray_eltptr(heap.array, largest_index);
        void* right = utarray_eltptr(heap.array, right_index);

        if(heap.is_greater(right, largest))
        {
            largest_index = right_index;
        }
    }

    if(largest_index != index)
    {
        void* largest = utarray_eltptr(heap.array, largest_index);
        void* current = utarray_eltptr(heap.array, index);

        /*printf("\tswap [%d] = %d <-> [%d] = %d\n",
                largest_index, *(int*) largest,
                index, *(int*) current);*/

        _swap_ptr(largest, current, heap.array->icd->sz);

        /*size_t elem_size = sizeof(heap.array->icd->sz)
        void* tmp = malloc(elem_size);

        if(tmp == NULL)
        {
            err(1, "Fatal error: cannot allocate memory");
        }

        memcpy(tmp, largest, elem_size);
        memcpy(largest, index, elem_size);
        memcpy(index, tmp, elem_size);
        free(tmp);*/

        heapify(heap, largest_index);
    }
}

void
heap_build(array_heap heap)
{
    if(heap_is_empty(heap))
    {
        return;
    }
    
    int last_index = utarray_len(heap.array) - 1;

    // integral division (floor equivalent)
    int last_parent_index = ((last_index - 1) / 2);

    for(int i = last_parent_index; i >= 0; i--)
    {
        heapify(heap, i);
    }
}

void
heap_push(array_heap heap, void* elem)
{
    utarray_push_back(heap.array, elem);

    int elem_index = utarray_len(heap.array) - 1;

    while(elem_index > 0)
    {
        elem = utarray_eltptr(heap.array, elem_index);

        // integral division (floor equivalent)
        int parent_index = (elem_index - 1) / 2;

        void* parent = utarray_eltptr(heap.array, parent_index);

        if(heap.is_greater(parent, elem))
        {
            break;
        }

        /*printf("swap [%d] = %d <-> [%d] = %d\n",
                parent_index, *(int*) parent,
                elem_index, *(int*) elem);*/

        _swap_ptr(parent, elem, heap.array->icd->sz);

        /*printf("\t=> [%d] = %d, [%d] = %d\n",
                parent_index, *(int*) utarray_eltptr(heap.array, parent_index),
                elem_index, *(int*) utarray_eltptr(heap.array, elem_index));*/

        elem_index = parent_index;
    }
}

void
heap_pop(array_heap heap)
{
    if(heap_is_empty(heap))
    {
        return;
    }

    void* front = utarray_front(heap.array);
    void* back = utarray_back(heap.array);

    memmove(front, back, heap.array->icd->sz);

    utarray_pop_back(heap.array);

    if(! heap_is_empty(heap))
    {
        heapify(heap, 0);
    }
}

void*
heap_first(array_heap heap)
{
    return utarray_front(heap.array);
}

bool
heap_is_empty(array_heap heap)
{
    if(utarray_len(heap.array))
    {
        return false;
    }

    return true;
}

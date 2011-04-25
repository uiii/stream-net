
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "utarray.h"
#include "array_heap.h"

bool is_greater(void* a, void* b)
{
    return *(int*)a < *(int*)b;
}

int main()
{
    array_heap heap;
    heap.array = NULL;
    heap.is_greater = is_greater;

    UT_array* array;
    UT_icd icd = {sizeof(int), NULL, NULL, NULL};
    utarray_new(array,&icd);

    for(int i = 0; i < 20; i++)
    {
        int* elem = (int*) malloc(sizeof(int));
        *elem = rand() % 1000;

        utarray_push_back(array, elem);
    }

    int* p;

    for(p=(int*)utarray_front(array);
        p!=NULL;
        p=(int*)utarray_next(array,p))
    {
        printf("%d\n", *p);
    }
    
    printf("------\n");

    heap.array = array;
    heap_build(heap);

    for(p=(int*)utarray_front(array);
        p!=NULL;
        p=(int*)utarray_next(array,p))
    {
        printf("%d\n", *p);
    }

    printf("------\n");

    int a = 1000;
    heap_push(heap, &a); 

    for(p=(int*)utarray_front(array);
        p!=NULL;
        p=(int*)utarray_next(array,p))
    {
        printf("%d\n", *p);
    }

    printf("------\n");

    heap_pop(heap);

    for(p=(int*)utarray_front(array);
        p!=NULL;
        p=(int*)utarray_next(array,p))
    {
        printf("%d\n", *p);
    }

}

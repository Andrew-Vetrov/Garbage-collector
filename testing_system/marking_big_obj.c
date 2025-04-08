#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../allocator/allocator.h"
#include "../gc.h"
#include "../scanner/marking.h"

#define SIZE 1000

extern size_t START_BIG_ALLOCATOR_HEAP;

int main() {
    void **array = (void **)gc_malloc(SIZE * sizeof(void *));
    assert(array != NULL);
    for (size_t i = 0; i < SIZE; i++) {
        array[i] = (void *)gc_malloc(4096);
        assert(array[i]);
    }

    full_marking();

    assert(is_marked((Object)array));
    for (size_t i = 0; i < SIZE; i++) {
        assert(is_marked((Object)array[i]));
    }

    return 0;
}

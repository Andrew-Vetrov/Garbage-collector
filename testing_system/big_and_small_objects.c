#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../allocator/allocator.h"
#include "../scanner/marking.h"

#define SIZE 1000

extern size_t START_ALLOCATOR_HEAP;
extern size_t START_BIG_ALLOCATOR_HEAP;

typedef struct block {
    int number;
    int *pointer;
} Block;

int main() {
    Block **array = (Block **)gc_malloc(SIZE * sizeof(Block *));

    assert(array != NULL);

    for (size_t i = 0; i < SIZE; i++) {
        array[i] = (Block *)gc_malloc(sizeof(Block));
        assert(array[i] != NULL);
    }

    full_marking();

    assert(is_marked(array) == true);

    for (size_t i = 0; i < SIZE; i++) {
        assert(is_marked(array[i]) == true);
    }
    
    return 0;
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../allocator/allocator.h"

extern size_t START_BIG_ALLOCATOR_HEAP;

int main() {

    // allocation without alignment
    void *p1 = (void *)gc_malloc(2400);
    void *p2 = (void *)gc_malloc(2400);
    void *p3 = (void *)gc_malloc(3000);
    void *p4 = (void *)gc_malloc(3000);
    void *p5 = (void *)gc_malloc(10000);

    // allocation with alignment to 8
    void *p6 = (void *)gc_malloc(9999);
    void *p7 = (void *)gc_malloc(100001);

    // MAX_OBJECT_SIZE (in small heap) + 1 allocated in big heap
    void *p8 = (void *)gc_malloc(2009);

    assert(p1 == START_BIG_ALLOCATOR_HEAP);
    assert(p2 == p1 + 2400);
    assert(p3 == p2 + 2400);
    assert(p4 == p3 + 3000);
    assert(p5 == p4 + 3000);
    assert(p6 == p5 + 10000);
    assert(p7 == p6 + 10000);
    assert(p8 == p7 + 100008);

    return 0;
}

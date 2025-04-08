#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "../allocator/allocator.h"
#include "../gc.h"

void sweep();
void set_bit_by_address(size_t object_addr, unsigned char bit);
size_t get_block_addr(size_t object_address);

void test_placing_after_sweeping() {
    clock_t start, stop;
    size_t objs[6];
    set_bit_by_address(objs[0] = gc_malloc(8), 1);
    set_bit_by_address(objs[1] = gc_malloc(32), 1);
    set_bit_by_address(objs[2] = gc_malloc(1024), 1);
    start = clock();
    sweep();
    stop = clock();
    objs[3] = gc_malloc(8);
    objs[4] = gc_malloc(32);
    objs[5] = gc_malloc(1024);
#ifdef DEBUG
    for (int i = 0; i < 6; i++) {
        printf("%d: %p\n", i, objs[i]);
    }
#endif
    assert(get_block_addr(objs[0]) == get_block_addr(objs[3]));
    assert(get_block_addr(objs[1]) == get_block_addr(objs[4]));
    assert(get_block_addr(objs[2]) == get_block_addr(objs[5]));
    printf("sweep completed in %ldms\n",
           (stop - start) / (CLOCKS_PER_SEC / 1000));
}

int main() { test_placing_after_sweeping(); }

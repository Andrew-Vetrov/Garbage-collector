#include "../allocator/allocator.h"
#include <time.h>
#include <stdio.h>

void sweep();
void set_bit_by_address(size_t object_addr, unsigned char bit);

int main() {
    clock_t start, stop;
    size_t objs[6];
    set_bit_by_address(objs[0] = allocate_new_object(8), 1);
    set_bit_by_address(objs[1] = allocate_new_object(32), 1);
    set_bit_by_address(objs[2] = allocate_new_object(1024), 1);
    start = clock();
    sweep();
    stop = clock(); 
    objs[3] = allocate_new_object(8);
    objs[4] = allocate_new_object(32);
    objs[5] = allocate_new_object(1024);
#ifdef DEBUG
    for (int i = 0; i < 6; i++) {
        printf("%d: %p\n", i, objs[i]);
    }
#endif
    if (((objs[3] - objs[0]) != 8) || ((objs[4] - objs[1]) != 32)
            || ((objs[5] - objs[2]) != 1024)) {
        fprintf(stderr, "objects was placed in different blocks after sweep()");
        return 1;
    }
    printf("sweep completed in %ldms\n", (stop - start) / (CLOCKS_PER_SEC / 1000));
    return 0;
}

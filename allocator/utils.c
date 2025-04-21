#include <stdlib.h>

#include "small-allocator.h"
#include "bitmap.h"

size_t end_rsp_value;

void show_bitmap(size_t object_addr) {
    size_t relative_object_addr = object_addr - get_small_heap_start();
    size_t block_addr = object_addr - (relative_object_addr % BLOCK_SIZE);
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

    unsigned char curr_byte;

    for (int i = 0; i < BITMAP_BYTES_COUNT; i++) {
        printf("byte's index = %2d. bits: ", i);
        curr_byte = *(unsigned char*)(bitmap_addr + i);
        for (int j = 0; j < sizeof(curr_byte) * 8; j++) {
            putchar(((curr_byte >> j) & 1) == 1 ? '1' : '0');
        }
        putchar('\n');
    }
}
#include "bitmap.h"

#include <stdlib.h>

#include "allocator.h"
#include "small-allocator.h"

unsigned char get_bit_by_address(size_t object_addr) {
    size_t object_relative_addr = object_addr - get_small_heap_start();
    size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
    size_t objects_addr = block_addr + BLOCK_HEADER_SIZE;
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

    size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);
    size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

    int object_index =
        (object_addr - objects_addr) / object_size_with_alignment;

    size_t byte_position =
        bitmap_addr + (object_index / (sizeof(unsigned char) * 8));
    size_t bit_position = object_index % (sizeof(unsigned char) * 8);

    return (unsigned char)(((*(unsigned char*)byte_position) >> bit_position) &
                           1);
}

void set_bit_by_address(size_t object_addr, unsigned char bit) {
    size_t object_relative_addr = object_addr - get_small_heap_start();
    size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
    size_t objects_addr = block_addr + BLOCK_HEADER_SIZE;
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

    size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);
    size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

    int object_index =
        (object_addr - objects_addr) / object_size_with_alignment;

    size_t byte_position =
        bitmap_addr + (object_index / (sizeof(unsigned char) * 8));
    size_t bit_position = object_index % (sizeof(unsigned char) * 8);

    if ((((*(unsigned char*)byte_position) >> bit_position) & 1) != bit) {
        *(unsigned char*)byte_position =
            (*(unsigned char*)byte_position) ^ (1 << bit_position);
    }
}

bool is_bitmap_empty(size_t block_addr) {
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);
    size_t curr_bytes_addr = bitmap_addr;

    for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
        if (*(size_t*)curr_bytes_addr != 0) {
            return false;
        }
        curr_bytes_addr += sizeof(size_t);
    }

    return true;
}
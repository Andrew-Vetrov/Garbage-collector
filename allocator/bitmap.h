#include <stdbool.h>
#include <stdlib.h>

#define BITMAP_BYTES_COUNT (64)

unsigned char get_bit_by_address(size_t object_addr);

void set_bit_by_address(size_t object_addr, unsigned char bit);

bool is_bitmap_empty(size_t block_addr);
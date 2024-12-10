#pragma once
#include <stdio.h>

#define GET_BITMAP_ADDR(block_addr) ((block_addr) + (16))
#define GET_OBJECT_SIZE_ADDR(block_addr) ((block_addr) + (8))
#define GET_SLIDER_POSITION_ADDR(block_addr) ((block_addr) + (0))
#define GET_SIZE_WITH_ALIGNMENT(size) \
    ((((size) % (8) == (0)) ? (0) : (8) - ((size) % (8))) + (size))

#define HEAP_SIZE (512 * 1024 * (size_t) 1024)
#define BLOCK_SIZE (4 * (size_t) 1024)
#define BLOCK_HEADER_SIZE (80)
#define MAX_OBJECT_SIZE (2008)
#define OBJECT_SIZE_UPPER_BOUND (MAX_OBJECT_SIZE + 1)
#define BITMAP_BYTES_COUNT (64)
#define BLOCKS_COUNT (HEAP_SIZE / BLOCK_SIZE)

extern size_t end_rsp_value;
extern size_t START_ALLOCATOR_HEAP;
extern size_t END_ALLOCATOR_HEAP;

__attribute__((constructor))
void init_allocator();

__attribute__((destructor))
void destroy_allocator();

size_t allocate_new_object(size_t object_size);

size_t get_object_size_by_address(size_t object_address);

size_t get_block_addr(size_t object_address);

void show_bitmap(size_t object_address);


extern int counter;
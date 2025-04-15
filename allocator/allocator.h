#pragma once
#include <stdbool.h>
#include <stdio.h>

#define GET_BITMAP_ADDR(block_addr) ((block_addr) + (16))
#define GET_OBJECT_SIZE_ADDR(block_addr) ((block_addr) + (8))
#define GET_SLIDER_POSITION_ADDR(block_addr) ((block_addr) + (0))
#define GET_SIZE_WITH_ALIGNMENT(size) \
    ((((size) % (8) == (0)) ? (0) : (8) - ((size) % (8))) + (size))

#ifndef HEAP_SIZE
#define HEAP_SIZE (512 * 1024 * (size_t)1024)
#endif

#define MAX_OBJECT_SIZE (2008)
#define OBJECT_SIZE_UPPER_BOUND (MAX_OBJECT_SIZE + 1)

__attribute__((constructor)) void __init_allocator();

__attribute__((destructor)) void __destroy_allocator();

size_t allocate_new_object(size_t size);

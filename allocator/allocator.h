#pragma once
#include <stdio.h>
#include <stdbool.h>

#define GET_BITMAP_ADDR(block_addr) ((block_addr) + (16))
#define GET_OBJECT_SIZE_ADDR(block_addr) ((block_addr) + (8))
#define GET_SLIDER_POSITION_ADDR(block_addr) ((block_addr) + (0))
#define GET_SIZE_WITH_ALIGNMENT(size) \
    ((((size) % (8) == (0)) ? (0) : (8) - ((size) % (8))) + (size))

#ifndef HEAP_SIZE
#define HEAP_SIZE (512 * 1024 * (size_t) 1024)
#endif

#define BLOCK_SIZE (4 * (size_t) 1024)
#define BLOCK_HEADER_SIZE (80)
#define MAX_OBJECT_SIZE (2008)
#define OBJECT_SIZE_UPPER_BOUND (MAX_OBJECT_SIZE + 1)
#define BITMAP_BYTES_COUNT (64)
#define BLOCKS_COUNT (HEAP_SIZE / BLOCK_SIZE)
#define HEADERS_COUNT (HEAP_SIZE / GET_SIZE_WITH_ALIGNMENT(MAX_OBJECT_SIZE + 1))
#define INVALID_ADDRESS (-1)
#define get_object_addr(object) ((size_t) object)

extern size_t end_rsp_value;

typedef size_t Object;

__attribute__((constructor))
void __init_allocator();

__attribute__((destructor))
void __destroy_allocator();

size_t allocate_new_object(size_t object_size);

size_t get_object_size_by_address(size_t object_address);

size_t get_block_addr(size_t object_address);

void show_bitmap(size_t object_address);

int get_object(size_t object_addr, Object* object);

void mark_object(Object object);

bool is_marked(Object object);

void sweep();
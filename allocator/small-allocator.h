#pragma once
#include <stdlib.h>

#include "allocator.h"

#define BLOCKS_COUNT (HEAP_SIZE / BLOCK_SIZE)
#define BLOCK_SIZE (4 * (size_t)1024)
#define BLOCK_HEADER_SIZE (80)
#define GET_BITMAP_ADDR(block_addr) ((block_addr) + (16))
#define GET_OBJECT_SIZE_ADDR(block_addr) ((block_addr) + (8))
#define GET_SLIDER_POSITION_ADDR(block_addr) ((block_addr) + (0))

typedef struct Node_t {
    size_t block_addr;
    struct Node_t* next_node;
} Node;

void __init_small_allocator();

void __destroy_small_allocator();

void init_header(Node* entry, size_t object_size);

void fill_all_bitmaps_with_zeros();

Node* allocate_new_block();

size_t allocate_new_small_object(size_t object_size);

/* get address of block in small heap where object is stored */
size_t get_block_addr(size_t object_addr);

/* get start address of small allocator's heap */
size_t get_small_heap_start();

/* get end address of small allocator's heap */
size_t get_small_heap_end();

Node* get_segreg_list_head(size_t size);

void set_segreg_list_head(size_t size, Node* new_head);

Node* get_block_node(int index);

Node* get_empty_list_head();

void set_empty_list_head(Node* new_head);
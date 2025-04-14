#include "small-objs-heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../logging/log.h"
#include "allocator.h"

size_t START_ALLOCATOR_HEAP = 0;
size_t END_ALLOCATOR_HEAP = 0;

static Node* SEGREG_LIST[OBJECT_SIZE_UPPER_BOUND] = {0};
static Node NODES_LIST[BLOCKS_COUNT];
static Node* EMPTY_LIST_HEAD = 0;

void __init_small_objs_heap() {
    START_ALLOCATOR_HEAP = (size_t)mmap(NULL, HEAP_SIZE, PROT_WRITE | PROT_READ,
                                        MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if (START_ALLOCATOR_HEAP == MAP_FAILED) {
        log(INIT_ALLOCATOR, ERROR);
        return;
    }

    END_ALLOCATOR_HEAP = START_ALLOCATOR_HEAP + HEAP_SIZE;

    for (int i = 0; i < BLOCKS_COUNT; i++) {
        NODES_LIST[i].block_addr = START_ALLOCATOR_HEAP + BLOCK_SIZE * i;
        *(size_t*)NODES_LIST[i].block_addr =
            NODES_LIST[i].block_addr + BLOCK_HEADER_SIZE;
        if (i != BLOCKS_COUNT - 1) {
            NODES_LIST[i].next_node = &NODES_LIST[i + 1];
        } else {
            NODES_LIST[i].next_node = 0;
        }
    }

    EMPTY_LIST_HEAD = &NODES_LIST[0];
}

void __destroy_small_objs_heap() {
    if (munmap((void*)START_ALLOCATOR_HEAP, HEAP_SIZE) == -1) {
        log(DESTROY_ALLOCATOR, ERROR);
    }
}

/* start of getters */

size_t get_small_obj_heap_start() { return START_ALLOCATOR_HEAP; }

size_t get_small_obj_heap_end() { return END_ALLOCATOR_HEAP; }

Node* get_segreg_list_head(size_t size) { return SEGREG_LIST[size]; }

void set_segreg_list_head(size_t size, Node* new_head) {
    SEGREG_LIST[size] = new_head;
}

Node* get_block_node(int index) { return &NODES_LIST[index]; }

Node* get_empty_list_head() { return EMPTY_LIST_HEAD; }

void set_empty_list_head(Node* new_head) { EMPTY_LIST_HEAD = new_head; }
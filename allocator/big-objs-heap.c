#include "big-objs-heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../logging/log.h"
#include "allocator.h"

static Header HEADERS_LIST[HEADERS_COUNT];
static Header* HEADER_LIST_HEAD = 0;

Header* free_p = NULL;
Header* occupied_p = NULL;

size_t START_BIG_ALLOCATOR_HEAP = 0;
size_t END_BIG_ALLOCATOR_HEAP = 0;

Header* get_new_header() {
    if (HEADER_LIST_HEAD == NULL) {
        return NULL;
    } else {
        Header* result = HEADER_LIST_HEAD;
        HEADER_LIST_HEAD = HEADER_LIST_HEAD->next_header;
        result->next_header = NULL;
        return result;
    }
}

void __init_big_objs_heap() {
    START_BIG_ALLOCATOR_HEAP =
        (size_t)mmap(NULL, HEAP_SIZE, PROT_WRITE | PROT_READ,
                     MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if (START_BIG_ALLOCATOR_HEAP == MAP_FAILED) {
        log(INIT_ALLOCATOR, B_ERROR);
        return;
    }

    for (int i = 0; i < HEADERS_COUNT; i++) {
        HEADERS_LIST[i].addr = HEADERS_LIST[i].size = 0;
        HEADERS_LIST[i].isMarked = false;
        if (i != HEADERS_COUNT - 1) {
            HEADERS_LIST[i].next_header = &HEADERS_LIST[i + 1];
        } else {
            HEADERS_LIST[i].next_header = 0;
        }
    }

    HEADER_LIST_HEAD = &HEADERS_LIST[0];

    END_BIG_ALLOCATOR_HEAP = START_BIG_ALLOCATOR_HEAP + HEAP_SIZE;

    free_p = get_new_header();

    free_p->isMarked = false;
    free_p->next_header = NULL;
    free_p->size = HEAP_SIZE;
    free_p->addr = START_BIG_ALLOCATOR_HEAP;

    occupied_p = NULL;
}

void __destroy_big_objs_heap() {
    if (munmap((void*)START_BIG_ALLOCATOR_HEAP, HEAP_SIZE) == -1) {
        log(DESTROY_ALLOCATOR, B_ERROR);
    }
}

size_t get_big_objs_heap_start() { return START_BIG_ALLOCATOR_HEAP; }

size_t get_big_objs_heap_end() { return END_BIG_ALLOCATOR_HEAP; }

Header* get_free_p() { return free_p; }

void set_free_p(Header* new_free_p) { free_p = new_free_p; }

Header* get_occupied_p() { return occupied_p; }

void set_occupied_p(Header* new_occupied_p) { occupied_p = new_occupied_p; }
#include "large-allocator.h"

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

void __init_large_allocator() {
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

void __destroy_large_allocator() {
    if (munmap((void*)START_BIG_ALLOCATOR_HEAP, HEAP_SIZE) == -1) {
        log(DESTROY_ALLOCATOR, B_ERROR);
    }
}

size_t allocate_large_object(size_t object_size) {
    Header *p, *prev = NULL;

    object_size = GET_SIZE_WITH_ALIGNMENT(object_size);
    log_t cts_result = check_the_space(object_size);

    if (free_p == NULL) {  // no free blocks
        return (size_t)NULL;
    }

    for (p = free_p; p != NULL; prev = p, p = p->next_header) {
        if (p->size >= object_size) {
            Header* new_header;

            if (p->size == object_size) {
                // move entire header to the occupied headers
                if (prev == NULL && p->next_header == NULL) {
                    free_p = NULL;
                } else {
                    if (prev)
                        prev->next_header = p->next_header;
                    else
                        free_p = p->next_header;
                }
                new_header = p;
            } else {
                // take as much as we need
                if ((new_header = get_new_header()) == NULL) {
                    log(OTHER, O_HEADER);
                    return (size_t)NULL;
                }

                new_header->addr = p->addr;
                new_header->size = object_size;

                p->addr += object_size;
                p->size -= object_size;
            }

            new_header->isMarked = false;

            if (occupied_p == NULL) {
                occupied_p = new_header;
                new_header->next_header = NULL;
            } else {
                new_header->next_header = occupied_p;
                occupied_p = new_header;
            }

            log(ALLOCATE_NEW_OBJECT, OK);
            return new_header->addr;
        }
    }

    return (size_t)NULL;
}

size_t get_large_heap_start() { return START_BIG_ALLOCATOR_HEAP; }

size_t get_large_heap_end() { return END_BIG_ALLOCATOR_HEAP; }

Header* get_free_p() { return free_p; }

void set_free_p(Header* new_free_p) { free_p = new_free_p; }

Header* get_occupied_p() { return occupied_p; }

void set_occupied_p(Header* new_occupied_p) { occupied_p = new_occupied_p; }
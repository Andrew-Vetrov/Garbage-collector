#include "small-allocator.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <threads.h>

#include "../logging/log.h"
#include "allocator.h"
#include "bitmap.h"
#include "utils.h"

size_t START_ALLOCATOR_HEAP = 0;
size_t END_ALLOCATOR_HEAP = 0;

static Node* SEGREG_LIST[OBJECT_SIZE_UPPER_BOUND] = {0};
static Node NODES_LIST[BLOCKS_COUNT];
static Node* EMPTY_LIST_HEAD = 0;

thread_local Node* nodes_cache[OBJECT_SIZE_UPPER_BOUND] = {0};
static pthread_mutex_t empty_list_lock;
static pthread_mutex_t segreg_lists_locks[OBJECT_SIZE_UPPER_BOUND];

void clear_cache() {
    for (int i = 0; i < OBJECT_SIZE_UPPER_BOUND; i++) {
        nodes_cache[i] = 0;
    }
}

void lock_empty_list() { my_assert(pthread_mutex_lock(&empty_list_lock) == 0); }

void unlock_empty_list() {
    my_assert(pthread_mutex_unlock(&empty_list_lock) == 0);
}

void lock_segreg_list(size_t object_size) {
    my_assert(pthread_mutex_lock(&segreg_lists_locks[object_size]) == 0);
}

void unlock_segreg_list(size_t object_size) {
    my_assert(pthread_mutex_unlock(&segreg_lists_locks[object_size]) == 0);
}

void __init_small_allocator() {
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

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    my_assert(pthread_mutex_init(&empty_list_lock, &mutex_attr) == 0);

    for (int i = 0; i < OBJECT_SIZE_UPPER_BOUND; i++) {
        my_assert(pthread_mutex_init(&segreg_lists_locks[i], &mutex_attr) == 0);
    }
    pthread_mutexattr_destroy(&mutex_attr);
}

void __destroy_small_allocator() {
    if (munmap((void*)START_ALLOCATOR_HEAP, HEAP_SIZE) == -1) {
        log(DESTROY_ALLOCATOR, ERROR);
    }

    pthread_mutex_destroy(&empty_list_lock);

    for (int i = 0; i < OBJECT_SIZE_UPPER_BOUND; i++) {
        pthread_mutex_destroy(&segreg_lists_locks[i]);
    }
}

void init_header(Node* entry, size_t object_size) {
    size_t block_addr = entry->block_addr;

    size_t object_size_addr = GET_OBJECT_SIZE_ADDR(block_addr);
    *(size_t*)object_size_addr = object_size;

    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);
    size_t curr_bytes_addr = bitmap_addr;

    *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) = block_addr + BLOCK_HEADER_SIZE;

    for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
        *(size_t*)curr_bytes_addr = 0;
        curr_bytes_addr += sizeof(size_t);
    }
}

void fill_all_bitmaps_with_zeros() {
    Node* curr_entry;
    for (int i = 1; i < OBJECT_SIZE_UPPER_BOUND; i++) {
        curr_entry = SEGREG_LIST[i];
        while (curr_entry != NULL) {
            size_t bitmap_addr = GET_BITMAP_ADDR(curr_entry->block_addr);
            size_t curr_bytes_addr = bitmap_addr;

            for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
                *(size_t*)curr_bytes_addr = 0;
                curr_bytes_addr += sizeof(size_t);
            }

            curr_entry = curr_entry->next_node;
        }
    }
}

Node* allocate_new_block() {
    lock_empty_list();
    if (EMPTY_LIST_HEAD == NULL) {
        log(OTHER, O_EMPTY_BLOCK);
        unlock_empty_list();
        return NULL;
    } else {
        Node* result = EMPTY_LIST_HEAD;
        EMPTY_LIST_HEAD = result->next_node;
        result->next_node = NULL;
        unlock_empty_list();
        return result;
    }
}

Node* get_block_from_segreg_list(size_t object_size) {
    lock_segreg_list(object_size);
    Node* curr_entry = SEGREG_LIST[object_size];
    if (curr_entry != NULL) {
        SEGREG_LIST[object_size] = curr_entry->next_node;
    }
    unlock_segreg_list(object_size);
    return curr_entry;
}

size_t allocate_small_object(size_t object_size) {
    size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);
    log_t cts_result = check_the_space(object_size_with_alignment);

    Node* curr_entry = nodes_cache[object_size];

    size_t slider_position;

    bool is_checking_cache = curr_entry != NULL;
    bool is_block_found = false;

    while (!is_block_found) {
        if (!is_checking_cache) {
            curr_entry = get_block_from_segreg_list(object_size);

            if (curr_entry == NULL) {
                curr_entry = allocate_new_block();

                if (curr_entry == NULL) {
                    fill_all_bitmaps_with_zeros();
                    return (size_t)NULL;
                }

                init_header(curr_entry, object_size);
                break;
            }
        }

        size_t block_addr = curr_entry->block_addr;
        slider_position = *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr);
        size_t next_block_addr = block_addr + BLOCK_SIZE;

        while (slider_position + object_size_with_alignment <=
               next_block_addr) {
            if (get_bit_by_address(slider_position) == 0) {
                *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) =
                    slider_position;
                is_block_found = true;
                break;
            } else {
                set_bit_by_address(slider_position, 0);
                slider_position += object_size_with_alignment;
            }
        }

        if (is_checking_cache) {
            if (slider_position + object_size_with_alignment >
                next_block_addr) {
                nodes_cache[object_size] = NULL;
            }
            is_checking_cache = false;
        }
    }

    slider_position =
        *(size_t*)GET_SLIDER_POSITION_ADDR(curr_entry->block_addr);

    size_t new_slider_position = slider_position + object_size_with_alignment;
    if (new_slider_position >= curr_entry->block_addr + BLOCK_SIZE) {
        nodes_cache[object_size] = NULL;
    } else {
        nodes_cache[object_size] = curr_entry;
    }
    *(size_t*)GET_SLIDER_POSITION_ADDR(curr_entry->block_addr) =
        new_slider_position;
    log(ALLOCATE_NEW_OBJECT, OK);
    return slider_position;
    /*
    size_t block_addr;
    size_t slider_position;
    size_t next_block_addr;
    size_t object_size_with_alignment =
    GET_SIZE_WITH_ALIGNMENT(object_size);

    while (curr_entry != NULL) {
        block_addr = curr_entry->block_addr;
        slider_position = *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr);
        next_block_addr = block_addr + BLOCK_SIZE;

        while (slider_position + object_size_with_alignment <=
               next_block_addr) {
            if (get_bit_by_address(slider_position) == 0) {
                *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) =
                    slider_position + object_size_with_alignment;
                log(ALLOCATE_NEW_OBJECT, OK);
                return slider_position;
            } else {
                set_bit_by_address(slider_position, 0);
                slider_position = slider_position +
    object_size_with_alignment;
            }
        }

        *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position;

        curr_entry = curr_entry->next_node;
        SEGREG_LIST[object_size] = curr_entry;
    }

    curr_entry = allocate_new_block();
    SEGREG_LIST[object_size] = curr_entry;
    if (curr_entry == NULL) {
        fill_all_bitmaps_with_zeros();
        return (size_t)NULL;
    } else {
        init_header(curr_entry, object_size);

        block_addr = curr_entry->block_addr;
        slider_position = *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr);

        *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) =
            slider_position + object_size_with_alignment;
        log(ALLOCATE_NEW_OBJECT, OK);
        return slider_position;
    }
    */
}

/* getters */

size_t get_block_addr(size_t object_addr) {
    size_t object_relative_addr = object_addr - START_ALLOCATOR_HEAP;
    return (object_addr - (object_relative_addr % BLOCK_SIZE));
}

size_t get_small_heap_start() { return START_ALLOCATOR_HEAP; }

size_t get_small_heap_end() { return END_ALLOCATOR_HEAP; }

Node* get_segreg_list_head(size_t size) { return SEGREG_LIST[size]; }

Node* get_block_node(int index) { return &NODES_LIST[index]; }

Node* get_empty_list_head() { return EMPTY_LIST_HEAD; }

/* setters */

void set_empty_list_head(Node* new_head) { EMPTY_LIST_HEAD = new_head; }

void set_segreg_list_head(size_t size, Node* new_head) {
    SEGREG_LIST[size] = new_head;
}
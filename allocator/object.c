#include "object.h"

#include <assert.h>

#include "../logging/log.h"
#include "allocator.h"
#include "bitmap.h"
#include "large-allocator.h"
#include "small-allocator.h"

size_t get_object_size(Object object) {
    size_t object_addr = get_object_addr(object);

    if (get_small_heap_start() <= object_addr &&
        object_addr < get_small_heap_end()) {
        size_t object_relative_addr = object_addr - get_small_heap_start();
        size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
        size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);

        return GET_SIZE_WITH_ALIGNMENT(object_size);

    } else if (get_large_heap_start() <= object_addr &&
               object_addr < get_large_heap_end()) {
        Header* curr_header = get_occupied_p();
        while (curr_header != NULL) {
            if (curr_header->addr == object_addr) {
                return curr_header->size;
            }
            curr_header = curr_header->next_header;
        }
    }

    assert(false);
}

int get_object(size_t object_addr, Object* object) {
    if (object_addr >= get_large_heap_start() &&
        object_addr < get_large_heap_end()) {
        Header* curr_header = get_occupied_p();
        while (curr_header != NULL) {
            if (curr_header->addr <= object_addr &&
                object_addr < curr_header->addr + curr_header->size) {
                *object = curr_header->addr;
                return 0;
            }
            curr_header = curr_header->next_header;
        }
    } else if (object_addr >= get_small_heap_start() &&
               object_addr < get_small_heap_end()) {
        size_t block_addr = get_block_addr(object_addr);
        size_t object_addr_in_block = object_addr - block_addr;

        if (object_addr_in_block >= 0 &&
            object_addr_in_block < BLOCK_HEADER_SIZE) {  // pointer to header
            return INVALID_ADDRESS;
        }

        size_t object_size = get_object_size(object_addr);

        if (object_size <= 0 ||
            object_size > MAX_OBJECT_SIZE) {  // uninitialized block
            return INVALID_ADDRESS;
        }

        object_size = GET_SIZE_WITH_ALIGNMENT(object_size);

        *object = object_addr -
                  ((object_addr_in_block - BLOCK_HEADER_SIZE) % object_size);
        return 0;
    }

    return INVALID_ADDRESS;
}

void mark_object(Object object) {
    size_t object_addr = get_object_addr(object);
    if (object_addr >= get_large_heap_start() &&
        object_addr < get_large_heap_end()) {
        Header* curr_header = get_occupied_p();
        while (curr_header != NULL) {
            if (curr_header->addr == object_addr) {
                curr_header->isMarked = true;

                log_mark_alive(curr_header->size);
                return;
            }
            curr_header = curr_header->next_header;
        }
    } else if (object_addr >= get_small_heap_start() &&
               object_addr < get_small_heap_end()) {
        set_bit_by_address(object_addr, 1);
        log_mark_alive(get_object_size(object_addr));
    } else {
        fprintf(stderr, "Invalid address was given in mark_object()\n");
        assert(false);
    }
}

bool is_marked(Object object) {
    size_t object_addr = get_object_addr(object);
    if (object_addr >= get_large_heap_start() &&
        object_addr < get_large_heap_end()) {
        Header* object_header = 0;
        for (Header* curr_header = get_occupied_p(); curr_header != NULL;
             curr_header = curr_header->next_header) {
            if (curr_header->addr == object_addr) {
                object_header = curr_header;
                break;
            }
        }

        assert(object_addr != 0);

        return object_header->isMarked;
    } else if (object_addr >= get_small_heap_start() &&
               object_addr < get_small_heap_end()) {
        return get_bit_by_address(object_addr) ? true : false;
    } else {
        fprintf(stderr, "Invalid address %p was given in is_marked()\n",
                object_addr);
        assert(false);
    }
}

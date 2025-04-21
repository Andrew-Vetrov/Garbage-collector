#include "allocator.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>

#include "../logging/log.h"
#include "bitmap.h"
#include "large-allocator.h"
#include "small-allocator.h"

__attribute__((constructor)) void __init_allocator() {
    log(INIT_ALLOCATOR, START);

    __init_small_allocator();
    __init_large_allocator();

    set_memory_limit(HEAP_SIZE * 2, 100);

    log(INIT_ALLOCATOR, OK);
}

__attribute__((destructor)) void __destroy_allocator() {
    __destroy_small_allocator();
    __destroy_large_allocator();

    log(DESTROY_ALLOCATOR, OK);
}

size_t allocate_new_object(size_t size) {
    size_t res = (size_t)NULL;
    if (size >= 1 && size <= MAX_OBJECT_SIZE) {
        if ((res = allocate_small_object(size)) == NULL) {
            log(ALLOCATE_NEW_OBJECT, HEAP_ERROR);
        }
    } else if (size > MAX_OBJECT_SIZE && size <= HEAP_SIZE) {
        if ((res = allocate_large_object(size)) == NULL) {
            log(ALLOCATE_NEW_OBJECT, B_HEAP_ERROR);
        }
    }
    return res;
}

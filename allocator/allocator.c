#include "allocator.h"

#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>

#include "../logging/log.h"
#include "bitmap.h"
#include "large-allocator.h"
#include "small-allocator.h"
#include "utils.h"

pthread_rwlock_t allocation_lock = PTHREAD_RWLOCK_INITIALIZER;

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
    pthread_rwlock_destroy(&allocation_lock);

    log(DESTROY_ALLOCATOR, OK);
}

void lock_allocation() {
    my_assert(pthread_rwlock_wrlock(&allocation_lock) == 0);
}

void unlock_allocation() { pthread_rwlock_unlock(&allocation_lock); }

size_t allocate_new_object(size_t size) {
    pthread_rwlock_rdlock(&allocation_lock);
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
    pthread_rwlock_unlock(&allocation_lock);
    return res;
}

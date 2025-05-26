#pragma once
#include <stdbool.h>
#include <stdio.h>

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

void lock_allocation();

void unlock_allocation();
#pragma once
#include <stdio.h>

extern size_t START_ALLOCATOR_HEAP;

__attribute__((constructor))
void init_allocator();

__attribute__((destructor))
void destroy_allocator();

size_t allocate_new_object(size_t object_size);

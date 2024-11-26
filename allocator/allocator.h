#pragma once
#include <stdio.h>

extern size_t START_ALLOCATOR_HEAP;
extern size_t end_rsp_value;
extern size_t START_ALLOCATOR_HEAP;
extern size_t END_ALLOCATOR_HEAP;

__attribute__((constructor))
void init_allocator();

__attribute__((destructor))
void destroy_allocator();

size_t allocate_new_object(size_t object_size);

size_t get_object_size_by_address(size_t object_address);

void show_bitmap(size_t object_address);
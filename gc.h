#include <stdlib.h>

size_t gc_malloc(size_t size);
size_t gc_calloc(size_t size);
size_t gc_realloc(void* memblock, size_t size);

__attribute__((constructor))
void __init_allocator();

__attribute__((destructor))
void __destroy_allocator();
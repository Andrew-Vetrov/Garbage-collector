#include <stdlib.h>

size_t gc_malloc(size_t size);

__attribute__((constructor))
void __init_allocator();

__attribute__((destructor))
void __destroy_allocator();
#ifndef MEM_OPS
#define MEM_OPS

#include <immintrin.h>
#include <stddef.h>

void* gc_memset(void* dest, int value, size_t count);
void* gc_memcpy(void* dest, const void* src, size_t count);

#endif
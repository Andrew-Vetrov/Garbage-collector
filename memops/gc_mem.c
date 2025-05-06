#include "gc_mem.h"

void* gc_memset(void* dest, int value, size_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    __m128i fill = _mm_set1_epi8((unsigned char)value);

    size_t i = 0;
    for (; i + 16 <= count; i += 16) {
        _mm_storeu_si128((__m128i*)(ptr + i), fill);
    }

    for (; i < count; i++) {
        ptr[i] = (unsigned char)value;
    }

    return dest;
}

void* gc_memcpy(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    size_t i = 0;

    for (; i + 16 <= count; i += 16) {
        __m128i chunk = _mm_loadu_si128((__m128i*)(s + i));
        _mm_storeu_si128((__m128i*)(d + i), chunk);
    }

    for (; i < count; i++) {
        d[i] = s[i];
    }

    return dest;
}

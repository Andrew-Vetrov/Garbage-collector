#include <stdbool.h>
#include <stdlib.h>

#include "allocator.h"

#define HEADERS_COUNT (HEAP_SIZE / GET_SIZE_WITH_ALIGNMENT(MAX_OBJECT_SIZE + 1))

typedef struct Header {
    size_t addr;
    size_t size;
    bool isMarked;
    struct Header* next_header;
} Header;

void __init_large_allocator();

void __destroy_large_allocator();

size_t allocate_large_object(size_t object_size);

/* getters and setters */

/* get start address of large allocator's heap */
size_t get_large_heap_start();

/* get end address of large allocator's heap */
size_t get_large_heap_end();

Header* get_free_p();

void set_free_p(Header* new_free_p);

Header* get_occupied_p();

void set_occupied_p(Header* new_occupied_p);
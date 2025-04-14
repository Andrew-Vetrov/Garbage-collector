#include <stdbool.h>
#include <stdlib.h>

typedef struct Header {
    size_t addr;
    size_t size;
    bool isMarked;
    struct Header* next_header;
} Header;

void __init_big_objs_heap();

void __destroy_big_objs_heap();

Header* get_new_header();

size_t get_big_objs_heap_start();

size_t get_big_objs_heap_end();

Header* get_free_p();

void set_free_p(Header* new_free_p);

Header* get_occupied_p();

void set_occupied_p(Header* new_occupied_p);
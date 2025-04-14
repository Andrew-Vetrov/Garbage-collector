#include "allocator.h"
#include <assert.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdbool.h>
#include "../scanner/marking.h"
#include "../logging/log.h"
#include "small-objs-heap.h"
#include "big-objs-heap.h"

size_t end_rsp_value;

void show_bitmap(size_t object_addr) {
    size_t relative_object_addr = object_addr - get_small_obj_heap_start();
    size_t block_addr = object_addr - (relative_object_addr % BLOCK_SIZE);
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

    unsigned char curr_byte;

    for (int i = 0; i < BITMAP_BYTES_COUNT; i++) {
        printf("byte's index = %2d. bits: ", i);
        curr_byte = *(unsigned char*)(bitmap_addr + i);
        for (int j = 0; j < sizeof(curr_byte) * 8; j++) {
            putchar(((curr_byte >> j) & 1) == 1 ? '1' : '0');
        }
        putchar('\n');
    }
}

size_t get_block_addr(size_t object_addr) {
    size_t object_relative_addr = object_addr - get_small_obj_heap_start();
    return (object_addr - (object_relative_addr % BLOCK_SIZE));
}

void init_header(Node* entry, size_t object_size) {
    size_t block_addr = entry->block_addr;

    size_t object_size_addr = GET_OBJECT_SIZE_ADDR(block_addr);
    *(size_t*)object_size_addr = object_size;

    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);
    size_t curr_bytes_addr = bitmap_addr;

    for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
        *(size_t*)curr_bytes_addr = 0;
        curr_bytes_addr += sizeof(size_t);
    }
}

void fill_all_bitmaps_with_zeros() {
    Node* curr_entry;
    for (int i = 1; i < OBJECT_SIZE_UPPER_BOUND; i++) {
        curr_entry = get_segreg_list_head(i);
        while (curr_entry != NULL) {
            size_t bitmap_addr = GET_BITMAP_ADDR(curr_entry->block_addr);
            size_t curr_bytes_addr = bitmap_addr;

            for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
                *(size_t*)curr_bytes_addr = 0;
                curr_bytes_addr += sizeof(size_t);
            }

            curr_entry = curr_entry->next_node;
        }
    }
}

unsigned char get_bit_by_address(size_t object_addr) {

    size_t object_relative_addr = object_addr - get_small_obj_heap_start();
    size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
    size_t objects_addr = block_addr + BLOCK_HEADER_SIZE;
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

    size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);
    size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

    int object_index = (object_addr - objects_addr) / object_size_with_alignment;

    size_t byte_position = bitmap_addr + (object_index / (sizeof(unsigned char) * 8));
    size_t bit_position = object_index % (sizeof(unsigned char) * 8);

    return (unsigned char)(((*(unsigned char*)byte_position) >> bit_position) & 1);
}

void set_bit_by_address(size_t object_addr, unsigned char bit) {
    size_t object_relative_addr = object_addr - get_small_obj_heap_start();
    size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
    size_t objects_addr = block_addr + BLOCK_HEADER_SIZE;
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

    size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);
    size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

    int object_index = (object_addr - objects_addr) / object_size_with_alignment;

    size_t byte_position = bitmap_addr + (object_index / (sizeof(unsigned char) * 8));
    size_t bit_position = object_index % (sizeof(unsigned char) * 8);

    if ((((*(unsigned char*)byte_position) >> bit_position) & 1) != bit) {
        *(unsigned char*)byte_position = (*(unsigned char*)byte_position) ^ (1 << bit_position);
    }
}

bool is_bitmap_empty(size_t block_addr) {
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);
    size_t curr_bytes_addr = bitmap_addr;

    for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
        if (*(size_t*)curr_bytes_addr != 0) {
            return false;
        }
        curr_bytes_addr += sizeof(size_t);
    }

    return true;
}

size_t get_object_size_by_address(size_t object_addr) {

    if (get_small_obj_heap_start() <= object_addr && object_addr < get_small_obj_heap_end()) {
        size_t object_relative_addr = object_addr - get_small_obj_heap_start();
        size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
        size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);
    
        return GET_SIZE_WITH_ALIGNMENT(object_size);
    
    } else if (get_big_objs_heap_start() <= object_addr && object_addr < get_big_objs_heap_end()) {
        Header *curr_header = get_occupied_p();
        while (curr_header != NULL) {
            if (curr_header->addr == object_addr) {
                return curr_header->size;
            }
            curr_header = curr_header->next_header;
        }
    }

    assert(false);
}

__attribute__((constructor))
void __init_allocator() {
    log(INIT_ALLOCATOR, START);

    __init_small_objs_heap();
    __init_big_objs_heap();

    set_memory_limit(HEAP_SIZE * 2, 100);

    log(INIT_ALLOCATOR, OK);
}

Node* allocate_new_block() {
    if (get_empty_list_head() == NULL) {
        log(OTHER, O_EMPTY_BLOCK);
        return NULL;
    }
    else {
        Node* result = get_empty_list_head();
        set_empty_list_head(result->next_node);
        result->next_node = NULL;
        return result;
    }
}

__attribute__((destructor))
void __destroy_allocator() {
    __destroy_small_objs_heap();
    __destroy_big_objs_heap();

    log(DESTROY_ALLOCATOR, OK);
}

void sweep() {
    log(SWEEP, START);
    // start of sweeping small objects
#ifdef DEBUG
    int empty_nodes_count = 0;
    int segreg_list_nodes_count = 0;
#endif
    set_empty_list_head(NULL);
    for (int i = 0; i < OBJECT_SIZE_UPPER_BOUND; i++) {
        set_segreg_list_head(i, NULL);
    }

    for (int i = 0; i < BLOCKS_COUNT; i++) {
        Node* node = get_block_node(i);
        *(size_t*)GET_SLIDER_POSITION_ADDR(node->block_addr) =
            node->block_addr + BLOCK_HEADER_SIZE;
        node->next_node = NULL;
        if (is_bitmap_empty(node->block_addr)) {
#ifdef DEBUG
            empty_nodes_count++;
#endif
            * (size_t*)GET_OBJECT_SIZE_ADDR(node->block_addr) = 0;
            if (get_empty_list_head() == NULL) {
                set_empty_list_head(node);
            }
            else {
                node->next_node = get_empty_list_head();
                set_empty_list_head(node);
            }
        }
        else {
#ifdef DEBUG
            segreg_list_nodes_count++;
#endif
            size_t object_size =
                *(size_t*)GET_OBJECT_SIZE_ADDR(node->block_addr);
            if (get_segreg_list_head(object_size) == NULL) {
                set_segreg_list_head(object_size, node);
            }
            else {
                node->next_node = get_segreg_list_head(object_size);
                set_segreg_list_head(object_size, node);
            }
        }
    }
#ifdef DEBUG
    printf("empty_nodes_count = %d\nsegreg_list_nodes_count = %d\n",
        empty_nodes_count, segreg_list_nodes_count);
#endif

    // start of sweeping BIG objects
    if (!get_occupied_p()) {
        log(SWEEP, OK);
        return;
    }

    Header* prev = NULL, * curr = get_occupied_p(), * move;

    while (curr != NULL) {
        if (curr->isMarked == true) {
            curr->isMarked = false;
            prev = curr;
            curr = curr->next_header;
        }
        else {
            move = curr;

            if (prev == NULL && curr->next_header == NULL) {
                set_occupied_p(curr = NULL);
            }
            else {
                if (prev) {
                    prev->next_header = curr->next_header;
                    curr = curr->next_header;
                }
                else {
                    set_occupied_p(curr = curr->next_header);
                }
            }

            move->next_header = NULL;

            // find correct position for this header in free list
            Header* free_p = get_free_p();
            if (free_p == NULL) {
                set_free_p(move);
                move->next_header = NULL;
            }
            else if (move->addr < free_p->addr) {
                if (move->addr + move->size < free_p->addr) {
                    move->next_header = free_p;
                }
                else {
                    move->size += free_p->size;
                    move->next_header = free_p->next_header;
                }
                set_free_p(move);
            }
            else {
                Header* prev_free_p = free_p, * curr_free_p = free_p->next_header;

                while (curr_free_p && !(prev_free_p->addr < move->addr && move->addr < curr_free_p->addr)) {
                    prev_free_p = curr_free_p;
                    curr_free_p = curr_free_p->next_header;
                }

                if (curr_free_p) {
                    if (move->addr + move->size < curr_free_p->addr)
                        move->next_header = curr_free_p;
                    else {
                        move->size += curr_free_p->size;
                        move->next_header = curr_free_p->next_header;
                    }
                }

                if (prev_free_p->addr + prev_free_p->size < move->addr)
                    prev_free_p->next_header = move;
                else {
                    prev_free_p->size += move->size;
                    prev_free_p->next_header = move->next_header;
                }
            }
        }
    }

    log(SWEEP, OK);
}

size_t allocate_new_object(size_t object_size) {
    log_t cts_result = check_the_space(GET_SIZE_WITH_ALIGNMENT(object_size));

    Node* curr_entry = get_segreg_list_head(object_size);

    size_t block_addr;
    size_t slider_position;
    size_t next_block_addr;
    size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

    while (curr_entry != NULL) {
        block_addr = curr_entry->block_addr;
        slider_position = *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr);
        next_block_addr = block_addr + BLOCK_SIZE;

        while (slider_position + object_size_with_alignment <= next_block_addr) {
            if (get_bit_by_address(slider_position) == 0) {
                *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position + object_size_with_alignment;
                log(ALLOCATE_NEW_OBJECT, OK);
                return slider_position;
            }
            else {
                set_bit_by_address(slider_position, 0);
                slider_position = slider_position + object_size_with_alignment;
            }
        }

        *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position;

        curr_entry = curr_entry->next_node;
        set_segreg_list_head(object_size, curr_entry);
    }

    curr_entry = allocate_new_block();
    set_segreg_list_head(object_size, curr_entry);
    if (curr_entry == NULL) {
        fill_all_bitmaps_with_zeros();
        return (size_t) NULL;
    }
    else {
        init_header(curr_entry, object_size);

        block_addr = curr_entry->block_addr;
        slider_position = *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr);

        *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position + object_size_with_alignment;
        log(ALLOCATE_NEW_OBJECT, OK);
        return slider_position;
    }
}

size_t allocate_new_BIG_object(size_t object_size) {
    Header* p, * prev = NULL;

    object_size = GET_SIZE_WITH_ALIGNMENT(object_size);
    log_t cts_result = check_the_space(object_size);

    if (get_free_p() == NULL) { // no free blocks
        return (size_t) NULL;
    }

    for (p = get_free_p(); p != NULL; prev = p, p = p->next_header) {
        if (p->size >= object_size) {

            Header* new_header;

            if (p->size == object_size) {
                // move entire header to the occupied headers
                if (prev == NULL && p->next_header == NULL) {
                    set_free_p(NULL);
                }
                else {
                    if (prev)
                        prev->next_header = p->next_header;
                    else
                        set_free_p(p->next_header);
                }
                new_header = p;
            }
            else {
                // take as much as we need
                if ((new_header = get_new_header()) == NULL) {
                    log(OTHER, O_HEADER);
                    return (size_t) NULL;
                }

                new_header->addr = p->addr;
                new_header->size = object_size;

                p->addr += object_size;
                p->size -= object_size;
            }

            new_header->isMarked = false;

            if (get_occupied_p() == NULL) {
                set_occupied_p(new_header);
                new_header->next_header = NULL;
            }
            else {
                new_header->next_header = get_occupied_p();
                set_occupied_p(new_header);
            }

            log(ALLOCATE_NEW_OBJECT, OK);
            return new_header->addr;
        }
    }

    return (size_t) NULL;
}

size_t gc_malloc(size_t size) {
    size_t res = (size_t) NULL;
    push_registers_to_stack();
    asm volatile("mov %%rsp, %0" : "=r" (end_rsp_value));
    if (size >= 1 && size <= MAX_OBJECT_SIZE) {
        res = allocate_new_object(size);
        if (res == NULL) {
            collect();
            res = allocate_new_object(size);
            if (res == NULL) {
                log(ALLOCATE_NEW_OBJECT, HEAP_ERROR);
            }
        }
    }
    else if (size > MAX_OBJECT_SIZE && size <= HEAP_SIZE) {
        res = allocate_new_BIG_object(size);
        if (res == NULL) {
            collect();
            res = allocate_new_BIG_object(size);
            if (res == NULL) {
                log(ALLOCATE_NEW_OBJECT, B_HEAP_ERROR);
            }
        }
    }

    return res;
}

int get_object(size_t object_addr, Object* object) {
    if (object_addr >= get_big_objs_heap_start() && object_addr < get_big_objs_heap_end()) {
        Header* curr_header = get_occupied_p();
        while (curr_header != NULL) {
            if (curr_header->addr <= object_addr && object_addr < curr_header->addr + curr_header->size) {
                *object = curr_header->addr;
                return 0;
            }
            curr_header = curr_header->next_header;
        }
    } else if (object_addr >= get_small_obj_heap_start() && object_addr < get_small_obj_heap_end()) {
        size_t block_addr = get_block_addr(object_addr);
        size_t object_addr_in_block = object_addr - block_addr;

        if (object_addr_in_block >= 0 && object_addr_in_block < BLOCK_HEADER_SIZE) {       // pointer to header
            return INVALID_ADDRESS;
        }

        size_t object_size = get_object_size_by_address(object_addr);

        if (object_size <= 0 || object_size > MAX_OBJECT_SIZE) {                           // uninitialized block
            return INVALID_ADDRESS;
        }

        object_size = GET_SIZE_WITH_ALIGNMENT(object_size);

        *object = object_addr - ((object_addr_in_block - BLOCK_HEADER_SIZE) % object_size);
        return 0;
    }

    return INVALID_ADDRESS;
}

void mark_object(Object object) {
    size_t object_addr = get_object_addr(object);
    if (object_addr >= get_big_objs_heap_start() && object_addr < get_big_objs_heap_end()) {
        Header* curr_header = get_occupied_p();
        while (curr_header != NULL) {
            if (curr_header->addr == object_addr) {
                curr_header->isMarked = true;

                log_mark_alive(curr_header->size);
                return;
            }
            curr_header = curr_header->next_header;
        }
    }
    else if (object_addr >= get_small_obj_heap_start() && object_addr < get_small_obj_heap_end()) {
        set_bit_by_address(object_addr, 1);
        log_mark_alive(get_object_size_by_address(object_addr));
    }
    else {
        fprintf(stderr, "Invalid address was given in mark_object()\n");
        assert(false);
    }
}

bool is_marked(Object object) {
    size_t object_addr = get_object_addr(object);
    if (object_addr >= get_big_objs_heap_start() &&
        object_addr < get_big_objs_heap_end()) {
        Header* object_header = 0;
        for (Header* curr_header = get_occupied_p(); curr_header != NULL;
            curr_header = curr_header->next_header) {
            if (curr_header->addr == object_addr) {
                object_header = curr_header;
                break;
            }
        }

        assert(object_addr != 0);

        return object_header->isMarked;
    } else if (object_addr >= get_small_obj_heap_start() && 
               object_addr < get_small_obj_heap_end()) {
        return get_bit_by_address(object_addr) ? true : false;
    } else {
        fprintf(stderr, "Invalid address %p was given in is_marked()\n", object_addr);
        assert(false);
    }
}

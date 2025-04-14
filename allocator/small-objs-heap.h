#pragma once
#include <stdlib.h>

typedef struct Node_t {
    size_t block_addr;
    struct Node_t* next_node;
} Node;

void __init_small_objs_heap();

void __destroy_small_objs_heap();

size_t get_small_obj_heap_start();

size_t get_small_obj_heap_end();

Node* get_segreg_list_head(size_t size);

void set_segreg_list_head(size_t size, Node* new_head);

Node* get_block_node(int index);

Node* get_empty_list_head();

void set_empty_list_head(Node* new_head);
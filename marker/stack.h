#include <stdlib.h>

#pragma once

typedef struct {
    size_t* data;
    int top;
    int capacity;
} Stack;

#define STACK_INIT_CAPACITY 16


Stack* create_stack();


int is_empty(Stack* stack);


void resize(Stack* stack);


void push(Stack* stack, size_t value);


size_t pop(Stack* stack);


void free_stack(Stack* stack);

#include <stdlib.h>
#include "stack.h"

Stack* create_stack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->data = (size_t**)malloc(STACK_INIT_CAPACITY * sizeof(size_t*));
    stack->top = -1;
    stack->capacity = STACK_INIT_CAPACITY;
    return stack;
}

int is_empty(Stack *stack) {
    return stack->top == -1;
}

void resize(Stack* stack) {
    stack->capacity *= 2;
    stack->data = (size_t**)realloc(stack->data, stack->capacity * sizeof(size_t*));
}

void push(Stack* stack, size_t* value) {
    if (stack->top == stack->capacity - 1) {
        resize(stack);
    }
    stack->data[++stack->top] = value;
}

size_t* pop(Stack* stack) {
    if (is_empty(stack)) {
        exit(EXIT_FAILURE);
    }
    return stack->data[stack->top--];
}

void free_stack(Stack* stack) {
    free(stack->data);
    free(stack);
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "../allocator/allocator.h"
#include "../scanner/segment_traverse.h"

#define REGISTER_NAME_SIZE 10
#define REGISTER_AMOUNT 13
#define STACK_INIT_CAPACITY 10

size_t start_rsp_value;
size_t heap_start_address;
size_t heap_end_address;
size_t current_stack_end;
extern char __bss_start;
extern char __data_start;
extern char edata;
extern char end;

const char REGISTERS[REGISTER_AMOUNT][REGISTER_NAME_SIZE] = {
    "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
    "r12", "r13", "r14", "r15"
};

void before_main(void) {
    asm volatile("mov %%rsp, %0" : "=r" (start_rsp_value));
}




// Stack structure and methods

typedef struct {
    size_t** data;
    int top;
    int capacity;
} Stack;

Stack* stack;

Stack* create_stack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    stack->data = (size_t**)malloc(STACK_INIT_CAPACITY * sizeof(size_t*));
    stack->top = -1;
    stack->capacity = STACK_INIT_CAPACITY;
    return stack;
}

int is_empty() {
    return stack->top == -1;
}

void resize() {
    stack->capacity *= 2;
    stack->data = (size_t**)realloc(stack->data, stack->capacity * sizeof(size_t*));
}

void push(size_t* value) {
    if (stack->top == stack->capacity - 1) {
        resize();
    }
    stack->data[++stack->top] = value;
}

size_t pop() {
    if (is_empty()) {
        fprintf(stderr, "Ошибка: попытка pop из пустого стека\n");
        exit(EXIT_FAILURE);
    }
    //printf("in pop  %llu\n", stack->data[stack->top--]);
    return stack->data[stack->top--];
}

void free_stack(Stack* stack) {
    free(stack->data);
    free(stack);
}
// Stack structure and methods

void mark(size_t* elem) {

    if (!get_bit_by_address(elem)) {
        set_bit_by_address(elem, 1);
        push(elem);
    }
}

void scan(size_t* elem) {
    size_t size = get_object_size_by_address(elem);

    for (int i = 0; i < size; i += sizeof(size_t)) {
        if (*(elem + i) >= START_ALLOCATOR_HEAP && *(elem + i) < END_ALLOCATOR_HEAP) {
            mark(elem + i);
            show_bitmap(elem);
        }
    }
}

void closure() {
    size_t cur_elem;
    while (!is_empty()) {
        cur_elem = pop();
        scan(cur_elem);
    }
}


void push_registers_to_stack() {
    char reg[REGISTER_NAME_SIZE];
    for (int i = 0; i < REGISTER_AMOUNT; i++) {
        asm volatile(
            "push %[reg]\n"
            :
            : [reg] "r" (REGISTERS[i])
            );
    }
}

void pop_registers_from_stack() {
    char reg[REGISTER_NAME_SIZE];
    for (int i = REGISTER_AMOUNT - 1; i >= 0; i--) {
        asm volatile(
            "pop %[reg]\n"
            :
            : [reg] "r" (REGISTERS[i]));
    }
}

void segment_traverse(size_t segment_start, size_t segment_end) {
    size_t size;
    stack = create_stack();
    for (size_t i = segment_start; i < segment_end; i += sizeof(size_t)) {
        if (*((size_t*)i) >= START_ALLOCATOR_HEAP && *((size_t*)i) < END_ALLOCATOR_HEAP) {
            size = get_object_size_by_address(*((size_t*)i));
            printf("size %d\n", size);
            if (size > 0) {
                mark(*((size_t*)i));
                show_bitmap(*((size_t*)i));
            }
        }
    }
    closure();
}

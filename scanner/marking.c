#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "../allocator/allocator.h"
#include "marking.h"
#include "stack.h"

#define REGISTER_NAME_SIZE 10
#define REGISTER_AMOUNT 13
#define STACK_INIT_CAPACITY 10

size_t start_rsp_value;
extern char __bss_start;
extern char __data_start;
extern char edata;
extern char end;

Stack* stack;

const char REGISTERS[REGISTER_AMOUNT][REGISTER_NAME_SIZE] = {
    "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
    "r12", "r13", "r14", "r15"
};

void before_main(void) {
    asm volatile("mov %%rsp, %0" : "=r" (start_rsp_value));
}


bool is_pointer_valid(size_t object_addr) {
    if (object_addr < START_ALLOCATOR_HEAP || object_addr >= END_ALLOCATOR_HEAP) {              //pointer isn't in heap
        return false;
    }

    size_t block_addr = get_block_addr(object_addr);
    size_t object_addr_in_block = object_addr - block_addr;

    if (object_addr_in_block >= 0 && object_addr_in_block < BLOCK_HEADER_SIZE) {                //pointer to header
        return false;
    }

    size_t object_size = GET_SIZE_WITH_ALIGNMENT(get_object_size_by_address(object_addr));      

    if ((object_addr_in_block - BLOCK_HEADER_SIZE) % object_size != 0) {                        //pointer to wrong position in block
        return false;
    }

    return true;
}

void mark(size_t* elem) {
    //if (elem != START_ALLOCATOR_HEAP && !get_bit_by_address(elem)) {
    printf("%p\n", elem);
    printf("BIT = %d\n", get_bit_by_address(elem));
    if (!get_bit_by_address(elem)) {
        set_bit_by_address(elem, 1);
        push(stack, elem);
    }
}

void scan(size_t elem) {
    size_t size = get_object_size_by_address(elem);
    printf("Haha\n");
    for (int i = 0; i < size; i += sizeof(size_t)) {
        if (*((size_t*)(elem + i)) >= START_ALLOCATOR_HEAP && *((size_t*)(elem + i)) < END_ALLOCATOR_HEAP) {
            if (get_object_size_by_address(elem + i) == 0) { 
                continue;
            }
            mark(*((size_t*)(elem + i)));
        }
    }
}

void closure() {
    size_t cur_elem;
    while (!is_empty(stack)) {
        cur_elem = pop(stack);
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
        if (*((size_t*)i) >= START_ALLOCATOR_HEAP && *((size_t*)i) < END_ALLOCATOR_HEAP
            && is_pointer_valid(i)) {
            size = get_object_size_by_address(*((size_t*)i));
            if (size > 0) {
                //printf("Started marking\n");
                mark(*((size_t*)i));
            }
        }
    }
    closure();
}

void full_marking() {
    segment_traverse(end_rsp_value, start_rsp_value);
    segment_traverse(&__data_start, &edata);
    segment_traverse(&__bss_start, &end);
}

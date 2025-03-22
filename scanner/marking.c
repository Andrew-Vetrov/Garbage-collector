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

void mark(size_t* elem) {
    if (!is_marked(elem)) {
        mark_object(elem);
        push(stack, elem);
    }
}

void scan(size_t object_addr) {
    size_t object_start_addr = get_valid_object(object_addr);
    size_t object_end_addr = object_start_addr + get_object_size_by_address(object_start_addr);
    for (size_t inner_object_addr = object_start_addr; inner_object_addr < object_end_addr; inner_object_addr += sizeof(size_t)) {
        size_t valid_inner_object_addr = get_valid_object(inner_object_addr);
        if (valid_inner_object_addr != 0) {
            mark((size_t*) valid_inner_object_addr);
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
    stack = create_stack();
    for (size_t object_addr = segment_start; object_addr < segment_end; object_addr += sizeof(size_t)) {
        size_t valid_object_addr = get_valid_object(object_addr);
        if (valid_object_addr != 0) {
            mark((size_t*) valid_object_addr);
        }
    }
    closure();
}

void collect() {
    full_marking();
    sweep();
}

void full_marking() {
    segment_traverse(end_rsp_value, start_rsp_value);
    segment_traverse(&__data_start, &edata);
    segment_traverse(&__bss_start, &end);
}

#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>
#include "../allocator/allocator.h"
#include "../logging/log.h"
#include "marking.h"
#include "stack.h"
#include <assert.h>

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
    stack = create_stack();
    asm volatile("mov %%rsp, %0" : "=r" (start_rsp_value));
}

<<<<<<< HEAD
void mark(Object object) {
    if (!is_marked(object)) {
        mark_object(object);
        push(stack, get_object_addr(object));
=======
void mark(size_t* elem) {
    //if (elem != START_ALLOCATOR_HEAP && !get_bit_by_address(elem)) {
    printf("%p\n", elem);
    printf("BIT = %d\n", get_bit_by_address(elem));
    if (!get_bit_by_address(elem)) {
        set_bit_by_address(elem, 1);
        push(stack, elem);
>>>>>>> 266071b (inner_pointer_fix: done)
    }
}

void scan(size_t object_addr) {
<<<<<<< HEAD
    Object object;
    if (get_object(object_addr, &object) == INVALID_ADDRESS) {
        return;
    }
    size_t object_start_addr = get_object_addr(object);
    size_t object_end_addr = object_start_addr + get_object_size_by_address(object_start_addr);
    for (size_t inner_object_addr = object_start_addr; inner_object_addr < object_end_addr; inner_object_addr += sizeof(size_t)) {
        Object inner_object;
        if (get_object(*(size_t*)inner_object_addr, &inner_object) == 0) {
            mark(inner_object);
=======
    size_t object_start_addr = get_valid_object(object_addr);
    size_t object_end_addr = object_start_addr + get_object_size_by_address(object_start_addr);
    for (size_t inner_object_addr = object_start_addr; inner_object_addr < object_end_addr; inner_object_addr += sizeof(size_t)) {
        size_t valid_inner_object_addr = get_valid_object(inner_object_addr);
        if (valid_inner_object_addr != 0) {
            mark((size_t*) valid_inner_object_addr);
>>>>>>> 266071b (inner_pointer_fix: done)
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
    assert(segment_start && segment_end && segment_start < segment_end);
    for (size_t object_addr = segment_start; object_addr < segment_end; object_addr += sizeof(size_t)) {
        Object object;
        if (get_object(*(size_t*)object_addr, &object) == 0) {
            mark(object);
        }
    }
    closure();
}

void collect() {
    full_marking();
    sweep();
}

void full_marking() {
    log(MARK, START);
    segment_traverse(end_rsp_value, start_rsp_value);
    segment_traverse((size_t)&__data_start, (size_t)&edata);
    segment_traverse((size_t)&__bss_start, (size_t)&end);
    log(MARK, OK);
}
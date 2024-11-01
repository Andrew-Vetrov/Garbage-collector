﻿#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "segment_traverse.h"

#define REGISTER_NAME_SIZE 10
#define REGISTER_AMOUNT 13

size_t RSP;
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
    asm volatile("mov %%rsp, %0" : "=r" (RSP));
    current_stack_end = RSP;
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
    for (size_t i = segment_start; i < segment_end; i += sizeof(size_t)) {
        if (*((size_t*)i) >= heap_start_address && *((size_t*)i) <= heap_end_address) {
            printf("FOUND\n");
        }
    }
}


#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "segment_traverse.h"

#define REGISTER_NAME_SIZE 10
#define REGISTER_AMOUNT 9

size_t RSP;
size_t heap_start_address;
size_t heap_end_address;
size_t current_stack_end;
extern char __bss_start;
extern char __data_start;
extern char edata;
extern char end;
int heap_counter = 0; //have to delete
int pseudo_heap_counter = 0; //have to delete
const char REGISTERS[REGISTER_AMOUNT][REGISTER_NAME_SIZE] = {
    "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
};  

void before_main(void) {
    asm volatile("mov %%rsp, %0" : "=r" (RSP));
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
    for (size_t i = segment_start; i <= segment_end; i += sizeof(size_t)) {
        if (*((size_t*)i) >= heap_start_address && *((size_t*)i) <= heap_end_address) {
            heap_counter++;
        }
    }
}


size_t createHeap(size_t length) {
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    int fd = -1; // anonymous mapping
    off_t offset = 0;

    heap_start_address = (size_t)mmap(NULL, length, prot, flags, fd, offset);

    return heap_start_address;
}

void* silly_allocate(size_t size) {
    pseudo_heap_counter++;
    asm volatile("mov %%rsp, %0" : "=r" (current_stack_end));
    static size_t offset = 0;
    void* ptr = offset + heap_start_address;
    offset += size;
    return ptr;
}

void testGC() {
    push_registers_to_stack();

    //stack traversing
    //traversing stack from its end due to special structure
    segment_traverse(current_stack_end, RSP);

    //.data traversing
    segment_traverse(&__data_start, &edata - sizeof(size_t));

    //.bss traversing
    segment_traverse(&__bss_start, &end - sizeof(size_t));

    pop_registers_from_stack();
}

int* b;
int* k = 12321421;

int main() {
    size_t heapLength = 1024;
    createHeap(heapLength);
    heap_end_address = heap_start_address + heapLength;
    int* a = (int*)silly_allocate(sizeof(int));
    b = (int*)silly_allocate(sizeof(int));
    k = (int*)silly_allocate(sizeof(int));
    printf("Создано %d объекта на куче\n\n", pseudo_heap_counter);;
    printf("Адрес объекта = %p\t%p\t%p\n", a, b, k);
    printf("Начало кучи = %p\n", heap_start_address);
    printf("Конец кучи = %p\n", heap_end_address);
    testGC();
    printf("\nНайдено %d объектов на куче\n", heap_counter);
    return 0;
}


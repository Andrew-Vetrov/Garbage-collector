#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define REGISTER_NAME_SIZE 10
#define REGISTER_AMOUNT 9

size_t RSP;
size_t* heapStartAddress;
size_t* heapEndAddress;
size_t currentStackEnd;

const char REGISTERS[REGISTER_AMOUNT][REGISTER_NAME_SIZE] = {
    "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
};


void beforeMain(void) __attribute__((constructor));

void beforeMain(void) {
    asm volatile("mov %%rsp, %0" : "=r" (RSP));
    printf("RSP value: %lx\n", RSP);
}

void pushRegistersToStack() {
    char reg[REGISTER_NAME_SIZE];
    for (int i = 0; i < REGISTER_AMOUNT; i++) {
        asm volatile(
            "push %[reg]\n" 
            : 
            : [reg] "r" (REGISTERS[i])
            );
    }
}

void popRegistersFromStack() {
    char reg[REGISTER_NAME_SIZE];
    for (int i = REGISTER_AMOUNT - 1; i >= 0; i--) {
        asm volatile(
            "pop %[reg]\n" 
            :
            : [reg] "r" (REGISTERS[i]));
    }
}

void stackTraverse(size_t* heapStartAddress, size_t* heapEndAddress) {
    for (size_t i = RSP; i > currentStackEnd; i -= 8) {
        if (*((size_t*)i) >= heapStartAddress && *((size_t*)i) <= heapEndAddress) {
            printf("Object is on heap\t\t\t%p\n", *((size_t*)i));
        }
    }
}

void createHeap(size_t length) {
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    int fd = -1; // anonymous mapping
    off_t offset = 0;

    heapStartAddress = (size_t)mmap(NULL, length, prot, flags, fd, offset);

    return heapStartAddress;
}

void* sillyAllocate(size_t size) {
    asm volatile("mov %%rsp, %0" : "=r" (currentStackEnd));
    static size_t offset = 0;
    void* ptr = offset + heapStartAddress;
    offset += size;
    return ptr;
}

void testGC() {
    pushRegistersToStack();

    
    stackTraverse(heapStartAddress, heapEndAddress);

    popRegistersFromStack();
}

int main() {
    size_t heapLength = 1024 * 1024 * 512; //512 Mb
    createHeap(heapLength);
    heapEndAddress = heapStartAddress + heapLength;
    int* a = (int*)sillyAllocate(sizeof(int) * 5);
    printf("a = %p\n", a);
    testGC();
    
    return 0;
}


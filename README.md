# Garbage-collector

## Description

This project implements a multithreaded conservative mark&sweep garbage collector for C programs. The collector supports automatic memory management, works with multiple threads, and provides tools for integration with user code.

## API

The public API consists of three functions:

- `void* gc_malloc(size_t size)`  
  Allocates a memory block of the given size (in bytes), similar to `malloc`. Returns a pointer to the allocated memory.

- `void* gc_calloc(size_t nmemb, size_t size)`  
  Allocates memory for an array of `nmemb` elements of `size` bytes each, initializes all bytes to zero (like `calloc`). Returns a pointer to the allocated memory.

- `void* gc_realloc(void* ptr, size_t size)`  
  Changes the size of the memory block pointed to by `ptr` to `size` bytes (like `realloc`). Returns a pointer to the reallocated memory, which may be different from `ptr`.
  
## Quick Start

1. Clone the repository:
   ```sh
   git clone https://github.com/Andrew-Vetrov/Garbage-collector.git
   cd Garbage-collector
   ```

2. Build the project:
   ```sh
   make
   ```

3. Build your program with the library, including `gc.h`:
   ```sh
   gcc your_program.c ./build/libgc.so
   ```
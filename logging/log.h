#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdarg.h>

#ifndef LOG_H_
#define LOG_H_

#define BUFFER_SIZE 500000
#define BUFFER_DEAD_NUMBER 200

#define INIT_ALLOCATOR 0
#define DESTROY_ALLOCATOR 1
#define ALLOCATE_NEW_OBJECT 2
#define MARK 3
#define SWEEP 4

#define ERROR -1
#define OK 0
#define START 1
#define ALIVE 2

typedef int8_t log_t;

static double log_time();

static void printf_in_file();

static void add_log_line(const char* format, ...);

void set_memory_limit(size_t heap_size, size_t percent);

void log(log_t type, log_t result);

log_t check_the_space(size_t object_size);

#endif
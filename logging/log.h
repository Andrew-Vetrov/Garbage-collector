#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#ifndef LOG_H_
#define LOG_H_

#define BUFFER_SIZE 2000000
#define BUFFER_DEAD_NUMBER 333

#define INIT_ALLOCATOR 0
#define DESTROY_ALLOCATOR 1
#define ALLOCATE_NEW_OBJECT 2
#define MARK 3
#define SWEEP 4
#define OTHER 5

#define O_EMPTY_BLOCK -6
#define O_HEADER -5
#define B_HEAP_ERROR -4
#define HEAP_ERROR -3
#define B_ERROR -2
#define ERROR -1
#define OK 0
#define START 1
#define ALIVE 2

typedef int8_t log_t;

static double log_time();

static void printf_in_file();

static void add_log_line(const char* format, ...);

static void log_new_object();

void set_memory_limit(size_t heap_size, size_t percent);

void log(log_t type, log_t result);

log_t check_the_space(size_t object_size);

void log_mark_alive(size_t object_size);

#endif
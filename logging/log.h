#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdarg.h>

#ifndef LOG_H_
#define LOG_H_

#define INIT_ALLOCATOR 0
#define DESTROY_ALLOCATOR 1
#define ALLOCATE_NEW_OBJECT 2
#define MARK 3

#define START 0
#define OK 1
#define ERROR -1

typedef int8_t log_t;

static double log_time();

static void add_log_line(const FILE* file, const char* format, ...);

void set_memory_limit(size_t heap_size, size_t percent);

void log(log_t type, log_t result);

int check_the_space(size_t object_size);

#endif
#include <stddef.h>
#pragma once

extern size_t start_rsp_value;

void before_main(void) __attribute__((constructor));
void push_registers_to_stack();
void pop_registers_from_stack();
void segment_traverse(size_t segment_start, size_t segment_end);
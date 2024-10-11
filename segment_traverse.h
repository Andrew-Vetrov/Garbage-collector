#pragma once

void before_main(void) __attribute__((constructor));
void push_registers_to_stack();
void pop_registers_from_stack();
void segment_traverse(size_t segment_start, size_t segment_end);
size_t createHeap(size_t length);
void testGC();
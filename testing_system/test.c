#include "../scanner/segment_traverse.h"
#include "../allocator/allocator.h"
#include <stdio.h>

typedef struct A { int* a; int* b; }A;

int main() {
	int* a = (int*)allocate_new_object(80);
	int* b = (int*)allocate_new_object(80);
	int* c = (int*)allocate_new_object(40);
	segment_traverse(end_rsp_value, start_rsp_value);
	
	//show_bitmap(a);	
	//printf("START = %llu\nEND = %llu\n%llu\n", START_ALLOCATOR_HEAP, END_ALLOCATOR_HEAP, a);
	return 0;

}
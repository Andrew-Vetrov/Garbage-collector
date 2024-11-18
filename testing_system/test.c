#include "../scanner/segment_traverse.h"
#include "../allocator/allocator.h"
#include <stdio.h>

typedef struct A { int* a1; int* a2; int* a3; int* a4; }A;

int main() {
	A* a = (A*)allocate_new_object(sizeof(A));
	a->a1 = (int*)allocate_new_object(24);
	a->a2 = (int*)allocate_new_object(32);
	a->a3 = (int*)allocate_new_object(64);
	a->a4 = (int*)allocate_new_object(128);
	segment_traverse(end_rsp_value, start_rsp_value);
	printf("\n\n\n");
	printf("%llu %llu\n", ((size_t*)a->a1 - (size_t*)a), ((size_t*)a->a2 - (size_t*)a));
	printf("size = %d\n", get_object_size_by_address(a + 64*4));
	//show_bitmap(a);	
	//printf("START = %llu\nEND = %llu\n%llu\n", START_ALLOCATOR_HEAP, END_ALLOCATOR_HEAP, a);
	return 0;

}
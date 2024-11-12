#include "../scanner/segment_traverse.h"
#include "../allocator/allocator.h"
#include <stdio.h>

typedef struct A { int* a; int* b; }A;

int main() {
	//A* a = (A*)allocate_new_object(sizeof(A));
	//A* c = (A*)allocate_new_object(sizeof(A));
	//A* b = (A*)allocate_new_object(sizeof(A));
	//printf("%p\n", &a);
	int* a = allocate_new_object(40);
	segment_traverse(end_rsp_value, start_rsp_value);
	return 0;
}
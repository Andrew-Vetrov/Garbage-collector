#include "../scanner/segment_traverse.h"
#include "../allocator/allocator.h"
#include <stdio.h>

typedef struct A { int* a1; int* a2; int* a3; int* a4; }A;

int main() {
	A** a = allocate_new_object(64);
	for (int i = 0; i < 8; i++) {
		if (i % 2 == 0) {
			allocate_new_object(32);
			continue;
		}
		a[i] = allocate_new_object(32);
	}
	segment_traverse(end_rsp_value, start_rsp_value);
	show_bitmap((size_t)*a);
	return 0;

}
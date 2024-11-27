#include "../scanner/marking.h"
#include "../allocator/allocator.h"
#include <stdio.h>

typedef struct A { int* a1; int* a2; int* a3; int* a4; }A;

int main() {
	A* a[8 * 16];
	for (int i = 0; i < 8 * 16; i++) {
		if (i % 2 == 1) {
			allocate_new_object(32);
			continue;
		}
		a[i] = (A*)allocate_new_object(32);
	}
	segment_traverse(end_rsp_value, start_rsp_value);
	show_bitmap((size_t*)*a);
	return 0;
}
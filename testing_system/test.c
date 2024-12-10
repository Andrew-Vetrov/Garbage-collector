#include "../scanner/marking.h"
#include "../allocator/allocator.h"
#include <stdio.h>
#define GET_SIZE_WITH_ALIGNMENT(size) ((((size) % (8) == (0)) ? (0) : (8) - ((size) % (8))) + (size))
typedef struct A { int* a1; int* a2; int* a3; int* a4; }A;

int main() {
	A* a[8 * 15];
	for (int i = 0; i < 8 * 15; i++) {
		a[i] = allocate_new_object(32);
		
	}
	full_marking();
	show_bitmap((size_t)a[0]);
	printf("%p\n", START_ALLOCATOR_HEAP);
	printf("%p\n", a[0]);
	return 0;
}
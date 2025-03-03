#include "../scanner/marking.h"
#include "../allocator/allocator.h"
#include <stdio.h>

int main() {
	const int size = 28;
	void* a[size];
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			a[j] = allocate_new_object(16);
		}
	}

	for (int i = 0; i < 20; i++) {
		full_marking();
		sweep();
	}

	printf("GC amount = %d\n", counter);
	return 0;
}

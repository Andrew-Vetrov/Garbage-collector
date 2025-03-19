#include "../scanner/marking.h"
#include "../allocator/allocator.h"
#include <stdio.h>

int main() {
	const int size = 10000;
	void* a[size];
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			a[j] = gc_malloc(16);
		}
	}
	return 0;
}

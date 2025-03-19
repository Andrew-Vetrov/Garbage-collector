#include "../scanner/marking.h"
#include "../allocator/allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	const int size = 1000;
	void* a[size][size];
	for (int i = 0; i < size*size; i++) {
		for (int j = 0; j < size; j++) {
			a[i%size][j] = gc_malloc(16);
		}
		for (int j = 0; j < size; j+=2) {
			a[i%size][j] = NULL;
		}
	}
	return 0;
}
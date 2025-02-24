#include "../scanner/marking.h"
#include "../allocator/allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	const int size = 20000;
	void* a[size];
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			a[j] = allocate_new_object(16);
		}
		for (int j = 0; j < size; j+=2) {
			a[j] = NULL;
		}
	}
	printf("GC amount = %d\n", counter);
	return 0;
}
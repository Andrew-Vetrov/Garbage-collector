#include "../allocator/allocator.h"
#include <stdio.h>
#include <stdlib.h>


int main() {
	for (int i = 0; i < 10000000000000000000; i++) {
		allocate_new_object(1);
	}

	return 0;
}
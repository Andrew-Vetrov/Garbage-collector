#include "../allocator/allocator.h"
#include <stdio.h>
#include <stdlib.h>


int main() {
	for (int i = 0; i < 100; i++) {
		allocate_new_object(1000);
	}
	
	return 0;
}
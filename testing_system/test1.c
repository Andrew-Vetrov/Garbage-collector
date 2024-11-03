#include <stdio.h>
#include <stdlib.h>
#include "../allocator/allocator.h"

int main() {
	init_allocator();
	allocate_new_object(123123);
	return 0;
}
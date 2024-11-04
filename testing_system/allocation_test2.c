#include "../allocator/allocator.h"
#include <stdio.h>
#include <stdlib.h>


int main() {
	allocate_new_object(1000);
	allocate_new_object(1000);
	return 0;
}
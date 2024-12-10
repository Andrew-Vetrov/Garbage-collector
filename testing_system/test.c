#include "../scanner/marking.h"
#include "../allocator/allocator.h"
#include <stdio.h>

int main() {
	for (int i = 0; i < 10000000; i++) {
		allocate_new_object(10);
	}
	//full_marking();

	return 0;
}
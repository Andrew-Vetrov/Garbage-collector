#include <sys/mman.h>
#include <stdio.h>

#define HEAP_SIZE (512 * 1024 * (size_t) 1024)
#define BLOCK_SIZE (4 * (size_t) 1024)

static size_t* START_ALLOCATOR_HEAP = NULL;
static size_t* NEW_BLOCK_PTR = NULL;

void init_allocator() {
	if (
		(START_ALLOCATOR_HEAP = mmap(NULL, HEAP_SIZE, PROT_WRITE | PROT_READ, \
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED
	) {
		fprintf(stderr, "Can't allocate allocator's heap!\n");
	}
}

size_t* allocate_new_block() {
	if (NEW_BLOCK_PTR + BLOCK_SIZE >= START_ALLOCATOR_HEAP + HEAP_SIZE) {
		fprintf(stderr, "Heap is over!\n");
		return NULL;
	} else {
		size_t* result = NEW_BLOCK_PTR;

		*NEW_BLOCK_PTR = (size_t) ((size_t) NEW_BLOCK_PTR + (size_t) 8); // ptr to new object space
		NEW_BLOCK_PTR += BLOCK_SIZE;

		return result;
	}
}

void destroy_allocator() {
	if (
		munmap((void*) START_ALLOCATOR_HEAP, HEAP_SIZE) == -1
	) {
		fprintf(stderr, "Can't unmap heap!\n");
	}
}

int main() {
	init_allocator();

	printf("%p\n", START_ALLOCATOR_HEAP);	
	
	
	for (size_t* ptr = START_ALLOCATOR_HEAP; (size_t) (ptr - START_ALLOCATOR_HEAP) < (size_t) HEAP_SIZE; ptr++) {
		printf("%ld\n", *ptr);
	}
	
	
	printf("Heap is reacheable\n");

	/*
	for (size_t i = 0; i < HEAP_SIZE / BLOCK_SIZE; i++) {
		allocate_new_block();
	}
	*/

	destroy_allocator();
	return 0;
}

#include <sys/mman.h>
#include <stdio.h>

#define HEAP_SIZE (513 * 1024 * (size_t) 1024)
#define BLOCK_SIZE (5 * (size_t) 1024)
#define MAX_OBJECT_SIZE (2 * 1024)

typedef struct Node_t {
	size_t start_allocator_ptr;
	struct Node_t* next_node;
} Node;

static size_t START_ALLOCATOR_HEAP = 0;
static Node NODES_LIST[HEAP_SIZE / BLOCK_SIZE];
static Node* SEGREG_LIST[MAX_OBJECT_SIZE] = {0};
static Node* EMPTY_LIST_HEAD = 0;

__attribute__((constructor))
void init_allocator() {
	START_ALLOCATOR_HEAP =
		(size_t) mmap(NULL, HEAP_SIZE,
				PROT_WRITE | PROT_READ,MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	
	if (START_ALLOCATOR_HEAP == MAP_FAILED) {
		fprintf(stderr, "Can't allocate allocator's heap!\n");
		return;
	}

	int nodes_count = HEAP_SIZE / BLOCK_SIZE;

	for (int i = 0; i < nodes_count; i++) {
		NODES_LIST[i].start_allocator_ptr = 
			START_ALLOCATOR_HEAP + BLOCK_SIZE * i;
		*(size_t*)NODES_LIST[i].start_allocator_ptr = 
			NODES_LIST[i].start_allocator_ptr + 8;
		if (i != nodes_count - 1) {
			NODES_LIST[i].next_node = &NODES_LIST[i + 1];
		} else {
			NODES_LIST[i].next_node = 0;
		}
	}

    EMPTY_LIST_HEAD = &NODES_LIST[0];
}

Node* allocate_new_block() {
	if (EMPTY_LIST_HEAD == NULL) {
		fprintf(stderr, "No empty blocks in garbage collector!\n");
		return NULL;
	} else {
		Node* result = EMPTY_LIST_HEAD;
		result->next_node = NULL;
		EMPTY_LIST_HEAD = EMPTY_LIST_HEAD->next_node;
		return result;
	}
}

__attribute__((destructor))
void destroy_allocator() {
	if (munmap((void*) START_ALLOCATOR_HEAP, HEAP_SIZE) == -1) {
		fprintf(stderr, "Can't unmap heap!\n");
	}
}

int main() {
	printf("%p\n", (size_t*) START_ALLOCATOR_HEAP);	

	for (
		size_t ptr = START_ALLOCATOR_HEAP;
		ptr - START_ALLOCATOR_HEAP < HEAP_SIZE; 
		ptr += 8
	) {
		*(size_t*)ptr = 0;
	}
	
	printf("Heap is reacheable\n");

	for (int i = 0; i < HEAP_SIZE / BLOCK_SIZE; i++) {
		allocate_new_block();
	}

	allocate_new_block();

	return 0;
}

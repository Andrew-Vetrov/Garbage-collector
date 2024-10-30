#include <sys/mman.h>
#include <stdio.h>

#define HEAP_SIZE (512 * 1024 * (size_t) 1024)
#define BLOCK_SIZE (4 * (size_t) 1024)
#define BLOCK_HEADER_SIZE (8)
#define MAX_OBJECT_SIZE (2040)

typedef struct Node_t {
	size_t start_allocator_ptr;
	struct Node_t* next_node;
} Node;

size_t START_ALLOCATOR_HEAP = 0;
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
			NODES_LIST[i].start_allocator_ptr + BLOCK_HEADER_SIZE;
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
		EMPTY_LIST_HEAD = EMPTY_LIST_HEAD->next_node;
		result->next_node = NULL;
		return result;
	}
}

__attribute__((destructor))
void destroy_allocator() {
	if (munmap((void*) START_ALLOCATOR_HEAP, HEAP_SIZE) == -1) {
		fprintf(stderr, "Can't unmap heap!\n");
	}
}

size_t allocate_new_object(size_t object_size) {
	if (object_size > MAX_OBJECT_SIZE) {
		fprintf(stderr, "Size of object is too large\n");
		return NULL;
	}

	Node* current_entry = SEGREG_LIST[object_size];

	if (current_entry == NULL) {
		if ((current_entry = SEGREG_LIST[object_size] = allocate_new_block()) == NULL) {
			fprintf(stderr, "Can't allocate new object!\n");
			return NULL;
		}
	}

	while (current_entry->next_node != NULL) {
		current_entry = current_entry->next_node;
	}

	size_t block_start = current_entry->start_allocator_ptr;
	size_t first_free_space = *(size_t *)block_start;
	size_t next_block_start = block_start + BLOCK_SIZE;
	size_t addition = (object_size % 8 == 0) ? 0 : 8 - (object_size % 8);

	if (first_free_space + object_size + addition > next_block_start) {
		if ((current_entry->next_node = allocate_new_block()) == NULL) {
			fprintf(stderr, "Can't allocate new object!\n");
			return NULL;
		}
		
		current_entry = current_entry->next_node;

		block_start = current_entry->start_allocator_ptr;
		first_free_space = *(size_t *)block_start;
	}

	*(size_t *)block_start = first_free_space + object_size + addition;
	return first_free_space;
}


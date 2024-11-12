#include <sys/mman.h>
#include <stdio.h>

#define HEAP_SIZE (512 * 1024 * (size_t) 1024)
#define BLOCK_SIZE (4 * (size_t) 1024)
#define BLOCK_HEADER_SIZE (80)
#define MAX_OBJECT_SIZE (2008)

typedef struct Node_t {
	size_t start_allocator_ptr;
	struct Node_t* next_node;
} Node;

size_t START_ALLOCATOR_HEAP = 0;
size_t end_rsp_value;
size_t END_ALLOCATOR_HEAP;
static Node NODES_LIST[HEAP_SIZE / BLOCK_SIZE];
static Node* SEGREG_LIST[MAX_OBJECT_SIZE] = {0};
static Node* EMPTY_LIST_HEAD = 0;

void init_header(Node* entry, size_t object_size) {
	size_t object_size_header = entry->start_allocator_ptr + 8;
	*(size_t *)object_size_header = object_size;
	
	size_t bitmap_start = object_size_header + 8;
	*(size_t *)bitmap_start = 0;
	for (int j = 0; j < 7; j++) {
		bitmap_start += 8;
		*(size_t *)bitmap_start = 0;
	}
}

void fill_all_bitmaps_with_zeros() {
	Node* current_entry;
	for (int i = 0; i < MAX_OBJECT_SIZE; i++) {
		current_entry = SEGREG_LIST[i];
		while (current_entry != NULL) {
			size_t bitmap_start = current_entry->start_allocator_ptr + 16;
			
			*(size_t *)bitmap_start = 0;
			for (int j = 0; j < 7; j++) {
				bitmap_start += 8;
				*(size_t *)bitmap_start = 0;
			}

			current_entry = current_entry->next_node;
		}
	}
}

int get_bit_by_address(size_t object_address) {
	size_t relative_address = object_address - START_ALLOCATOR_HEAP;
	size_t block_start = object_address - (relative_address % BLOCK_SIZE);
	size_t objects_start = block_start + BLOCK_HEADER_SIZE;
	size_t bitmap_start = block_start + 16;

	size_t object_size = *(size_t *)(block_start + 8);
	size_t addition = (object_size % 8 == 0) ? 0 : 8 - (object_size % 8);
	object_size += addition;

	int object_index = (object_address - objects_start) / object_size;

	size_t byte_position = bitmap_start + (object_index / 8);
	int bit_position = object_index % 8;

	return ((*(unsigned char *)byte_position) >> bit_position) & 1;
}

void set_bit_by_address(size_t object_address, int bit) {
	size_t relative_address = object_address - START_ALLOCATOR_HEAP;
	size_t block_start = object_address - (relative_address % BLOCK_SIZE);
	size_t objects_start = block_start + BLOCK_HEADER_SIZE;
	size_t bitmap_start = block_start + 16;

	size_t object_size = *(size_t *)(block_start + 8);
	size_t addition = (object_size % 8 == 0) ? 0 : 8 - (object_size % 8);
	object_size += addition;

	int object_index = (object_address - objects_start) / object_size;

	size_t byte_position = bitmap_start + (object_index / 8);
	int bit_position = object_index % 8;

	if ((((*(unsigned char *)byte_position) >> bit_position) & 1) != bit) {
		*(unsigned char *)byte_position = (*(unsigned char *)byte_position) ^ (1 << bit_position);
	}
}

size_t get_object_size_by_address(size_t object_address) {
	size_t relative_address = object_address - START_ALLOCATOR_HEAP;
	size_t block_start = object_address - (relative_address % BLOCK_SIZE);
	size_t object_size = *(size_t *)(block_start + 8);
    
    return object_size;
}

__attribute__((constructor))
void init_allocator() {
	START_ALLOCATOR_HEAP =
		(size_t) mmap(NULL, HEAP_SIZE,
				PROT_WRITE | PROT_READ,MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	
	if (START_ALLOCATOR_HEAP == MAP_FAILED) {
		fprintf(stderr, "Can't allocate allocator's heap!\n");
		return;
	}
	END_ALLOCATOR_HEAP = START_ALLOCATOR_HEAP + HEAP_SIZE;
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
	asm volatile("mov %%rsp, %0" : "=r" (end_rsp_value));
	Node* current_entry = SEGREG_LIST[object_size];
	Node *prev_entry;

	size_t addition = (object_size % 8 == 0) ? 0 : 8 - (object_size % 8);
	size_t block_start;
	size_t free_space_address;
	size_t next_block_start;

	if (current_entry == NULL) {
		if ((current_entry = SEGREG_LIST[object_size] = allocate_new_block()) == NULL) {
			fill_all_bitmaps_with_zeros();
			// call GC and then try again to allocate new object?
			// if couldn't allocate - error

			// put your code here :))

		} else {
			init_header(current_entry, object_size);
		}
	}

	while (current_entry != NULL) {
		block_start = current_entry->start_allocator_ptr;
		free_space_address = *(size_t *)block_start;
		next_block_start = block_start + BLOCK_SIZE;

		while (free_space_address + object_size + addition <= next_block_start) {
			if (get_bit_by_address(free_space_address) == 0) {
				*(size_t *)block_start = free_space_address + object_size + addition;
				return free_space_address;
			} else {
				set_bit_by_address(free_space_address, 0);
				free_space_address = free_space_address + object_size + addition;
			}
		}

		*(size_t *)block_start = free_space_address;

		prev_entry = current_entry;
		current_entry->next_node;
	}

	current_entry = prev_entry;

	if ((current_entry->next_node = allocate_new_block()) == NULL) {
		fill_all_bitmaps_with_zeros();
		// call GC and then try again to allocate new object?
		// if couldn't allocate - error

		// put your code here :))

	} else {
		current_entry = current_entry->next_node;

		init_header(current_entry, object_size);

		block_start = current_entry->start_allocator_ptr;
		free_space_address = *(size_t *)block_start;

		*(size_t *)block_start = free_space_address + object_size + addition;
		return free_space_address;
	}
}

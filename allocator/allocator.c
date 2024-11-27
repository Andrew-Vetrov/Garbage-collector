#include <sys/mman.h>
#include <stdio.h>

#define GET_BITMAP_ADDR(block_addr) ((block_addr) + (16))
#define GET_OBJECT_SIZE_ADDR(block_addr) ((block_addr) + (8))
#define GET_SLIDER_POSITION_ADDR(block_addr) ((block_addr) + (0))
#define GET_SIZE_WITH_ALIGNMENT(size) ((((size) % (8) == (0)) ? (0) : (8) - ((size) % (8))) + (size))

#define HEAP_SIZE (512 * 1024 * (size_t) 1024)
#define BLOCK_SIZE (4 * (size_t) 1024)
#define BLOCK_HEADER_SIZE (80)
#define MAX_OBJECT_SIZE (2008)
#define BITMAP_BYTES_COUNT (64)

typedef struct Node_t {
	size_t block_addr;
	struct Node_t* next_node;
} Node;

size_t START_ALLOCATOR_HEAP = 0;
size_t end_rsp_value;
size_t END_ALLOCATOR_HEAP;
static Node NODES_LIST[HEAP_SIZE / BLOCK_SIZE];
static Node* SEGREG_LIST[MAX_OBJECT_SIZE + 1] = {0};
static Node* EMPTY_LIST_HEAD = 0;

void show_bitmap(size_t object_addr) {
	size_t relative_object_addr = object_addr - START_ALLOCATOR_HEAP;
	size_t block_addr = object_addr - (relative_object_addr % BLOCK_SIZE);
	size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

	unsigned char curr_byte;

	for (int i = 0; i < BITMAP_BYTES_COUNT; i++) {
		printf("byte's index = %2d. bits: ", i);
		curr_byte = *(unsigned char *)(bitmap_addr + i);
		for (int j = 0; j < sizeof(curr_byte) * 8; j++) {
			putchar(((curr_byte >> j) & 1) == 1 ? '1' : '0');
		}
		putchar('\n');
	}
}

void init_header(Node* entry, size_t object_size) {
	size_t block_addr = entry->block_addr;

	size_t object_size_addr = GET_OBJECT_SIZE_ADDR(block_addr);
	*(size_t *)object_size_addr = object_size;
	
	size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);
	size_t curr_bytes_addr = bitmap_addr;

	for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
		*(size_t *)curr_bytes_addr = 0;
		curr_bytes_addr += sizeof(size_t);
	}
}

void fill_all_bitmaps_with_zeros() {
	Node* curr_entry;
	for (int i = 1; i <= MAX_OBJECT_SIZE; i++) {
		curr_entry = SEGREG_LIST[i];
		while (curr_entry != NULL) {
			size_t bitmap_addr = GET_BITMAP_ADDR(curr_entry->block_addr);
			size_t curr_bytes_addr = bitmap_addr;

			for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
				*(size_t *)curr_bytes_addr = 0;
				curr_bytes_addr += sizeof(size_t);
			}

			curr_entry = curr_entry->next_node;
		}
	}
}

unsigned char get_bit_by_address(size_t object_addr) {
	size_t object_relative_addr = object_addr - START_ALLOCATOR_HEAP;
	size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
	size_t objects_addr = block_addr + BLOCK_HEADER_SIZE;
	size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

	size_t object_size = *(size_t *)GET_OBJECT_SIZE_ADDR(block_addr);
	size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

	int object_index = (object_addr - objects_addr) / object_size_with_alignment;

	size_t byte_position = bitmap_addr + (object_index / (sizeof(unsigned char) * 8));
	size_t bit_position = object_index % (sizeof(unsigned char) * 8);

	return (unsigned char)(((*(unsigned char *)byte_position) >> bit_position) & 1);
}

void set_bit_by_address(size_t object_addr, unsigned char bit) {
	size_t object_relative_addr = object_addr - START_ALLOCATOR_HEAP;
	size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
	size_t objects_addr = block_addr + BLOCK_HEADER_SIZE;
	size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

	size_t object_size = *(size_t *)GET_OBJECT_SIZE_ADDR(block_addr);
	size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

	int object_index = (object_addr - objects_addr) / object_size_with_alignment;

	size_t byte_position = bitmap_addr + (object_index / (sizeof(unsigned char) * 8));
	size_t bit_position = object_index % (sizeof(unsigned char) * 8);

	if ((((*(unsigned char *)byte_position) >> bit_position) & 1) != bit) {
		*(unsigned char *)byte_position = (*(unsigned char *)byte_position) ^ (1 << bit_position);
	}
}

<<<<<<< HEAD
<<<<<<< HEAD
=======
unsigned char is_bitmap_empty(size_t block_address) {
    size_t bitmap_addr = block_address + 16;
    for (int i = 0; i < 7; i++) { // TODO: make a define for 7
        if (*(size_t*) bitmap_addr != 0) {
=======
unsigned char is_bitmap_empty(size_t block_addr) {
    size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);
	size_t curr_bytes_addr = bitmap_addr;

	for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
		if (*(size_t*)curr_bytes_addr != 0) {
>>>>>>> adf28cd7dd9b164b103770e8d2b5dfa0476a0cf0
            return 1;
        }
		curr_bytes_addr += sizeof(size_t);
	}

    return 0;
}

<<<<<<< HEAD
>>>>>>> 5e72580193872c2b968b91d321357f17a34fdda2
size_t get_object_size_by_address(size_t object_address) {
	size_t relative_address = object_address - START_ALLOCATOR_HEAP;
	size_t block_start = object_address - (relative_address % BLOCK_SIZE);
	size_t object_size = *(size_t *)(block_start + 8);
    
=======
size_t get_object_size_by_address(size_t object_addr) {
	size_t object_relative_addr = object_addr - START_ALLOCATOR_HEAP;
	size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
	size_t object_size = *(size_t *)GET_OBJECT_SIZE_ADDR(block_addr);

>>>>>>> adf28cd7dd9b164b103770e8d2b5dfa0476a0cf0
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
		NODES_LIST[i].block_addr = 
			START_ALLOCATOR_HEAP + BLOCK_SIZE * i;
		*(size_t*)NODES_LIST[i].block_addr = 
			NODES_LIST[i].block_addr + BLOCK_HEADER_SIZE;
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
<<<<<<< HEAD
	asm volatile("mov %%rsp, %0" : "=r" (end_rsp_value));
	Node* current_entry = SEGREG_LIST[object_size];
	Node *prev_entry;
=======

	Node* curr_entry = SEGREG_LIST[object_size];
>>>>>>> adf28cd7dd9b164b103770e8d2b5dfa0476a0cf0

	size_t block_addr;
	size_t slider_position;
	size_t next_block_addr;
	size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

<<<<<<< HEAD
<<<<<<< HEAD
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
=======
	size_t addition = (object_size % 8 == 0) ? 0 : 8 - (object_size % 8);
	size_t block_start;
	size_t free_space_address;
	size_t next_block_start;

	while (current_entry != NULL) {
		block_start = current_entry->start_allocator_ptr;
		free_space_address = *(size_t *)block_start;
		next_block_start = block_start + BLOCK_SIZE;
=======
	while (curr_entry != NULL) {
		block_addr = curr_entry->block_addr;
		slider_position = *(size_t *)GET_SLIDER_POSITION_ADDR(block_addr);
		next_block_addr = block_addr + BLOCK_SIZE;
>>>>>>> adf28cd7dd9b164b103770e8d2b5dfa0476a0cf0

		while (slider_position + object_size_with_alignment <= next_block_addr) {
			if (get_bit_by_address(slider_position) == 0) {
				*(size_t *)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position + object_size_with_alignment;
				return slider_position;
			} else {
				set_bit_by_address(slider_position, 0);
				slider_position = slider_position + object_size_with_alignment;
			}
		}

		*(size_t *)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position;

		curr_entry = SEGREG_LIST[object_size] = curr_entry->next_node;
	}

<<<<<<< HEAD
	if ((current_entry = SEGREG_LIST[object_size] = allocate_new_block()) == NULL) {
>>>>>>> 5e72580193872c2b968b91d321357f17a34fdda2
=======
	if ((curr_entry = SEGREG_LIST[object_size] = allocate_new_block()) == NULL) {
>>>>>>> adf28cd7dd9b164b103770e8d2b5dfa0476a0cf0
		fill_all_bitmaps_with_zeros();
		// call GC and then try to allocate new object again?
		// if couldn't allocate - error

		// put your code here :))

	} else {
<<<<<<< HEAD
<<<<<<< HEAD
		current_entry = current_entry->next_node;
=======
		init_header(current_entry, object_size);
>>>>>>> 5e72580193872c2b968b91d321357f17a34fdda2

		init_header(current_entry, object_size);
=======
		init_header(curr_entry, object_size);
>>>>>>> adf28cd7dd9b164b103770e8d2b5dfa0476a0cf0

		block_addr = curr_entry->block_addr;
		slider_position = *(size_t *)GET_SLIDER_POSITION_ADDR(block_addr);

		*(size_t *)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position + object_size_with_alignment;
		return slider_position;
	}
}

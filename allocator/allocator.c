#include <sys/mman.h>
#include <stdio.h>
#include <stdbool.h>

#define GET_BITMAP_ADDR(block_addr) ((block_addr) + (16))
#define GET_OBJECT_SIZE_ADDR(block_addr) ((block_addr) + (8))
#define GET_SLIDER_POSITION_ADDR(block_addr) ((block_addr) + (0))
#define GET_SIZE_WITH_ALIGNMENT(size) \
    ((((size) % (8) == (0)) ? (0) : (8) - ((size) % (8))) + (size))

#define HEAP_SIZE (512 * 1024 * (size_t) 1024)
#define BLOCK_SIZE (4 * (size_t) 1024)
#define BLOCK_HEADER_SIZE (80)
#define MAX_OBJECT_SIZE (2008)
#define OBJECT_SIZE_UPPER_BOUND (MAX_OBJECT_SIZE + 1)
#define BITMAP_BYTES_COUNT (64)
#define BLOCKS_COUNT (HEAP_SIZE / BLOCK_SIZE)
int counter = 0;
typedef struct Node_t {
	size_t block_addr;
	struct Node_t* next_node;
} Node;

size_t START_ALLOCATOR_HEAP = 0;
size_t END_ALLOCATOR_HEAP = 0;
static Node* SEGREG_LIST[OBJECT_SIZE_UPPER_BOUND] = { 0 };
static Node NODES_LIST[BLOCKS_COUNT];
static Node* EMPTY_LIST_HEAD = 0;
size_t end_rsp_value;

void show_bitmap(size_t object_addr) {
	size_t relative_object_addr = object_addr - START_ALLOCATOR_HEAP;
	size_t block_addr = object_addr - (relative_object_addr % BLOCK_SIZE);
	size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

	unsigned char curr_byte;

	for (int i = 0; i < BITMAP_BYTES_COUNT; i++) {
		printf("byte's index = %2d. bits: ", i);
		curr_byte = *(unsigned char*)(bitmap_addr + i);
		for (int j = 0; j < sizeof(curr_byte) * 8; j++) {
			putchar(((curr_byte >> j) & 1) == 1 ? '1' : '0');
		}
		putchar('\n');
	}
}

size_t get_block_addr(size_t object_addr) {
	size_t object_relative_addr = object_addr - START_ALLOCATOR_HEAP;
	return (object_addr - (object_relative_addr % BLOCK_SIZE));
}

void init_header(Node* entry, size_t object_size) {
	size_t block_addr = entry->block_addr;

	size_t object_size_addr = GET_OBJECT_SIZE_ADDR(block_addr);
	*(size_t*)object_size_addr = object_size;

	size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);
	size_t curr_bytes_addr = bitmap_addr;

	for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
		*(size_t*)curr_bytes_addr = 0;
		curr_bytes_addr += sizeof(size_t);
	}
}

void fill_all_bitmaps_with_zeros() {
	Node* curr_entry;
	for (int i = 1; i < OBJECT_SIZE_UPPER_BOUND; i++) {
		curr_entry = SEGREG_LIST[i];
		while (curr_entry != NULL) {
			size_t bitmap_addr = GET_BITMAP_ADDR(curr_entry->block_addr);
			size_t curr_bytes_addr = bitmap_addr;

			for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
				*(size_t*)curr_bytes_addr = 0;
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

	size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);
	size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

	int object_index = (object_addr - objects_addr) / object_size_with_alignment;

	size_t byte_position = bitmap_addr + (object_index / (sizeof(unsigned char) * 8));
	size_t bit_position = object_index % (sizeof(unsigned char) * 8);
	//printf("BIT = %d\n", (unsigned char)(((*(unsigned char*)byte_position) >> bit_position) & 1));

	return (unsigned char)(((*(unsigned char*)byte_position) >> bit_position) & 1);
}

void set_bit_by_address(size_t object_addr, unsigned char bit) {
	size_t object_relative_addr = object_addr - START_ALLOCATOR_HEAP;
	size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
	size_t objects_addr = block_addr + BLOCK_HEADER_SIZE;
	size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);

	size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);
	size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

	int object_index = (object_addr - objects_addr) / object_size_with_alignment;

	size_t byte_position = bitmap_addr + (object_index / (sizeof(unsigned char) * 8));
	size_t bit_position = object_index % (sizeof(unsigned char) * 8);

	if ((((*(unsigned char*)byte_position) >> bit_position) & 1) != bit) {
		*(unsigned char*)byte_position = (*(unsigned char*)byte_position) ^ (1 << bit_position);
	}
}

bool is_bitmap_empty(size_t block_addr) {
	size_t bitmap_addr = GET_BITMAP_ADDR(block_addr);
	size_t curr_bytes_addr = bitmap_addr;

	for (int j = 0; j < BITMAP_BYTES_COUNT / sizeof(size_t); j++) {
		if (*(size_t*)curr_bytes_addr != 0) {
            return false;
        }
		curr_bytes_addr += sizeof(size_t);
	}

    return true;
}

size_t get_object_size_by_address(size_t object_addr) {
	size_t object_relative_addr = object_addr - START_ALLOCATOR_HEAP;
	size_t block_addr = object_addr - (object_relative_addr % BLOCK_SIZE);
	size_t object_size = *(size_t*)GET_OBJECT_SIZE_ADDR(block_addr);

	return object_size;
}

__attribute__((constructor))
void init_allocator() {
	START_ALLOCATOR_HEAP =
		(size_t)mmap(NULL, HEAP_SIZE,
			PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

	if (START_ALLOCATOR_HEAP == MAP_FAILED) {
		fprintf(stderr, "Can't allocate allocator's heap!\n");
		return;
	}

    END_ALLOCATOR_HEAP = START_ALLOCATOR_HEAP + HEAP_SIZE;

	for (int i = 0; i < BLOCKS_COUNT; i++) {
		NODES_LIST[i].block_addr = 
			START_ALLOCATOR_HEAP + BLOCK_SIZE * i;
		*(size_t*)NODES_LIST[i].block_addr =
			NODES_LIST[i].block_addr + BLOCK_HEADER_SIZE;
		if (i != BLOCKS_COUNT - 1) {
			NODES_LIST[i].next_node = &NODES_LIST[i + 1];
		}
		else {
			NODES_LIST[i].next_node = 0;
		}
	}

	EMPTY_LIST_HEAD = &NODES_LIST[0];
}

Node* allocate_new_block() {
	if (EMPTY_LIST_HEAD == NULL) {
		//fprintf(stderr, "No empty blocks in garbage collector!\n");
		return NULL;
	}
	else {
		Node* result = EMPTY_LIST_HEAD;
		EMPTY_LIST_HEAD = EMPTY_LIST_HEAD->next_node;
		result->next_node = NULL;
		return result;
	}
}

__attribute__((destructor))
void destroy_allocator() {
	if (munmap((void*)START_ALLOCATOR_HEAP, HEAP_SIZE) == -1) {
		fprintf(stderr, "Can't unmap heap!\n");
	}
}

void sweep() {
#ifdef DEBUG
    int empty_nodes_count = 0;
    int segreg_list_nodes_count = 0;
#endif
    EMPTY_LIST_HEAD = NULL;
    for (int i = 0; i < OBJECT_SIZE_UPPER_BOUND; i++) {
        SEGREG_LIST[i] = NULL;
    }
 
    for (int i = 0; i < BLOCKS_COUNT; i++) {
        *(size_t*) GET_SLIDER_POSITION_ADDR(NODES_LIST[i].block_addr) =
            NODES_LIST[i].block_addr + BLOCK_HEADER_SIZE;
        NODES_LIST[i].next_node = NULL;
        if (is_bitmap_empty(NODES_LIST[i].block_addr)) {
#ifdef DEBUG
            empty_nodes_count++;
#endif
            *(size_t*) GET_OBJECT_SIZE_ADDR(NODES_LIST[i].block_addr) = 0;
            if (EMPTY_LIST_HEAD == NULL) {
                EMPTY_LIST_HEAD = &NODES_LIST[i];
            } else {
                NODES_LIST[i].next_node = EMPTY_LIST_HEAD;
                EMPTY_LIST_HEAD = &NODES_LIST[i];
            }
        } else {
#ifdef DEBUG
            segreg_list_nodes_count++;
#endif
            size_t object_size = 
                *(size_t*) GET_OBJECT_SIZE_ADDR(NODES_LIST[i].block_addr);
            if (SEGREG_LIST[object_size] == NULL) {
                SEGREG_LIST[object_size] = &NODES_LIST[i];
            } else {
                NODES_LIST[i].next_node = SEGREG_LIST[object_size];
                SEGREG_LIST[object_size] = &NODES_LIST[i];
            }
        }
    }
#ifdef DEBUG
    printf("empty_nodes_count = %d\nsegreg_list_nodes_count = %d\n", 
            empty_nodes_count, segreg_list_nodes_count);
#endif
}

int cnt = 0;
size_t allocate_new_object(size_t object_size) {
	if (object_size > MAX_OBJECT_SIZE) {
		fprintf(stderr, "Size of object is too large\n");
		return NULL;
	}
	asm volatile("mov %%rsp, %0" : "=r" (end_rsp_value));
	Node* curr_entry = SEGREG_LIST[object_size];

	size_t block_addr;
	size_t slider_position;
	size_t next_block_addr;
	size_t object_size_with_alignment = GET_SIZE_WITH_ALIGNMENT(object_size);

	while (curr_entry != NULL) {
		block_addr = curr_entry->block_addr;
		slider_position = *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr);
		next_block_addr = block_addr + BLOCK_SIZE;

		while (slider_position + object_size_with_alignment <= next_block_addr) {
			if (get_bit_by_address(slider_position) == 0) {
				*(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position + object_size_with_alignment;
				return slider_position;
			}
			else {
				set_bit_by_address(slider_position, 0);
				slider_position = slider_position + object_size_with_alignment;
			}
		}

		*(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position;

		curr_entry = SEGREG_LIST[object_size] = curr_entry->next_node;
	}
	if ((curr_entry = SEGREG_LIST[object_size] = allocate_new_block()) == NULL) {
		printf("GC\n");
		fill_all_bitmaps_with_zeros();
		
		if (cnt == 1) {
			printf("Gone wrong\n");
		}
		cnt = 1;
		full_marking();
		sweep();
		counter++;
		return allocate_new_object(object_size);

	}
	else {
		cnt = 0;
		init_header(curr_entry, object_size);

		block_addr = curr_entry->block_addr;
		slider_position = *(size_t*)GET_SLIDER_POSITION_ADDR(block_addr);

		*(size_t*)GET_SLIDER_POSITION_ADDR(block_addr) = slider_position + object_size_with_alignment;
		return slider_position;
	}
}

#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

HashMap* create_hashmap() {
	HashMap* table = (HashMap*)malloc(sizeof(HashMap));

	table->count = 0;
	table->capacity = INITIAL_CAPACITY;

	table->chains = (hm_element*)malloc(table->capacity * sizeof(hm_element));

	for (int i = 0; i < table->capacity; i++)
		table->chains[i].next = NULL;

	table->a = rand(), table->b = rand();

	while (!table->a)
		table->a = rand();

	return table;
}

void destroy_hashmap(HashMap* table) {
	hm_element* ptr, *now;

	for (int i = 0; i < table->capacity; i++) {
		ptr = table->chains[i].next;

		while (ptr) {
			now = ptr;
			ptr = now->next;

			free(now);
		}
	}

	free(table->chains);
	free(table);
}

static int get_hash(HashMap* table, TYPE key) { // simple hash for example
	return abs(((long long)table->a * key) ^ table->b) % table->capacity;
}

bool get_block_hm(HashMap* table, TYPE key) { // !
	hm_element* ptr = &table->chains[get_hash(table, key)];

	while (ptr->next) {
		ptr = ptr->next;

		if (ptr->elem.key == key)
			return true;
	}

	return false;
}

static void insert_elem(HashMap* table, block_with_key elem) {
	hm_element* ptr = &table->chains[get_hash(table, elem.key)];

	while (ptr->next)
		ptr = ptr->next;
	
	ptr->next = (hm_element*)malloc(sizeof(hm_element));
	
	ptr->next->next = NULL;
	ptr->next->elem = elem;
}

static void realloc_hashmap(HashMap* table) {

	int indx = 0;
	block_with_key* all_elements = (block_with_key*)malloc(table->count * sizeof(block_with_key));
	for (int i = 0; i < table->count; i++) {
		hm_element* ptr = &table->chains[i];

		while (ptr->next) {
			all_elements[indx++] = ptr->elem;
			ptr = ptr->next;
		}
	}

	assert(indx == table->count);

	table->capacity *= 1.5;
	table->chains = (hm_element*)realloc(table->chains, table->capacity * sizeof(hm_element));

	for (int i = 0; i < table->capacity; i++)
		table->chains[i].next = NULL;

	for (int i = 0; i < indx; i++)
		insert_elem(table, all_elements[i]);

	free(all_elements);
}

void insert_block_hm(HashMap* table, void* block, TYPE key) {
	if (table->count == table->capacity)
		realloc_hashmap(table);
	
	block_with_key new_elem = {block, key};

	insert_elem(table, new_elem);
}

void delete_block_hm(HashMap* table, void* block, TYPE key) {
	hm_element* ptr = &table->chains[get_hash(table, key)];

	while (ptr->next) {
		if (ptr->next->elem.block == block) {
			free(ptr->next);
			table->count--;

			return;
		}

		ptr = ptr->next;
	}
}
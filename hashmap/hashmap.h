#pragma once

#ifndef _HASHMAP
#include <stdbool.h>

#define _HASHMAP

#define TYPE int
#define INITIAL_CAPACITY 8

#endif

typedef struct block_with_key {
	void* block;
	TYPE key;
} block_with_key;

typedef struct hm_element {
	struct hm_element* next;
	block_with_key elem;
} hm_element;

typedef struct HashMap {
	hm_element* chains;
	int count;
	int capacity;
	int a, b; // for get_hash() function
} HashMap;

HashMap* create_hashmap();

void destroy_hashmap(HashMap* table);

static int get_hash(HashMap* table, TYPE key);

bool get_block_hm(HashMap* table, TYPE key);

static void insert_elem(HashMap* table, block_with_key elem);

static void realloc_hashmap(HashMap* table);

void insert_block_hm(HashMap* table, void* block, TYPE key);

void delete_block_hm(HashMap* table, void* block, TYPE key);
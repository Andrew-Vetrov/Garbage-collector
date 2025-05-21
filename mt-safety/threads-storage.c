#include "threads-storage.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static StorageCell *created_threads_list = NULL;

pthread_mutex_t storage_lock =
    PTHREAD_MUTEX_INITIALIZER;  // lock for work with created_threads_list

static volatile unsigned int thread_count = 0;

pthread_mutex_t *get_storage_lock() { return &storage_lock; }

StorageCell *create_cell_for_thread() {
    StorageCell *new_node = (StorageCell *)calloc(1, sizeof(StorageCell));

    if (new_node == NULL) {
        fprintf(stderr,
                "garbage-collector: mt-safety: create_cell_for_thread: "
                "calloc() ended with an error\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&storage_lock);
    thread_count++;

    if (created_threads_list != NULL) {
        created_threads_list->prev = new_node;
    }

    new_node->next = created_threads_list;
    created_threads_list = new_node;
    pthread_mutex_unlock(&storage_lock);
    return new_node;
}

void destroy_cell(StorageCell *cell) {
    pthread_mutex_lock(&storage_lock);
    thread_count--;

    if (cell->prev) {
        cell->prev->next = cell->next;
    }
    if (cell->next) {
        cell->next->prev = cell->prev;
    }
    if (created_threads_list == cell) {
        created_threads_list = cell->next;
    }
    free(cell);

    pthread_mutex_unlock(&storage_lock);
}

__attribute__((destructor)) void __destroy_storage() {
    pthread_mutex_lock(&storage_lock);
    StorageCell *prev = NULL;
    for (StorageCell *node = created_threads_list; node; node = node->next) {
        free(prev);
        prev = node;
    }
    free(prev);
    created_threads_list = NULL;

    pthread_mutex_unlock(&storage_lock);
}

unsigned int get_threads_storage_size() {
    pthread_mutex_lock(&storage_lock);
    unsigned int result = thread_count;
    pthread_mutex_unlock(&storage_lock);
    return result;
}

// Start of storage traversing API

StorageCell *current_node_of_traversing = NULL;  // bad for mutlithread reading

void start_threads_storage_traverse() {
    current_node_of_traversing = created_threads_list;
}

pthread_t get_next_thread() {
    if (current_node_of_traversing == NULL) {
        return 0;
    }

    pthread_t result = current_node_of_traversing->thread;
    current_node_of_traversing = current_node_of_traversing->next;

    return result;
}

bool is_storage_empty() {
    return (current_node_of_traversing == NULL) ? true : false;
}
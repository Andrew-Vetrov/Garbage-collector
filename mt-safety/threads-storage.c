#include "threads-storage.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static StorageCell *created_threads_list = NULL;

pthread_mutex_t storage_lock =
    PTHREAD_MUTEX_INITIALIZER;  // lock for work with created_threads_list

StorageCell *create_cell_for_thread() {
    StorageCell *new_node = (StorageCell *)calloc(1, sizeof(StorageCell));

    if (new_node == NULL) {
        fprintf(stderr,
                "garbage-collector: mt-safety: store_thread: malloc() "
                "ended with an error\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&new_node->lock, NULL);

    pthread_mutex_lock(&new_node->lock);
    pthread_mutex_lock(&storage_lock);

    if (created_threads_list != NULL) {
        created_threads_list->prev = new_node;
    }

    new_node->next = created_threads_list;
    created_threads_list = new_node;
#ifdef DEBUG
    fprintf(stderr, "Thread %lld was stored\n", thread);
#endif
    pthread_mutex_unlock(&storage_lock);
    pthread_mutex_unlock(&new_node->lock);
    return new_node;
}

void destroy_cell(StorageCell *cell) {
    pthread_mutex_lock(&storage_lock);

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

__attribute__((destructor)) void destroy_storage() {
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

// Start of storage traversing API

StorageCell *current_node_of_traversing = NULL;

void start_threads_storage_traverse() {
    current_node_of_traversing = created_threads_list;
}

pthread_t get_next_thread() {
    StorageCell *result = current_node_of_traversing;
    if (current_node_of_traversing != NULL) {
        current_node_of_traversing = current_node_of_traversing->next;
    } else {
        return 0;
    }
    return result->thread;
}

bool is_traversing_ended() {
    return (current_node_of_traversing == NULL) ? true : false;
}
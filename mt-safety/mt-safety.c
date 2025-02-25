#ifdef DEBUG
#include <stdio.h>
#endif
#include <pthread.h>
#include <stdbool.h>
#define MAX_THREADS_COUNT 1000000  // Yes, it's hardcoded

typedef struct ThreadNode_t {
    pthread_t thread;
    struct ThreadNode_t *next, *prev;
} ThreadNode;

static ThreadNode threads[MAX_THREADS_COUNT] = {0};
static ThreadNode *free_nodes_list = NULL;
static ThreadNode *created_threads_list = NULL;

__attribute__((constructor)) void init_threads_storage() {
    free_nodes_list = &threads[0];
    for (int i = 0; i < MAX_THREADS_COUNT; i++) {
        threads[i].prev = (i != 0) ? &threads[i - 1] : NULL;
        threads[i].next = (i != MAX_THREADS_COUNT - 1) ? &threads[i + 1] : NULL;
    }
#ifdef DEBUG
    fprintf(stderr, "Threads storage was inititalized\n");
#endif
}

void on_create_thread(pthread_t thread) {
    if (free_nodes_list == NULL) {
        return;
    }

    ThreadNode *new_node = free_nodes_list;
    free_nodes_list = free_nodes_list->next;
    free_nodes_list->prev = NULL;

    new_node->thread = thread;
    new_node->prev = NULL;

    if (created_threads_list != NULL) {
        created_threads_list->prev = new_node;
        new_node->next = created_threads_list;
    } else {
        new_node->next = NULL;
    }
    
    created_threads_list = new_node;
#ifdef DEBUG
    fprintf(stderr, "Thread %lld was created\n", thread);
#endif
}

int __wrap_pthread_create(pthread_t *__restrict__ thread,
                          const pthread_attr_t *__restrict__ attr,
                          void *(*start_routine)(void *),
                          void *__restrict__ arg) {
#ifdef DEBUG
    fprintf(stderr, "pthread_create was wrapped\n");
#endif
    if (free_nodes_list == NULL) {
        // TODO: make an error
    }

    int result = pthread_create(thread, attr, start_routine, arg);

    if (result == 0) {
        on_create_thread(*thread);
    }

    return result;
}

ThreadNode* current_node_of_traversing = NULL;

void start_created_threads_traverse() {
    current_node_of_traversing = created_threads_list;
}

pthread_t get_next_thread() {
    ThreadNode* result = current_node_of_traversing;
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
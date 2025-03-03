#ifdef DEBUG
#include <stdio.h>
#endif
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "allocation-lock.h"
#include "threads-storage.h"

typedef struct {
    void *(*user_routine)(void *);
    void *arg;
    StorageCell *thread_node;
} WrapperArgs;

void *wrap_user_routine(void *arg) {
    WrapperArgs *args = (WrapperArgs *)arg;

    void *result = args->user_routine(args->arg);

    free(args);
    destroy_cell(args->thread_node);

    return result;
}

int __wrap_pthread_create(pthread_t *__restrict__ thread,
                          const pthread_attr_t *__restrict__ attr,
                          void *(*start_routine)(void *),
                          void *__restrict__ arg) {
#ifdef DEBUG
    fprintf(stderr, "pthread_create was wrapped\n");
#endif
    lock_allocation();
    WrapperArgs *args = (WrapperArgs *)calloc(1, sizeof(WrapperArgs));
    StorageCell *thread_node = create_cell_for_thread();

    args->arg = arg;
    args->user_routine = start_routine;
    args->thread_node = thread_node;

    int result = pthread_create(thread, attr, start_routine, arg);

    if (result == 0) {
        thread_node->thread = *thread;
    } else {
        free(args);
        destroy_cell(thread_node);
    }

    unlock_allocation();
    return result;
}
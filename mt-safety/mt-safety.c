#ifdef DEBUG
#include <stdio.h>
#endif
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "threads-storage.h"
#include "stop-the-world.h"

typedef struct {
    void *(*user_routine)(void *);
    void *arg;
    StorageCell *thread_node;
} WrapperArgs;

__attribute__((constructor))
void __add_main_thread_to_storage() {
    pthread_t thread_id = pthread_self();
    StorageCell* thread_node = create_cell_for_thread();
    thread_node->thread = thread;
}

void *wrap_user_routine(void *arg) {
    WrapperArgs *args = (WrapperArgs *)arg;

    prepare_thread_to_stop();

    void *result = args->user_routine(args->arg);

    destroy_cell(args->thread_node);
    free(args);

    return result;
}

pthread_rwlock_t thread_creation_lock = PTHREAD_RWLOCK_INITIALIZER;

int __wrap_pthread_create(pthread_t *__restrict__ thread,
                          const pthread_attr_t *__restrict__ attr,
                          void *(*start_routine)(void *),
                          void *__restrict__ arg) {
#ifdef DEBUG
    fprintf(stderr, "pthread_create was wrapped\n");
#endif
    pthread_rwlock_rdlock(&thread_creation_lock);
    WrapperArgs *args = (WrapperArgs *)calloc(1, sizeof(WrapperArgs));
    StorageCell *thread_node = create_cell_for_thread();

    args->arg = arg;
    args->user_routine = start_routine;
    args->thread_node = thread_node;

    int result = pthread_create(thread, attr, wrap_user_routine, args);

    if (result == 0) {
        thread_node->thread = *thread;
#ifdef DEBUG
        fprintf(stderr, "thread %lu was catched\n", *thread);
#endif
    } else {
        free(args);
        destroy_cell(thread_node);
    }

    pthread_rwlock_unlock(&thread_creation_lock);

    return result;
}
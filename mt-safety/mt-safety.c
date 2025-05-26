#ifdef DEBUG
#include <stdio.h>
#endif
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../allocator/allocator.h"
#include "../marker/marking.h"
#include "../sweeper/sweep.h"
#include "stop-the-world.h"
#include "threads-storage.h"

typedef struct {
    void *(*user_routine)(void *);
    void *arg;
    StorageCell *thread_node;
} WrapperArgs;

pthread_t service_thread;
static pthread_barrier_t signal_barrier;
static pthread_mutex_t service_thrd_call_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_spinlock_t signal_delivery_lock;
static bool is_gc_called = false;
static bool kill_service_thread = false;

void *service_thread_routine(void *);

__attribute__((constructor)) void __init_mt_safety() {
    pthread_spin_init(&signal_delivery_lock, 0);
    pthread_barrier_init(&signal_barrier, NULL, 2);
    pthread_create(&service_thread, NULL, service_thread_routine, NULL);
}

__attribute__((destructor)) void __destroy_mt_safety() {
    kill_service_thread = true;
    pthread_barrier_wait(&signal_barrier);
    pthread_barrier_destroy(&signal_barrier);
    pthread_mutex_destroy(&service_thrd_call_lock);
    pthread_spin_destroy(&signal_delivery_lock);
}

void call_service_thread() {
    pthread_mutex_lock(&service_thrd_call_lock);
    if (!is_gc_called) {
        is_gc_called = true;
        pthread_barrier_wait(&signal_barrier);
        pthread_spin_lock(&signal_delivery_lock);
        pthread_spin_unlock(&signal_delivery_lock);
    }
    pthread_mutex_unlock(&service_thrd_call_lock);
}

void *service_thread_routine(void *) {
    while (1) {
        pthread_spin_lock(&signal_delivery_lock);
        pthread_barrier_wait(&signal_barrier);
        if (kill_service_thread) {
            return NULL;
        }
        lock_allocation();
        pthread_spin_unlock(&signal_delivery_lock);
        stop_the_world();
        mark();
        sweep();
        start_the_world();
        pthread_mutex_lock(&service_thrd_call_lock);
        is_gc_called = false;
        pthread_mutex_unlock(&service_thrd_call_lock);
        unlock_allocation();
    }
    return NULL;
}

__attribute__((constructor)) void __add_main_thread_to_storage() {
    pthread_t thread_id = pthread_self();
    StorageCell *thread_node = create_cell_for_thread();
    thread_node->thread = thread_id;
}

void *wrap_user_routine(void *arg) {
    WrapperArgs *args = (WrapperArgs *)arg;

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

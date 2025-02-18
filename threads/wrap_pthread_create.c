#include <pthread.h>
#include <stdio.h>

typedef void* (thread_routine)(void*);

extern int __real_pthread_create(
        pthread_t *restrict thread, 
        const pthread_attr_t *restrict attr, 
        thread_routine start_routine,
        void *restrict arg);

static thread_routine* user_thread_routine = NULL;

void* wrap_start_routine(void* arg) {
    printf("Hello from wrap thread routine!\n");
    return user_thread_routine(arg);
}

int __wrap_pthread_create(
        pthread_t *restrict thread, 
        const pthread_attr_t *restrict attr, 
        thread_routine start_routine,
        void *restrict arg) {
    user_thread_routine = start_routine;
    printf("routine ptr: %p\n", user_thread_routine);
    return __real_pthread_create(thread, attr, wrap_start_routine, arg);
}

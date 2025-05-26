#include <pthread.h>
#define pthread_create __wrap_pthread_create


__attribute__((constructor)) void __init_mt_safety();

__attribute__((destructor)) void __destroy_mt_safety();

int __wrap_pthread_create(pthread_t *__restrict__ thread,
                          const pthread_attr_t *__restrict__ attr,
                          void *(*start_routine)(void *),
                          void *__restrict__ arg);

__attribute__((constructor)) void __init_stop_the_world();

void call_service_thread();

__attribute__((constructor)) void __add_main_thread_to_storage();

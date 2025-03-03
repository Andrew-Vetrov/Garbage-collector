#define _GNU_SOURCE
#include <assert.h>
#include <features.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define THREADS_COUNT 1000
#include "../mt-safety/mt-safety.h"
#include "../mt-safety/threads-storage.h"

void* thread_routine() { return NULL; }

pthread_t ths[THREADS_COUNT] = {0};

void on_abort() { exit(1); }

int main() {
    signal(SIGABRT, on_abort);
    bool is_thread_catched[THREADS_COUNT] = {false};
    for (int i = 0; i < THREADS_COUNT; i++) {
        assert(0 == pthread_create(&ths[i], NULL, thread_routine, NULL));
#ifdef DEBUG
        printf("thread %ld started\n", ths[i]);
#endif
    }

    start_created_threads_traverse();

    int threads_processed = 0;

    while (!is_traversing_ended()) {
        pthread_t thread = get_next_thread();

        for (int i = 0; i < THREADS_COUNT; i++) {
            if (ths[i] == thread) {
                is_thread_catched[i] = true;
                break;
            }
        }
#ifdef DEBUG
        printf("%4d \ %4d threads processed\n", ++threads_processed,
               THREADS_COUNT);
#endif
        assert(threads_processed <= THREADS_COUNT);
    }

    for (int i = 0; i < THREADS_COUNT; i++) {
        assert(is_thread_catched[i] == true);
    }

    return 0;
}

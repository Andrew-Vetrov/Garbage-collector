#define _GNU_SOURCE
#include <assert.h>
#include <features.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#define CREATORS_COUNT (10)
#define THREADS_COUNT (100)
#define ALL_THRDS_COUNT (CREATORS_COUNT * THREADS_COUNT + CREATORS_COUNT + 1)
#include "../mt-safety/mt-safety.h"
#include "../mt-safety/threads-storage.h"

pthread_barrier_t barrier1, barrier2;

void* routine() {
    pthread_barrier_wait(&barrier1);
    pthread_barrier_wait(&barrier2);
    return NULL;
}

pthread_t ths[THREADS_COUNT * CREATORS_COUNT];
bool is_thread_catched[THREADS_COUNT * CREATORS_COUNT];

void* thread_creator(void* arg) {
    int thread_num = (int)arg;
    for (int i = 0; i < THREADS_COUNT; i++) {
        is_thread_catched[thread_num * THREADS_COUNT + i] = false;
        assert(0 == pthread_create(&ths[thread_num * THREADS_COUNT + i], NULL,
                                   routine, NULL));
    }
    pthread_barrier_wait(&barrier1);
    pthread_barrier_wait(&barrier2);
    return NULL;
}

int main() {
    struct rlimit rl;
    getrlimit(RLIMIT_NPROC, &rl);
    assert(rl.rlim_cur >= CREATORS_COUNT * THREADS_COUNT);

    pthread_barrier_init(&barrier1, NULL, ALL_THRDS_COUNT);
    pthread_barrier_init(&barrier2, NULL, ALL_THRDS_COUNT);
    pthread_t threads[CREATORS_COUNT];
    for (int i = 0; i < CREATORS_COUNT; i++) {
        assert(0 ==
               pthread_create(&threads[i], NULL, thread_creator, (void*)i));
    }

    pthread_barrier_wait(&barrier1);

    start_threads_storage_traverse();

    int threads_proccessed = 0;
    bool is_main_catched = false;

    while (!is_storage_empty()) {
        pthread_t thread = get_next_thread();
        assert(++threads_proccessed <= ALL_THRDS_COUNT);
        if (pthread_equal(thread, pthread_self())) {
            assert(is_main_catched == false);
            is_main_catched = true;
            continue;
        }
        for (int i = 0; i < CREATORS_COUNT * THREADS_COUNT; i++) {
            if (pthread_equal(ths[i], thread)) {
                is_thread_catched[i] = true;
                break;
            }
        }
    }

    assert(is_main_catched);
    for (int i = 0; i < CREATORS_COUNT * THREADS_COUNT; i++) {
        assert(is_thread_catched[i] == true);
    }

    pthread_barrier_wait(&barrier2);

    for (int i = 0; i < CREATORS_COUNT * THREADS_COUNT; i++) {
        pthread_join(ths[i], NULL);
    }

    start_threads_storage_traverse();
    pthread_t main_thrd = get_next_thread();
    assert(pthread_equal(main_thrd, pthread_self()));
    assert(is_storage_empty() == true);

    pthread_barrier_destroy(&barrier1);
    pthread_barrier_destroy(&barrier2);
    return 0;
}

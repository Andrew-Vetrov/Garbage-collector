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
#include "../mt-safety/mt-safety.h"
#include "../mt-safety/threads-storage.h"

pthread_barrier_t barrier;

void* routine() {
    pthread_barrier_wait(&barrier);
    pthread_barrier_wait(&barrier);
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
    return NULL;
}

int main() {
    struct rlimit rl;
    getrlimit(RLIMIT_NPROC, &rl);
    assert(rl.rlim_cur >= CREATORS_COUNT * THREADS_COUNT);

    pthread_barrier_init(&barrier, NULL, CREATORS_COUNT * THREADS_COUNT + 1);
    pthread_t threads[CREATORS_COUNT];
    for (int i = 0; i < CREATORS_COUNT; i++) {
        assert(0 ==
               pthread_create(&threads[i], NULL, thread_creator, (void*)i));
    }

    pthread_barrier_wait(&barrier);

    pthread_mutex_lock(get_storage_lock());
    start_threads_storage_traverse();

    int threads_proccessed = 0;

    while (!is_traversing_ended()) {
        pthread_t thread = get_next_thread();
        assert(++threads_proccessed <=
               CREATORS_COUNT * THREADS_COUNT + CREATORS_COUNT);

        for (int i = 0; i < CREATORS_COUNT * THREADS_COUNT; i++) {
            if (pthread_equal(ths[i], thread)) {
                is_thread_catched[i] = true;
                break;
            }
        }
    }

    pthread_mutex_unlock(get_storage_lock());

    for (int i = 0; i < CREATORS_COUNT * THREADS_COUNT; i++) {
        assert(is_thread_catched[i] == true);
    }

    pthread_barrier_wait(&barrier);

    for (int i = 0; i < CREATORS_COUNT * THREADS_COUNT; i++) {
        pthread_join(ths[i], NULL);
    }

    pthread_mutex_lock(get_storage_lock());

    start_threads_storage_traverse();

    assert(is_traversing_ended() == true);

    pthread_mutex_unlock(get_storage_lock());
    pthread_barrier_destroy(&barrier);
    return 0;
}

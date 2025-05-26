#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>

#include "../allocator/small-allocator.h"
#include "../marker/marking.h"
#include "mt-safety-control.h"
#include "threads-storage.h"
#define SIG_TO_STOP SIGUSR1

static sigset_t all_sig_set;
static pthread_barrier_t barrier;
static unsigned int threads_stopped;
static bool is_world_started = false;
static pthread_mutex_t is_world_started_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t start_world_cond = PTHREAD_COND_INITIALIZER;

void handler(int sig) {
    if (sig != SIG_TO_STOP) {
        return;
    }
    clear_cache();
    push_registers_to_stack();
    pthread_barrier_wait(&barrier);
    pthread_mutex_lock(&is_world_started_lock);
    if (!is_world_started) {
        pthread_cond_wait(&start_world_cond, &is_world_started_lock);
    }
    pthread_mutex_unlock(&is_world_started_lock);
}

__attribute__((constructor)) void __init_stop_the_world() {
    struct sigaction sigact;
    sigact.sa_handler = handler;
    sigact.sa_flags = SA_RESTART;
    sigfillset(&sigact.sa_mask);
    sigaction(SIG_TO_STOP, &sigact, NULL);
}

__attribute__((destructor)) void __destroy_stop_the_world() {
    pthread_mutex_destroy(&is_world_started_lock);
    pthread_cond_destroy(&start_world_cond);
}

void stop_the_world() {
    pthread_rwlock_wrlock(
        &thread_creation_lock);  // it's very bad way to call lock() and
                                 // unlock() in different functions, we need to
                                 // change it
    threads_stopped = get_threads_storage_size();
    pthread_barrier_init(&barrier, NULL, threads_stopped + 1);
    pthread_mutex_lock(&is_world_started_lock);
    is_world_started = false;
    pthread_mutex_unlock(&is_world_started_lock);
    start_threads_storage_traverse();
    while (!is_storage_empty()) {
        pthread_kill(get_next_thread(), SIG_TO_STOP);
    }
    pthread_barrier_wait(&barrier);
    pthread_barrier_destroy(&barrier);
}

void start_the_world() {
    pthread_mutex_lock(&is_world_started_lock);
    is_world_started = true;
    pthread_mutex_unlock(&is_world_started_lock);
    pthread_cond_broadcast(&start_world_cond);
    pthread_rwlock_unlock(&thread_creation_lock);
}
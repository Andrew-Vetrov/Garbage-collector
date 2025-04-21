#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include "mt-safety-control.h"
#include "threads-storage.h"
#define SIG_TO_STOP SIGUSR1

static sigset_t all_sig_set;
static pthread_barrier_t barrier;
sem_t waiting_point;
static unsigned int thread_stopped;

void handler(int sig) {
    sigset_t old_sigset;
    pthread_sigmask(SIG_SETMASK, &all_sig_set, &old_sigset);
    if (sig != SIG_TO_STOP) {
        return;
    }
    pthread_barrier_wait(&barrier);
    sem_wait(&waiting_point);
}

void init_threads_stop() {
    sigfillset(&all_sig_set);
    sem_init(&waiting_point, 0, 0);
}

void prepare_thread_to_stop() { signal(SIG_TO_STOP, handler); }

void stop_the_world() {
    pthread_rwlock_wrlock(
        &thread_creation_lock);  // it's very bad way to call lock() and
                                 // unlock() in different functions, we need to
                                 // change it
    thread_stopped = get_threads_storage_size();
    pthread_barrier_init(&barrier, NULL, thread_stopped + 1);
    start_threads_storage_traverse();
    while (!is_traversing_ended()) {
        pthread_kill(get_next_thread(), SIG_TO_STOP);
    }
    pthread_barrier_wait(&barrier);
    pthread_barrier_destroy(&barrier);
}

void start_the_world() {
    for (unsigned int i = 0; i < thread_stopped; i++) {
        sem_post(&waiting_point);
    }
    pthread_rwlock_unlock(&thread_creation_lock);
}
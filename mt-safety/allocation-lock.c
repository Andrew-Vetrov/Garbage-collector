#include <pthread.h>

volatile static unsigned char allocation_lock_flag = 0;
pthread_cond_t allocation_ended = PTHREAD_COND_INITIALIZER;
pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;

void wait_allocation_unlock() {
    if (allocation_lock_flag) {
        pthread_cond_wait(&allocation_ended, &cond_mutex);
    }
}

void lock_allocation() { allocation_lock_flag = 1; }

void unlock_allocation() {
    allocation_lock_flag = 0;
    pthread_cond_signal(&allocation_ended);
}
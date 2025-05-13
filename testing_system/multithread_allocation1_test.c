#include "../gc.h"
#include "../mt-safety/mt-safety.h"
#include "../mt-safety/stop-the-world.h"
#include "../marker/marking.h"
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* thread_func(void* args) {
    pthread_mutex_lock(&counter_mutex);
    const int size = 40000;
    size_t a[size];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size / 100; j++) {
            a[j] = gc_malloc(16);
        }
    }
    printf("allocated array\n");
    pthread_mutex_unlock(&counter_mutex);
}

int main() {
    pthread_t t1, t2, t3;
    int thread1 = __wrap_pthread_create(&t1, NULL, thread_func, NULL);
    int thread2 = __wrap_pthread_create(&t2, NULL, thread_func, NULL);
    int thread3 = __wrap_pthread_create(&t3, NULL, thread_func, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    return 0;
}

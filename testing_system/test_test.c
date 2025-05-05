#include "../gc.h"
#include <stdio.h>
#include <unistd.h>

void* thread_func(void* args) {
	int* a = gc_malloc(16);
    printf("%llu\n", a);
}

int main() {
    pthread_t t1, t2, t3;
    int thread1 = __wrap_pthread_create(&t1, NULL, thread_func, NULL);

    pthread_join(t1, NULL);
    return 0;
}

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../mt-safety/mt-safety.h"
#include "../mt-safety/stop-the-world.h"

static int counter = 0;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* thread_func(void* args) {
	while(1) {
		pthread_mutex_lock(&counter_mutex);
		counter++;
		pthread_mutex_unlock(&counter_mutex);
	}
}

void test_stop_and_resume_world() {
	pthread_t t1, t2, t3;
	int prev;
	int thread1 = __wrap_pthread_create(&t1, NULL, thread_func, NULL);
	int thread2 = __wrap_pthread_create(&t2, NULL, thread_func, NULL);
	int thread3 = __wrap_pthread_create(&t3, NULL, thread_func, NULL);

	sleep(2);
	stop_the_world();

	prev = counter;
	sleep(1);

	if (prev != counter) {
		exit(1);
	}
	start_the_world();
	sleep(1);

	if (counter == prev) {
		exit(1);
	}
}

int main() {
	test_stop_and_resume_world();
	return 0;
}

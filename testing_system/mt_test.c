#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>

#include "../allocator/allocator.h"
#include "../allocator/large-allocator.h"
#include "../allocator/small-allocator.h"
#include "../gc.h"
#define THRDS_CNT (4)
#define ALLOC_CNT (BLOCKS_COUNT * 2 / THRDS_CNT)

pthread_barrier_t barrier;

void* routine(void* arg) {
    size_t objs[ALLOC_CNT] = {0};
    for (int i = 0; i < ALLOC_CNT / 2; i++) {
        assert((objs[i * 2] = gc_malloc(MAX_OBJECT_SIZE)) != NULL);
        *(size_t*)objs[i * 2] = 0;
        if (i < TREENODE_COUNT) {
            assert((objs[i * 2 + 1] = gc_malloc(MAX_OBJECT_SIZE + 1)) != NULL);
            *(size_t*)objs[i * 2 + 1] = 0;
        }
        if (i % 1000 == 0) {
            printf("iteration %d/%ld\n", i, ALLOC_CNT / 2);
        }
    }
    pthread_barrier_wait(&barrier);

    for (int i = ALLOC_CNT / 4; i < ALLOC_CNT / 2; i++) {
        objs[i * 2] = objs[i * 2 + 1] = 0;
    }
    // pthread_barrier_wait(&barrier);
    for (int i = ALLOC_CNT / 4; i < ALLOC_CNT / 2; i++) {
        assert((objs[i * 2] = gc_malloc(MAX_OBJECT_SIZE)) != NULL);

        if (i < TREENODE_COUNT) {
            assert((objs[i * 2 + 1] = gc_malloc(MAX_OBJECT_SIZE + 1)) != NULL);
        }
        if (i % 1000 == 0) {
            printf("iteration %d/%ld\n", i, ALLOC_CNT / 2);
        }
    }
    for (int i = 0; i < ALLOC_CNT / 2; i++) {
        *(size_t*)objs[i * 2] = *(size_t*)objs[i * 2 + 1] = 0;
    }
    return 0;
}

int main() {
    pthread_barrier_init(&barrier, NULL, THRDS_CNT);
    pthread_t thrds[THRDS_CNT];

    for (int i = 0; i < THRDS_CNT; i++) {
        pthread_create(&thrds[i], NULL, routine, NULL);
    }

    for (int i = 0; i < THRDS_CNT; i++) {
        pthread_join(thrds[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}
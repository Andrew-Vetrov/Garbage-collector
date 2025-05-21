#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#include "../allocator/allocator.h"
#include "../allocator/object.h"
#include "../gc.h"
#include "../marker/marking.h"
#include "../sweeper/sweep.h"

#define THRD_OBJS_CNT (10)
#define THRDS_CNT (4)
#define OBJS_CNT (THRD_OBJS_CNT * THRDS_CNT)

size_t thrd_objs_cnt = THRD_OBJS_CNT;
atomic_uint allocated_objs_cnt = 0;
size_t objs_addrs[OBJS_CNT] = {0};

void* thread_routine(void* arg) {
    size_t object_size = (size_t)arg;
    for (int i = 0; i < thrd_objs_cnt; i++) {
        size_t obj = objs_addrs[allocated_objs_cnt++] = gc_malloc(object_size);
        assert((void*)obj != NULL);
    }

    return 0;
}

void check_allocated_objs() {
    assert(allocated_objs_cnt == OBJS_CNT);

    Object objs[OBJS_CNT];

    for (unsigned int i = 0; i < OBJS_CNT; i++) {
        assert(0 == get_object(objs_addrs[i], &objs[i]) &&
               get_object_addr(objs[i]) == objs_addrs[i]);
    }

    for (unsigned int i = 0; i < OBJS_CNT; i++) {
        for (unsigned int j = 0; j < OBJS_CNT; j++) {
            if (i == j) {
                continue;
            }

            assert(objs_addrs[i] != objs_addrs[j]);
        }
    }
}

void allocate_objs(size_t object_size) {
    pthread_t thrds[THRDS_CNT];

    for (int i = 0; i < THRDS_CNT; i++) {
        assert(0 == pthread_create(&thrds[i], NULL, thread_routine,
                                   (void*)object_size));
    }

    for (int i = 0; i < THRDS_CNT; i++) {
        assert(0 == pthread_join(thrds[i], NULL));
    }
}

void small_allocator_test() {
    allocated_objs_cnt = 0;
    allocate_objs(MAX_OBJECT_SIZE);
    check_allocated_objs();
    for (unsigned int i = OBJS_CNT / 2; i < OBJS_CNT; i++) {
        objs_addrs[i] = 0;
    }
    allocated_objs_cnt = OBJS_CNT / 2;
    thrd_objs_cnt /= 2;
    mark();
    sweep();
    allocate_objs(MAX_OBJECT_SIZE);
    check_allocated_objs();
}

int main() {
    small_allocator_test();
    return 0;
}

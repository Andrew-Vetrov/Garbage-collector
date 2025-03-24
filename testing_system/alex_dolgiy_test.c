#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../allocator/allocator.h"
#include "../gc.h"
#include "../scanner/marking.h"

#define SAME_SIZE_OBJS_COUNT (10000)

void same_size_objects_marking_test(size_t objects_size) {
    assert(objects_size > 0 &&
           objects_size * SAME_SIZE_OBJS_COUNT < HEAP_SIZE * 2);

    size_t alive_objects_addrs[SAME_SIZE_OBJS_COUNT] = {0};

    for (unsigned int i = 0; i < SAME_SIZE_OBJS_COUNT; i++) {
        alive_objects_addrs[i] = gc_malloc(objects_size);
        assert(alive_objects_addrs[i] != NULL);
    }

    for (unsigned int i = 0; i < SAME_SIZE_OBJS_COUNT; i += 2) {
        alive_objects_addrs[i] = (size_t)NULL;
    }

    full_marking();

    for (unsigned int i = 1; i < SAME_SIZE_OBJS_COUNT; i += 2) {
        Object object;
        assert(get_object(alive_objects_addrs[i], &object) == 0);
        assert(get_object_addr(object) == alive_objects_addrs[i]);
        assert(is_marked(object));
    }
}

#define DIFF_SIZE_OBJS_COUNT (10000)
#define DIFF_SIZE_MAX_SIZE (10000)

void diff_size_objs_test() {
    size_t alive_objects_addrs[DIFF_SIZE_OBJS_COUNT] = {0};

    for (unsigned int i = 0; i < DIFF_SIZE_OBJS_COUNT; i++) {
        alive_objects_addrs[i] = gc_malloc(i % DIFF_SIZE_MAX_SIZE + 1);
        assert(alive_objects_addrs[i] != NULL);
    }

    for (unsigned int i = 0; i < DIFF_SIZE_OBJS_COUNT; i += 2) {
        alive_objects_addrs[i] = (size_t)NULL;
    }

    full_marking();

    for (unsigned int i = 1; i < DIFF_SIZE_OBJS_COUNT; i += 2) {
        Object object;
        assert(get_object(alive_objects_addrs[i], &object) == 0);
        assert(get_object_addr(object) == alive_objects_addrs[i]);
        assert(is_marked(object));
    }
}

void inner_pointer_test() {
    const int array_size = 100;
    int* array = (int*)gc_malloc(array_size * sizeof(int));

    assert(array != NULL);

    array += array_size - 1;
    full_marking();
    array -= array_size - 1;
    Object object;
    assert(get_object((size_t)array, &object) == 0);
    assert(is_marked(object));
}

int main() {
    same_size_objects_marking_test(80);
    diff_size_objs_test();
    inner_pointer_test();
}
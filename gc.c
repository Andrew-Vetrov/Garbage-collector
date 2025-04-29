#include "allocator/allocator.h"
#include "allocator/object.h"
#include "allocator/utils.h"
#include "logging/log.h"
#include "marker/marking.h"
#include "sweeper/sweep.h"
#include <string.h>

size_t gc_malloc(size_t size) {
    if (size < 1 || size > HEAP_SIZE) {
        return (size_t)NULL;
    }

    size_t res = (size_t)NULL;
    res = allocate_new_object(size);
    if (res == NULL) {
        mark();
        sweep();
        res = allocate_new_object(size);
    }
    return res;
}

size_t gc_calloc(size_t size) {
    size_t res = gc_malloc(size);
    if (res != NULL) {
        memset((void*) res, 0, size);
    }

    return res;
}

size_t gc_realloc(void* memblock, size_t size) {
    size_t res = gc_malloc(size);
    if (res != NULL) {
        memcpy((void*) res, memblock, get_object_size((Object) memblock));
        //gc_free(memblock); // to do
    }

    return res;
}
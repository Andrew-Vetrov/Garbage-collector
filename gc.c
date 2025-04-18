#include "allocator/allocator.h"
#include "allocator/utils.h"
#include "logging/log.h"
#include "marker/marking.h"
#include "sweeper/sweep.h"

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
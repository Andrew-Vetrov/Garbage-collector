#include "allocator/allocator.h"
#include "allocator/object.h"
#include "allocator/utils.h"
#include "logging/log.h"
#include "memops/gc_mem.h"
#include "mt-safety/mt-safety.h"
#include "sweeper/sweep.h"

size_t gc_malloc(size_t size) {
    if (size < 1 || size > HEAP_SIZE) {
        return (size_t)NULL;
    }
    size_t res = (size_t)NULL;
    res = allocate_new_object(size);
    if (res == NULL) {
        call_service_thread();
        res = allocate_new_object(size);
    }
    return res;
}

size_t gc_calloc(size_t size) {
    size_t res = gc_malloc(size);
    if (res != NULL) {
        gc_memset((void*)res, 0, size);
    }

    return res;
}

size_t gc_realloc(void* memblock, size_t size) {
    if (memblock == NULL) {
        return gc_malloc(size);
    }

    Object obj;
    if (get_object((size_t)memblock, &obj) != 0) {
        return NULL;
    }
    size_t res = gc_malloc(size);
    if (res != NULL) {
        gc_memcpy((void*)res, obj, get_object_size(obj));
    } else {
    }

    return res;
}

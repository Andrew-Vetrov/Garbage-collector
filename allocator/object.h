#include <stdbool.h>
#include <stdlib.h>

#define get_object_addr(object) ((size_t)object)
#define INVALID_ADDRESS (-1)

typedef size_t Object;

int get_object(size_t object_addr, Object* object);

size_t get_object_size(Object object);

void mark_object(Object object);

bool is_marked(Object object);
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../allocator/allocator.h"
#include "../scanner/marking.h"

typedef struct Node_t {
    struct Node_t **left_array, **right_array;
} Node;

void multiheap_relation_test() {
    Node** root_array =
        (Node**)gc_malloc(sizeof(Node*) * (MAX_OBJECT_SIZE + 1));

    for (size_t i = 0; i < MAX_OBJECT_SIZE + 1; i++) {
        root_array[i] = (Node*)gc_malloc(sizeof(Node));
        assert(root_array[i] != NULL);
        root_array[i]->left_array =
            (Node**)gc_malloc(sizeof(Node*) * (MAX_OBJECT_SIZE + 1));
        root_array[i]->right_array =
            (Node**)gc_malloc(sizeof(Node*) * (MAX_OBJECT_SIZE + 1));

        for (size_t j = 0; j < MAX_OBJECT_SIZE + 1; j++) {
            root_array[i]->left_array[j] = (Node*)gc_malloc(sizeof(Node));
            root_array[i]->left_array[j]->left_array =
                root_array[i]->left_array[j]->right_array = 0;
        }
        for (size_t j = 0; j < MAX_OBJECT_SIZE + 1; j++) {
            root_array[i]->right_array[j] = (Node*)gc_malloc(sizeof(Node));
            root_array[i]->right_array[j]->left_array =
                root_array[i]->right_array[j]->right_array = 0;
        }
    }

    full_marking();

    Object object;
    assert(get_object((size_t)root_array, &object) == 0);
    assert(is_marked(object));

    for (size_t i = 0; i < MAX_OBJECT_SIZE + 1; i++) {
        assert(get_object((size_t)root_array[i], &object) == 0);
        assert(is_marked(object));
        for (size_t j = 0; j < MAX_OBJECT_SIZE + 1; j++) {
            assert(get_object((size_t)root_array[i]->left_array[j], &object) ==
                   0);
            assert(is_marked(object));
            assert(get_object((size_t)root_array[i]->right_array[j], &object) ==
                   0);
            assert(is_marked(object));

            if (j % 2 == 0) {
                root_array[i]->left_array[j] = 0;
            } else {
                root_array[i]->right_array[j] = 0;
            }
        }

        if (i % 2 == 0) {
            root_array[i] = 0;
        }
    }

    sweep();
    full_marking();

    for (size_t i = 1; i < MAX_OBJECT_SIZE + 1; i += 2) {
        assert(get_object((size_t)root_array[i], &object) == 0);
        assert(is_marked(object));
        for (size_t j = 0; j < MAX_OBJECT_SIZE + 1; j++) {
            if (j % 2 != 0) {
                assert(get_object((size_t)root_array[i]->left_array[j],
                                  &object) == 0);
                assert(is_marked(object));
            } else {
                assert(get_object((size_t)root_array[i]->right_array[j],
                                  &object) == 0);
                assert(is_marked(object));
            }
        }
    }
}

int main() { multiheap_relation_test(); }
#include <stdio.h>
#include <stdlib.h>
#include "../allocator/allocator.h"

typedef struct Node {
    struct Node* left;
    struct Node* right;
} Node;

Node* create_node() {
    Node* node = (Node*)allocate_new_object(sizeof(Node));
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node* build_tree(int depth) {
    if (depth == 0) {
        return NULL;
    }

    Node* root = create_node();
    root->left = build_tree(depth - 1);
    root->right = build_tree(depth - 1);
    return root;
}

int main() {
    int size1 = 10000000;
    void* test_arr[size1];
    for (int i = 0; i < size1; i++) {
        test_arr[i] = allocate_new_object(16);
    }
    return 0;
    const int depth = 40;
    const int size = 10;
    Node* array[size];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            array[j] = build_tree(depth);
        }
    }
    printf("GC amount = %d\n", counter);
    return 0;
}

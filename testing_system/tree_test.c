#include <stdio.h>
#include <stdlib.h>
#include "../allocator/allocator.h"

typedef struct Node {
    struct Node* left;
    struct Node* right;
} Node;

Node* create_node() {
    Node* node = (Node*)gc_malloc(sizeof(Node));
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
    const int depth = 20;
    const int size = 10;
    Node* array[size];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            array[j] = build_tree(depth);
        }
    }
    return 0;
}

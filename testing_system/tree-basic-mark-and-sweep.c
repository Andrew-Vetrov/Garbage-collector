#include <stdio.h>
#include <assert.h>
#include "../gc.h"
#include "../allocator/large-allocator.h"
#include "../marker/marking.h"
#include "../sweeper/sweep.h"

#define NUMBER_OF_POINTERS 100

void print_node(TreeNode *node) {
    printf("l - %p\n", node->left);
    printf("r - %p\n", node->right);
    printf("addr - %p\n", (void *)node->block.addr);
    printf("size - %lu\n", node->block.size);
    printf("marked - %u\n\n", node->block.isMarked);
}

void print_tree(TreeNode *node) {
    if (node) {
        print_tree(node->left);
        print_node(node);
        print_tree(node->right);
    }
}

int main() {

    TreeNode *node1;
    TreeNode *node2;

    int *pointers[NUMBER_OF_POINTERS] = {0};

    for (int i = 0; i < NUMBER_OF_POINTERS; i++) {
        pointers[i] = (int *)gc_malloc(4096);
        pointers[i]++;
    }
    
    mark();

    for (TreeNode* node = get_occupied_root(); node != NULL; node = node->right) {
        assert(node->block.isMarked == true);
    }

    sweep();

    for (TreeNode* node = get_occupied_root(); node != NULL; node = node->right) {
        assert(node->block.isMarked == false);
    }

    sweep();

    node1 = get_occupied_root();
    node2 = get_free_root();

    assert(node1 == NULL);
    assert(node2->left == NULL && node2->right == NULL);
    assert(node2->block.addr = get_large_heap_start());
    assert(node2->block.size == HEAP_SIZE);

    return 0;
}

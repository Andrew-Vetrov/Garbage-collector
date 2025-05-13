#include <stdio.h>
#include <assert.h>
#include "../gc.h"
#include "../allocator/large-allocator.h"
#include "../sweeper/sweep.h"

void print_node(TreeNode *node) {
    printf("l - %p\n", node->left);
    printf("r - %p\n", node->right);
    printf("addr - %p\n", (void *)node->block.addr);
    printf("size - %lu\n", node->block.size);
    printf("marked - %u\n\n", node->block.isMarked);
}

void print_tree(TreeNode *node) {
    if (node != NULL) {
        print_tree(node->left);
        print_node(node);
        print_tree(node->right);
    }
}

int main() {

    gc_malloc(4096);
    gc_malloc(4096);
    gc_malloc(4096);
    
    size_t i = 0;
    TreeNode *node;

    for(node = get_occupied_root(); node != NULL; node = node->right, i++) {
        assert(node->block.addr == get_large_heap_start() + (0x1000 * i));
    }

    assert(get_free_root()->block.addr == get_large_heap_start() + (0x1000 * i));
    i = 0;

    get_occupied_root()->block.isMarked = true;
    get_occupied_root()->right->block.isMarked = false;
    get_occupied_root()->right->right->block.isMarked = true;

    sweep();

    assert(get_occupied_root()->block.addr == get_large_heap_start());
    assert(get_occupied_root()->right->block.addr == get_large_heap_start() + 0x2000);
    assert(get_free_root()->block.addr == get_large_heap_start() + 0x3000);
    assert(get_free_root()->left->block.addr == get_large_heap_start() + 0x1000);

    get_occupied_root()->block.isMarked = true;
    get_occupied_root()->right->block.isMarked = false;

    sweep();

    assert(get_occupied_root()->left == NULL
        && get_occupied_root()->right == NULL
        && get_occupied_root()->block.addr == get_large_heap_start()
        && get_occupied_root()->block.size == 0x1000);

    assert(get_free_root() != NULL);

    assert(get_free_root()->left == NULL
        && get_free_root()->right == NULL
        && get_free_root()->block.addr == get_large_heap_start() + 0x1000
        && get_free_root()->block.size == HEAP_SIZE - 0x1000);

    return 0;
}
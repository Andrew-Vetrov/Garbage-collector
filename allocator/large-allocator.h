#include <stdbool.h>
#include <stdlib.h>

#include "allocator.h"

#define TREENODE_COUNT (HEAP_SIZE / GET_SIZE_WITH_ALIGNMENT(MAX_OBJECT_SIZE + 1))

typedef struct Header {
    size_t addr;
    size_t size;
    bool isMarked;
} Header;

typedef struct TreeNode {
    Header block;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

void __init_large_allocator();

void __destroy_large_allocator();

size_t allocate_large_object(size_t object_size);

/* getters and setters */

/* get start address of large allocator's heap */
size_t get_large_heap_start();

/* get end address of large allocator's heap */
size_t get_large_heap_end();

TreeNode* get_free_root();

void set_free_root(TreeNode* new_free_root);

TreeNode* get_occupied_root();

void set_occupied_root(TreeNode* new_occupied_root);

TreeNode* search_bst(TreeNode* root, size_t addr);

TreeNode* ext_search_bst(TreeNode* root, size_t addr);

void cleanup_bst(TreeNode** root);

size_t get_lhs_address();

size_t get_lhe_address();

size_t get_fr_root_address();

size_t get_oc_root_address();

size_t get_tnd_address();

size_t get_trnlh_address();

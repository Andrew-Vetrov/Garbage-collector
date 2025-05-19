#include "large-allocator.h"

TreeNode* insert_bst(TreeNode* root, TreeNode* node);

TreeNode* search_bst(TreeNode* root, size_t addr);

TreeNode* ext_search_bst(TreeNode* root, size_t addr);

TreeNode* remove_bst(TreeNode* root, size_t addr, TreeNode** removed);

void cleanup_bst(TreeNode** root);

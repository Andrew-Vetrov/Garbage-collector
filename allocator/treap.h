#include "large-allocator.h"

/*

void best_fit_search(TreeNode* root, size_t size, TreeNode** best_fit_node);

*/

void first_fit_search(TreeNode* root, size_t size, TreeNode** first_fit_node);

void split_treap(TreeNode* t, size_t addr, TreeNode** l, TreeNode** r);

TreeNode* merge_treap(TreeNode* l, TreeNode* r);

TreeNode* find_rightmost(TreeNode* t);

TreeNode* find_leftmost(TreeNode* t);

TreeNode* insert_treap(TreeNode* root, TreeNode* new_node);

TreeNode* remove_treap(TreeNode* t, size_t addr, TreeNode** removed);

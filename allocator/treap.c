#include "large-allocator.h"
#include "binary-search-tree.h"

/*
Implementation of the primitive best fit allocation.

void best_fit_search(TreeNode* root, size_t size, TreeNode** best_fit_node) {

    if (*best_fit_node && *best_size == size) {
        return;
    }

    if (root == NULL) {
        return;
    }

    if (root->block.size == size) {
        *best_fit_node = root;
        *best_size = size;
        return;
    }

    if (root->block.size > size) {
        if (root->block.size < *best_size) {
            *best_fit_node = root;
            *best_size = root->block.size;    
        }
        best_fit_search(root->left, size, best_fit_node, best_size);
        best_fit_search(root->right, size, best_fit_node, best_size);
    }
}
*/

void first_fit_search(TreeNode* root, size_t size, TreeNode** first_fit_node) {
    if (root && root->block.size >= size) {
        *first_fit_node = root;
    } else {
        *first_fit_node = NULL;
    }
}

void split_treap(TreeNode* t, size_t addr, TreeNode** l, TreeNode** r) {
    if (t == NULL) {
        *l = *r = NULL;
    } else if (addr < t->block.addr) {
        split_treap(t->left, addr, l, &(t->left));
        *r = t;
    } else {
        split_treap(t->right, addr, &(t->right), r);
        *l = t;
    }
}

TreeNode* merge_treap(TreeNode* l, TreeNode* r) {
    if (l == NULL || r == NULL) return l ? l : r;

    if (l->block.size > r->block.size) {
        l->right = merge_treap(l->right, r);
        return l;
    } else {
        r->left = merge_treap(l, r->left);
        return r;
    }
}

TreeNode* find_rightmost(TreeNode* t) {
    if (t == NULL) return NULL;
    while (t->right) t = t->right;
    return t;
}

TreeNode* find_leftmost(TreeNode* t) {
    if (t == NULL) return NULL;
    while (t->left) t = t->left;
    return t;
}

TreeNode* insert_treap(TreeNode* root, TreeNode* new_node) {
    if (new_node == NULL) return root;

    TreeNode *l = NULL, *r = NULL;
    split_treap(root, new_node->block.addr, &l, &r);

    TreeNode* left_neighbor = find_rightmost(l);
    if (left_neighbor && (left_neighbor->block.addr + left_neighbor->block.size == new_node->block.addr)) {
        new_node->block.addr = left_neighbor->block.addr;
        new_node->block.size += left_neighbor->block.size;
        TreeNode* tmp_removed = NULL;
        l = remove_bst(l, left_neighbor->block.addr, &tmp_removed);
        if (tmp_removed)
            free_tree_node(tmp_removed);
    }

    TreeNode* right_neighbor = find_leftmost(r);
    if (new_node && right_neighbor && (new_node->block.addr + new_node->block.size == right_neighbor->block.addr)) {
        new_node->block.size += right_neighbor->block.size;
        TreeNode* tmp_removed = NULL;
        r = remove_bst(r, right_neighbor->block.addr, &tmp_removed);
        if (tmp_removed)
            free_tree_node(tmp_removed);
    }

    return merge_treap(merge_treap(l, new_node), r);
}

TreeNode* remove_treap(TreeNode* t, size_t addr, TreeNode** removed) {
    if (t == NULL) {
        *removed = NULL;
        return NULL;
    }

    if (t->block.addr == addr) {
        *removed = t;
        TreeNode* tmp = merge_treap(t->left, t->right);
        return tmp;
    } else if (addr < t->block.addr) {
        t->left = remove_treap(t->left, addr, removed);
    } else {
        t->right = remove_treap(t->right, addr, removed);
    }

    return t;
}

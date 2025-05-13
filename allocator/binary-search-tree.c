#include "large-allocator.h"
#include "treap.h"

TreeNode* insert_bst(TreeNode* root, TreeNode* node) {
    if (root == NULL) return node;

    if (node->block.addr < root->block.addr) {
        root->left = insert_bst(root->left, node);
    } else {
        root->right = insert_bst(root->right, node);
    }
    return root;
}

TreeNode* search_bst(TreeNode* root, size_t addr) {
    if (root == NULL) return NULL;

    if (addr == root->block.addr)
        return root;
    else if (addr < root->block.addr)
        return search_bst(root->left, addr);
    else
        return search_bst(root->right, addr);
}

TreeNode* ext_search_bst(TreeNode* root, size_t addr) {
    if (root == NULL) return NULL;

    if (root->block.addr <= addr && addr < root->block.addr + root->block.size)
        return root;
    else if (addr < root->block.addr)
        return ext_search_bst(root->left, addr);
    else
        return ext_search_bst(root->right, addr);
}

TreeNode* remove_bst(TreeNode* root, size_t addr, TreeNode** removed) {
    if (root == NULL) {
        *removed = NULL;
        return NULL;
    }

    if (addr < root->block.addr) {
        root->left = remove_bst(root->left, addr, removed);
    } else if (addr > root->block.addr) {
        root->right = remove_bst(root->right, addr, removed);
    } else {
        *removed = root;

        if (root->left == NULL) {
            TreeNode* new_root = root->right;
            root->left = root->right = NULL;
            return new_root;
        } else if (root->right == NULL) {
            TreeNode* new_root = root->left;
            root->left = root->right = NULL;
            return new_root;
        } else {
    
            TreeNode* min_node_parent = root;
            TreeNode* min_node = root->right;
            while (min_node->left) {
                min_node_parent = min_node;
                min_node = min_node->left;
            }

            root->block = min_node->block;

            if (min_node_parent->left == min_node) {
                min_node_parent->left = remove_bst(min_node, min_node->block.addr, removed);
            } else {
                min_node_parent->right = remove_bst(min_node, min_node->block.addr, removed);
            }

            return root;
        }
    }

    return root;
}

void cleanup_bst(TreeNode** root) {
    if ((*root) == NULL) return;

    TreeNode* insert_treap(TreeNode* root, TreeNode* new_node);

    cleanup_bst(&(*root)->left);
    cleanup_bst(&(*root)->right);

    if ((*root)->block.isMarked) {
        (*root)->block.isMarked = false;
    } else {
        TreeNode* removed = NULL;
        *root = remove_bst(*root, (*root)->block.addr, &removed);

        TreeNode **free_root_addr = (TreeNode **)get_fr_root_address();
        *free_root_addr = insert_treap(*free_root_addr, removed);
    }
}

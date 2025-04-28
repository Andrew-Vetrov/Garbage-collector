#include "large-allocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../logging/log.h"
#include "allocator.h"

static TreeNode TREENODES_LIST[TREENODE_COUNT];
static TreeNode* TREENODE_LIST_HEAD = 0;

TreeNode* free_root = NULL;
TreeNode* occupied_root = NULL;

size_t START_BIG_ALLOCATOR_HEAP = 0;
size_t END_BIG_ALLOCATOR_HEAP = 0;

TreeNode* get_new_tree_node() {
    if (TREENODE_COUNT == NULL) {
        return NULL;
    } else {
        TreeNode* result = TREENODE_LIST_HEAD;
        TREENODE_LIST_HEAD = TREENODE_LIST_HEAD->right;
        result->left = result->right = NULL;
        return result;
    }
}

void free_tree_node(TreeNode* node) {
    if (node) {
        node->block.addr = 0;
        node->block.size = 0;
        node->block.isMarked = false;
        node->left = NULL;
        node->right = TREENODE_LIST_HEAD;
        TREENODE_LIST_HEAD = node;
    }
}

void __init_large_allocator() {
    START_BIG_ALLOCATOR_HEAP =
        (size_t)mmap(NULL, HEAP_SIZE, PROT_WRITE | PROT_READ,
                     MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if (START_BIG_ALLOCATOR_HEAP == MAP_FAILED) {
        log(INIT_ALLOCATOR, B_ERROR);
        return;
    }

    for (int i = 0; i < TREENODE_COUNT; i++) {
        TREENODES_LIST[i].block.addr = TREENODES_LIST[i].block.size = 0;
        TREENODES_LIST[i].block.isMarked = false;
        if (i != TREENODE_COUNT - 1) {
            TREENODES_LIST[i].right = &TREENODES_LIST[i + 1];
        } else {
            TREENODES_LIST[i].right = 0;
        }
    }

    TREENODE_LIST_HEAD = &TREENODES_LIST[0];

    END_BIG_ALLOCATOR_HEAP = START_BIG_ALLOCATOR_HEAP + HEAP_SIZE;

    free_root = get_new_tree_node();

    free_root->left = free_root->right = NULL;
    free_root->block.isMarked = false;
    free_root->block.size = HEAP_SIZE;
    free_root->block.addr = START_BIG_ALLOCATOR_HEAP;
}

void __destroy_large_allocator() {
    if (munmap((void*)START_BIG_ALLOCATOR_HEAP, HEAP_SIZE) == -1) {
        log(DESTROY_ALLOCATOR, B_ERROR);
    }
}

TreeNode* insert_bst(TreeNode* root, TreeNode* node) {
    if (!root) return node;

    if (node->block.addr < root->block.addr) {
        root->left = insert_bst(root->left, node);
    } else {
        root->right = insert_bst(root->right, node);
    }
    return root;
}

TreeNode* search_bst(TreeNode* root, size_t addr) {
    if (!root) return NULL;

    if (addr == root->block.addr)
        return root;
    else if (addr < root->block.addr)
        return search_bst(root->left, addr);
    else
        return search_bst(root->right, addr);
}

TreeNode* ext_search_bst(TreeNode* root, size_t addr) {
    if (!root) return NULL;

    if (root->block.addr <= addr && addr < root->block.addr + HEAP_SIZE)
        return root;
    else if (addr < root->block.addr)
        return search_bst(root->left, addr);
    else
        return search_bst(root->right, addr);
}

TreeNode* remove_bst(TreeNode* root, size_t addr, TreeNode** removed) {
    if (!root) {
        *removed = NULL;
        return NULL;
    }

    if (addr < root->block.addr) {
        root->left = remove_bst(root->left, addr, removed);
    } else if (addr > root->block.addr) {
        root->right = remove_bst(root->right, addr, removed);
    } else {
        *removed = root;

        if (!root->left) {
            TreeNode* new_root = root->right;
            root->left = root->right = NULL;
            return new_root;
        } else if (!root->right) {
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
    if (!(*root)) return;

    cleanup_bst(&(*root)->left);
    cleanup_bst(&(*root)->right);

    if ((*root)->block.isMarked) {
        (*root)->block.isMarked = false;
    } else {
        TreeNode* removed = NULL;
        *root = remove_bst(*root, (*root)->block.addr, &removed);
        free_root = insert_treap(free_root, removed);
    }
}

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
    if (!t) {
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
    if (!l || !r) return l ? l : r;

    if (l->block.size > r->block.size) {
        l->right = merge_treap(l->right, r);
        return l;
    } else {
        r->left = merge_treap(l, r->left);
        return r;
    }
}

TreeNode* find_rightmost(TreeNode* t) {
    if (!t) return NULL;
    while (t->right) t = t->right;
    return t;
}

TreeNode* find_leftmost(TreeNode* t) {
    if (!t) return NULL;
    while (t->left) t = t->left;
    return t;
}

TreeNode* insert_treap(TreeNode* root, TreeNode* new_node) {
    if (!new_node) return root;

    TreeNode *l = NULL, *r = NULL;
    split_treap(root, new_node->block.addr, &l, &r);

    TreeNode* left_neighbor = find_rightmost(l);
    if (left_neighbor && (left_neighbor->block.addr + left_neighbor->block.size == new_node->block.addr)) {
        left_neighbor->block.size += new_node->block.size;
        free_tree_node(new_node);
        new_node = left_neighbor;
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
    if (!t) {
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

size_t allocate_large_object(size_t object_size) {
    TreeNode* first_fit_node;
    size_t res = 0;
    object_size = GET_SIZE_WITH_ALIGNMENT(object_size);

    first_fit_search(free_root, object_size, &first_fit_node);

    if (first_fit_node == NULL) {
        return NULL;
    }

    res = first_fit_node->block.addr;
    free_root = remove_treap(free_root, first_fit_node->block.addr, &first_fit_node);

    if (first_fit_node->block.size == object_size) {
        occupied_root = insert_bst(occupied_root, first_fit_node);
    } else {

        first_fit_node->block.size = object_size;

        TreeNode* new_node = get_new_tree_node();
        new_node->block.addr = first_fit_node->block.addr + object_size;
        new_node->block.size = first_fit_node->block.size - object_size;

        occupied_root = insert_bst(occupied_root, first_fit_node);
        free_root = insert_treap(free_root, new_node);
    }

    return res;
}

size_t get_large_heap_start() { return START_BIG_ALLOCATOR_HEAP; }

size_t get_large_heap_end() { return END_BIG_ALLOCATOR_HEAP; }

TreeNode* get_free_root() { return free_root; }

void set_free_root(TreeNode* new_free_root) { free_root = new_free_root; }

TreeNode* get_occupied_root() { return occupied_root; }

void set_occupied_root(TreeNode* new_occupied_root) { occupied_root = new_occupied_root; }

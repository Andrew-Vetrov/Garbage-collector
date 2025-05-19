#include "large-allocator.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../logging/log.h"
#include "allocator.h"
#include "binary-search-tree.h"
#include "treap.h"

static TreeNode* TREENODES_LIST = NULL;
static TreeNode* TREENODE_LIST_HEAD = NULL;

TreeNode* free_root = NULL;
TreeNode* occupied_root = NULL;

size_t* START_BIG_ALLOCATOR_HEAP = NULL;
size_t* END_BIG_ALLOCATOR_HEAP = NULL;

pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;

TreeNode* get_new_tree_node() {
    if (TREENODE_LIST_HEAD == NULL) {
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
    if ((TREENODES_LIST =
             (TreeNode*)malloc(sizeof(TreeNode) * TREENODE_COUNT)) == NULL) {
        log(INIT_ALLOCATOR, B_ERROR);
        return;
    }

    if ((START_BIG_ALLOCATOR_HEAP = (size_t*)malloc(sizeof(size_t))) == NULL) {
        log(INIT_ALLOCATOR, B_ERROR);
        return;
    }

    if ((END_BIG_ALLOCATOR_HEAP = (size_t*)malloc(sizeof(size_t))) == NULL) {
        log(INIT_ALLOCATOR, B_ERROR);
        return;
    }

    *START_BIG_ALLOCATOR_HEAP =
        (size_t)mmap(NULL, HEAP_SIZE, PROT_WRITE | PROT_READ,
                     MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if ((*START_BIG_ALLOCATOR_HEAP) == MAP_FAILED) {
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

    *END_BIG_ALLOCATOR_HEAP = (*START_BIG_ALLOCATOR_HEAP) + HEAP_SIZE;

    free_root = get_new_tree_node();

    free_root->left = free_root->right = NULL;
    free_root->block.isMarked = false;
    free_root->block.size = HEAP_SIZE;
    free_root->block.addr = (*START_BIG_ALLOCATOR_HEAP);
}

void __destroy_large_allocator() {
    if (munmap((void*)(*START_BIG_ALLOCATOR_HEAP), HEAP_SIZE) == -1) {
        log(DESTROY_ALLOCATOR, B_ERROR);
    }

    free(TREENODES_LIST);
    free(START_BIG_ALLOCATOR_HEAP);
    free(END_BIG_ALLOCATOR_HEAP);

    pthread_mutex_destroy(&global_lock);
}

size_t allocate_large_object(size_t object_size) {
    pthread_mutex_lock(&global_lock);
    TreeNode* first_fit_node;
    size_t res = 0;

    object_size = GET_SIZE_WITH_ALIGNMENT(object_size);
    log_t cts_result = check_the_space(object_size);

    first_fit_search(free_root, object_size, &first_fit_node);

    if (first_fit_node == NULL) {
        pthread_mutex_unlock(&global_lock);
        return (size_t)NULL;
    }

    res = first_fit_node->block.addr;
    free_root =
        remove_treap(free_root, first_fit_node->block.addr, &first_fit_node);

    if (first_fit_node->block.size == object_size) {
        occupied_root = insert_bst(occupied_root, first_fit_node);
    } else {
        TreeNode* new_node;

        if ((new_node = get_new_tree_node()) == NULL) {
            free_root = insert_treap(free_root, first_fit_node);
            log(OTHER, O_HEADER);
            pthread_mutex_unlock(&global_lock);
            return (size_t)NULL;
        }

        new_node->block.addr = first_fit_node->block.addr + object_size;
        new_node->block.size = first_fit_node->block.size - object_size;

        first_fit_node->block.size = object_size;

        occupied_root = insert_bst(occupied_root, first_fit_node);
        free_root = insert_treap(free_root, new_node);
    }

    log(ALLOCATE_NEW_OBJECT, OK);
    pthread_mutex_unlock(&global_lock);
    return res;
}

size_t get_large_heap_start() { return *START_BIG_ALLOCATOR_HEAP; }

size_t get_large_heap_end() { return *END_BIG_ALLOCATOR_HEAP; }

TreeNode* get_free_root() { return free_root; }

void set_free_root(TreeNode* new_free_root) { free_root = new_free_root; }

TreeNode* get_occupied_root() { return occupied_root; }

void set_occupied_root(TreeNode* new_occupied_root) {
    occupied_root = new_occupied_root;
}

size_t get_oc_root_address() { return (size_t)&occupied_root; }

size_t get_fr_root_address() { return (size_t)&free_root; }

/*
    debug shit

    size_t get_lhs_address() { return &START_BIG_ALLOCATOR_HEAP; }
    size_t get_lhe_address() { return &END_BIG_ALLOCATOR_HEAP; }
    size_t get_tnd_address() { return &TREENODES_LIST; }
    size_t get_trnlh_address() { return &TREENODE_LIST_HEAD; }
*/

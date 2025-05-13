#include "../allocator/allocator.h"
#include "../allocator/bitmap.h"
#include "../allocator/large-allocator.h"
#include "../allocator/small-allocator.h"
#include "../allocator/binary-search-tree.h"
#include "../logging/log.h"

void sweep_small_heap() {
    #ifdef DEBUG
        int empty_nodes_count = 0;
        int segreg_list_nodes_count = 0;
    #endif
        set_empty_list_head(NULL);
        for (int i = 0; i < OBJECT_SIZE_UPPER_BOUND; i++) {
            set_segreg_list_head(i, NULL);
        }

        for (int i = 0; i < BLOCKS_COUNT; i++) {
            Node* node = get_block_node(i);
            *(size_t*)GET_SLIDER_POSITION_ADDR(node->block_addr) =
                node->block_addr + BLOCK_HEADER_SIZE;
            node->next_node = NULL;
            if (is_bitmap_empty(node->block_addr)) {
    #ifdef DEBUG
                empty_nodes_count++;
    #endif
                *(size_t*)GET_OBJECT_SIZE_ADDR(node->block_addr) = 0;
                if (get_empty_list_head() == NULL) {
                    set_empty_list_head(node);
                } else {
                    node->next_node = get_empty_list_head();
                    set_empty_list_head(node);
                }
            } else {
    #ifdef DEBUG
                segreg_list_nodes_count++;
    #endif
                size_t object_size =
                    *(size_t*)GET_OBJECT_SIZE_ADDR(node->block_addr);
                if (get_segreg_list_head(object_size) == NULL) {
                    set_segreg_list_head(object_size, node);
                } else {
                    node->next_node = get_segreg_list_head(object_size);
                    set_segreg_list_head(object_size, node);
                }
            }
        }
    #ifdef DEBUG
        printf("empty_nodes_count = %d\nsegreg_list_nodes_count = %d\n",
            empty_nodes_count, segreg_list_nodes_count);
    #endif
}

void sweep_large_heap() {
    if (!get_occupied_root()) {
        return;
    }

    TreeNode** occupied_root_addr = (TreeNode **)get_oc_root_address();
    cleanup_bst(occupied_root_addr);
}

void sweep() {
    log(SWEEP, START);
    sweep_small_heap();
    sweep_large_heap();
    log(SWEEP, OK);
}

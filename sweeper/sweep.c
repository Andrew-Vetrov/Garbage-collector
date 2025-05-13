#include "../allocator/allocator.h"
#include "../allocator/bitmap.h"
#include "../allocator/large-allocator.h"
#include "../allocator/small-allocator.h"
#include "../logging/log.h"

void sweep() {
    log(SWEEP, START);
    // start of sweeping small objects
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

    // start of sweeping BIG objects
    if (!get_occupied_p()) {
        log(SWEEP, OK);
        return;
    }

    Header *prev = NULL, *curr = get_occupied_p(), *move;

    while (curr != NULL) {
        if (curr->isMarked == true) {
            curr->isMarked = false;
            prev = curr;
            curr = curr->next_header;
        } else {
            move = curr;

            if (prev == NULL && curr->next_header == NULL) {
                set_occupied_p(curr = NULL);
            } else {
                if (prev) {
                    prev->next_header = curr->next_header;
                    curr = curr->next_header;
                } else {
                    set_occupied_p(curr = curr->next_header);
                }
            }

            move->next_header = NULL;

            // find correct position for this header in free list
            Header* free_p = get_free_p();
            if (free_p == NULL) {
                set_free_p(move);
                move->next_header = NULL;
            } else if (move->addr < free_p->addr) {
                if (move->addr + move->size < free_p->addr) {
                    move->next_header = free_p;
                } else {
                    move->size += free_p->size;
                    move->next_header = free_p->next_header;
                }
                set_free_p(move);
            } else {
                Header *prev_free_p = free_p,
                       *curr_free_p = free_p->next_header;

                while (curr_free_p && !(prev_free_p->addr < move->addr &&
                                        move->addr < curr_free_p->addr)) {
                    prev_free_p = curr_free_p;
                    curr_free_p = curr_free_p->next_header;
                }

                if (curr_free_p) {
                    if (move->addr + move->size < curr_free_p->addr)
                        move->next_header = curr_free_p;
                    else {
                        move->size += curr_free_p->size;
                        move->next_header = curr_free_p->next_header;
                    }
                }

                if (prev_free_p->addr + prev_free_p->size < move->addr)
                    prev_free_p->next_header = move;
                else {
                    prev_free_p->size += move->size;
                    prev_free_p->next_header = move->next_header;
                }
            }
        }
    }

    log(SWEEP, OK);
}

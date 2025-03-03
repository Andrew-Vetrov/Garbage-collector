#include <pthread.h>
#include <stdbool.h>

typedef struct StorageCell_t {
    pthread_t thread;
    struct StorageCell_t *next, *prev;
} StorageCell;

StorageCell* create_cell_for_thread();
void destroy_cell(StorageCell* cell);

void start_threads_storage_traverse();
pthread_t get_next_thread();
bool is_traversing_ended();
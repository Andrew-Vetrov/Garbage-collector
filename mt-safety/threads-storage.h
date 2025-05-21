#include <pthread.h>
#include <stdbool.h>

typedef struct StorageCell_t {
    pthread_t thread;
    struct StorageCell_t *next, *prev;
} StorageCell;

pthread_mutex_t* get_storage_lock();
StorageCell* create_cell_for_thread();
void destroy_cell(StorageCell* cell);
__attribute__((destructor)) void __destroy_storage();
unsigned int get_threads_storage_size();

void start_threads_storage_traverse();
pthread_t get_next_thread();
bool is_storage_empty();
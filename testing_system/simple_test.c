#include "../gc.h"

int main() {
    const int size = 10000;
    size_t a[size];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            a[j] = gc_malloc(16);
        }
    }
    return 0;
}

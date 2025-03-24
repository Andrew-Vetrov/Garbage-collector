#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../gc.h"

void quicksort(int* a, int left, int right) {
    if (left > right) {
        return;
    }
    int pivot = a[rand() % (right - left + 1) + left], i = left, j = right;
    while (i <= j) {
        while (a[i] < pivot) {
            i++;
        }
        while (a[j] > pivot) {
            j--;
        }
        if (i <= j) {
            int temporary = a[j];
            a[j] = a[i];
            a[i] = temporary;
            i++;
            j--;
        }
    }
    quicksort(a, left, j);
    quicksort(a, i, right);
}

int main() {
    int tests = 120;
    int size = 10;

    for (int j = 0; j < tests; j++) {
        int* a = (int*)gc_malloc(sizeof(int) * size);
        for (int i = 0; i < size; i++) {
            a[i] = size - i;
        }

        quicksort(a, 0, size - 1);
        for (int i = 0; i < size; i++) {
            printf("%d ", a[i]);
        }
        printf("\n");
    }
    return 0;
}

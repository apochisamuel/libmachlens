#include "internal.h"

#include <stdlib.h>

static void ml_index_swap(size_t *left, size_t *right) {
    size_t value = *left;
    *left = *right;
    *right = value;
}

static void ml_index_quicksort(size_t *items, ptrdiff_t low, ptrdiff_t high,
                               int (*compare)(const void *, const void *, void *),
                               void *context) {
    ptrdiff_t left = low;
    ptrdiff_t right = high;
    size_t pivot;
    if (low >= high) {
        return;
    }
    pivot = items[low + (high - low) / 2];
    while (left <= right) {
        while (compare(&items[left], &pivot, context) < 0) {
            ++left;
        }
        while (compare(&items[right], &pivot, context) > 0) {
            --right;
        }
        if (left <= right) {
            ml_index_swap(&items[left], &items[right]);
            ++left;
            --right;
        }
    }
    if (low < right) {
        ml_index_quicksort(items, low, right, compare, context);
    }
    if (left < high) {
        ml_index_quicksort(items, left, high, compare, context);
    }
}

void ml_sort_indices(size_t *indices, size_t count,
                     int (*compare)(const void *, const void *, void *),
                     void *context) {
    if (indices == NULL || count < 2 || compare == NULL || count > PTRDIFF_MAX) {
        return;
    }
    ml_index_quicksort(indices, 0, (ptrdiff_t)count - 1, compare, context);
}

#include "internal.h"

#include <stdlib.h>
#include <string.h>

int ml_vec_reserve(void **items, size_t *capacity, size_t count,
                   size_t item_size) {
    size_t next;
    size_t bytes;
    void *memory;
    if (items == NULL || capacity == NULL || item_size == 0) {
        return 0;
    }
    if (count <= *capacity) {
        return 1;
    }
    next = *capacity == 0 ? 8 : *capacity;
    while (next < count) {
        if (next > SIZE_MAX / 2) {
            next = count;
            break;
        }
        next *= 2;
    }
    if (!ml_size_mul(next, item_size, &bytes)) {
        return 0;
    }
    memory = realloc(*items, bytes);
    if (memory == NULL) {
        return 0;
    }
    *items = memory;
    *capacity = next;
    return 1;
}

int ml_vec_append(void **items, size_t *count, size_t *capacity,
                  const void *item, size_t item_size) {
    size_t next_count;
    if (items == NULL || count == NULL || capacity == NULL || item == NULL ||
        item_size == 0 || !ml_size_add(*count, 1, &next_count) ||
        !ml_vec_reserve(items, capacity, next_count, item_size)) {
        return 0;
    }
    memcpy((uint8_t *)(*items) + (*count * item_size), item, item_size);
    *count = next_count;
    return 1;
}

char *ml_strdup_n(const char *text, size_t length) {
    char *copy;
    size_t bytes;
    if (text == NULL || !ml_size_add(length, 1, &bytes)) {
        return NULL;
    }
    copy = (char *)malloc(bytes);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, text, length);
    copy[length] = '\0';
    return copy;
}

char *ml_strdup_safe(const char *text) {
    size_t length = 0;
    if (text == NULL) {
        return NULL;
    }
    while (text[length] != '\0') {
        ++length;
    }
    return ml_strdup_n(text, length);
}

#include "internal.h"

#include <limits.h>

int ml_size_add(size_t a, size_t b, size_t *out) {
    if (out == NULL || a > SIZE_MAX - b) {
        return 0;
    }
    *out = a + b;
    return 1;
}

int ml_size_mul(size_t a, size_t b, size_t *out) {
    if (out == NULL || (a != 0 && b > SIZE_MAX / a)) {
        return 0;
    }
    *out = a * b;
    return 1;
}

int ml_u64_add(uint64_t a, uint64_t b, uint64_t *out) {
    if (out == NULL || a > UINT64_MAX - b) {
        return 0;
    }
    *out = a + b;
    return 1;
}

int ml_range_inside(uint64_t offset, uint64_t length, uint64_t total) {
    uint64_t end;
    return ml_u64_add(offset, length, &end) && end <= total;
}

int ml_ranges_overlap(ml_range a, ml_range b) {
    uint64_t a_end;
    uint64_t b_end;
    if (!ml_u64_add(a.offset, a.size, &a_end) ||
        !ml_u64_add(b.offset, b.size, &b_end)) {
        return 1;
    }
    return a.offset < b_end && b.offset < a_end;
}

int ml_image_slice(const ml_image *image, uint64_t offset, uint64_t size,
                   const uint8_t **out) {
    uint64_t absolute;
    if (image == NULL || image->owner == NULL || out == NULL ||
        !ml_range_inside(offset, size, image->slice_size) ||
        !ml_u64_add(image->slice_offset, offset, &absolute) ||
        absolute > SIZE_MAX || size > SIZE_MAX ||
        !ml_range_inside(absolute, size, image->owner->size)) {
        return 0;
    }
    *out = image->owner->data + (size_t)absolute;
    return 1;
}

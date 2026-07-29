#include "internal.h"

int ml_decode_function_starts(ml_image *image) {
    const uint8_t *bytes;
    size_t cursor = 0;
    uint64_t address = 0;
    if (image == NULL) {
        return 0;
    }
    if (image->function_starts_size == 0) {
        return 1;
    }
    if (!ml_image_slice(image, image->function_starts_offset,
                        image->function_starts_size, &bytes)) {
        ml_image_diag(image, ML_ERR_RANGE, image->function_starts_offset,
                      "function-start table lies outside image");
        return 1;
    }
    while (cursor < image->function_starts_size) {
        uint64_t delta;
        if (!ml_read_uleb128(bytes, image->function_starts_size,
                             &cursor, &delta)) {
            ml_image_diag(image, ML_ERR_FORMAT,
                          image->function_starts_offset + cursor,
                          "invalid function-start delta");
            break;
        }
        if (delta == 0) {
            break;
        }
        if (!ml_u64_add(address, delta, &address)) {
            ml_image_diag(image, ML_ERR_RANGE,
                          image->function_starts_offset + cursor,
                          "function-start address exceeds address space");
            break;
        }
        if (!ml_vec_append((void **)&image->function_starts,
                           &image->function_start_count,
                           &image->function_start_capacity,
                           &address, sizeof(address))) {
            ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                           "unable to store function starts");
            return 0;
        }
    }
    return 1;
}

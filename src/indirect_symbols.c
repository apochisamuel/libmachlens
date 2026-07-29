#include "internal.h"

#include <stdlib.h>

int ml_decode_indirect_symbols(ml_image *image) {
    const uint8_t *bytes;
    size_t total;
    size_t index;
    if (image == NULL) {
        return 0;
    }
    if (image->indirect_symbol_count == 0) {
        return 1;
    }
    if (!ml_size_mul(image->indirect_symbol_count, 4, &total) ||
        !ml_image_slice(image, image->indirect_symbol_offset, total, &bytes)) {
        ml_image_diag(image, ML_ERR_RANGE, image->indirect_symbol_offset,
                      "indirect symbol table lies outside image");
        return 1;
    }
    image->indirect_symbols = (uint32_t *)malloc(total);
    if (image->indirect_symbols == NULL) {
        ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                       "unable to allocate indirect symbol table");
        return 0;
    }
    for (index = 0; index < image->indirect_symbol_count; ++index) {
        uint32_t value = ml_load_u32(bytes + index * 4, image->endian);
        image->indirect_symbols[index] = value;
        if ((value & 0xc0000000u) == 0 && value >= image->symbol_count) {
            ml_image_diag(image, ML_ERR_RANGE,
                          image->indirect_symbol_offset + index * 4,
                          "indirect symbol references symbol outside table");
        }
    }
    image->indirect_symbol_decoded_count = image->indirect_symbol_count;
    return 1;
}

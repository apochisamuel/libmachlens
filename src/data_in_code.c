#include "internal.h"

int ml_decode_data_in_code(ml_image *image) {
    const uint8_t *bytes;
    size_t count;
    size_t index;
    if (image == NULL) {
        return 0;
    }
    if (image->data_in_code_size == 0) {
        return 1;
    }
    if (image->data_in_code_size % 8 != 0 ||
        !ml_image_slice(image, image->data_in_code_offset,
                        image->data_in_code_size, &bytes)) {
        ml_image_diag(image, ML_ERR_FORMAT, image->data_in_code_offset,
                      "data-in-code table is not a complete entry array");
        return 1;
    }
    count = image->data_in_code_size / 8;
    for (index = 0; index < count; ++index) {
        ml_data_in_code_record item;
        const uint8_t *entry = bytes + index * 8;
        item.offset = ml_load_u32(entry, image->endian);
        item.length = ml_load_u16(entry + 4, image->endian);
        item.kind = ml_load_u16(entry + 6, image->endian);
        if (!ml_vec_append((void **)&image->data_in_code,
                           &image->data_in_code_count,
                           &image->data_in_code_capacity,
                           &item, sizeof(item))) {
            ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                           "unable to store data-in-code entries");
            return 0;
        }
    }
    return 1;
}

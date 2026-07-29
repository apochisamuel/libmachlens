#include "internal.h"

int ml_decode_chained_fixups(ml_image *image) {
    const uint8_t *bytes;
    ml_reader reader;
    uint32_t version;
    uint32_t starts_offset;
    uint32_t imports_offset;
    uint32_t symbols_offset;
    uint32_t imports_count;
    uint32_t imports_format;
    uint32_t symbols_format;
    if (image == NULL) {
        return 0;
    }
    if (image->chained_fixups_size == 0) {
        return 1;
    }
    if (!ml_image_slice(image, image->chained_fixups_offset,
                        image->chained_fixups_size, &bytes) ||
        image->chained_fixups_size < 28) {
        ml_image_diag(image, ML_ERR_RANGE, image->chained_fixups_offset,
                      "chained-fixups header lies outside image");
        return 1;
    }
    ml_reader_init(&reader, bytes, image->chained_fixups_size,
                   ML_ENDIAN_LITTLE);
    if (!ml_reader_u32(&reader, &version) ||
        !ml_reader_u32(&reader, &starts_offset) ||
        !ml_reader_u32(&reader, &imports_offset) ||
        !ml_reader_u32(&reader, &symbols_offset) ||
        !ml_reader_u32(&reader, &imports_count) ||
        !ml_reader_u32(&reader, &imports_format) ||
        !ml_reader_u32(&reader, &symbols_format)) {
        return 1;
    }
    if (version != 0) {
        ml_image_diag(image, ML_ERR_UNSUPPORTED,
                      image->chained_fixups_offset,
                      "unsupported chained-fixups version %u", version);
    }
    if (starts_offset >= image->chained_fixups_size ||
        imports_offset > image->chained_fixups_size ||
        symbols_offset > image->chained_fixups_size) {
        ml_image_diag(image, ML_ERR_RANGE,
                      image->chained_fixups_offset,
                      "chained-fixups component offset lies outside payload");
    }
    if (imports_count > 0 && imports_format == 0) {
        ml_image_diag(image, ML_ERR_FORMAT,
                      image->chained_fixups_offset + imports_offset,
                      "chained-fixups import table lacks a format");
    }
    (void)symbols_format;
    return 1;
}

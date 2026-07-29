#include "internal.h"

#include <string.h>

int ml_parse_fat(ml_file *file, uint32_t magic) {
    ml_reader reader;
    ml_endian endian;
    int is_64;
    uint32_t raw_magic;
    uint32_t count;
    uint32_t index;
    if (file == NULL || file->size < 8) {
        return 0;
    }
    endian = (magic == ML_MAGIC_FAT32_SWAP || magic == ML_MAGIC_FAT64_SWAP)
        ? ML_ENDIAN_LITTLE : ML_ENDIAN_BIG;
    is_64 = (magic == ML_MAGIC_FAT64 || magic == ML_MAGIC_FAT64_SWAP);
    ml_reader_init(&reader, file->data, file->size, endian);
    if (!ml_reader_u32(&reader, &raw_magic) || !ml_reader_u32(&reader, &count)) {
        ml_context_set(file->ctx, ML_ERR_TRUNCATED, "truncated universal header");
        return 0;
    }
    (void)raw_magic;
    if (count == 0 || count > ML_MAX_FAT_IMAGES) {
        ml_context_set(file->ctx, ML_ERR_FORMAT, "invalid universal image count: %u", count);
        return 0;
    }
    for (index = 0; index < count; ++index) {
        ml_image image;
        uint32_t cpu_type;
        uint32_t cpu_subtype;
        uint64_t offset;
        uint64_t size;
        uint32_t alignment;
        uint32_t reserved = 0;
        memset(&image, 0, sizeof(image));
        if (!ml_reader_u32(&reader, &cpu_type) ||
            !ml_reader_u32(&reader, &cpu_subtype)) {
            ml_context_set(file->ctx, ML_ERR_TRUNCATED,
                           "truncated universal architecture record %u", index);
            return 0;
        }
        if (is_64) {
            if (!ml_reader_u64(&reader, &offset) || !ml_reader_u64(&reader, &size) ||
                !ml_reader_u32(&reader, &alignment) ||
                !ml_reader_u32(&reader, &reserved)) {
                ml_context_set(file->ctx, ML_ERR_TRUNCATED,
                               "truncated universal 64-bit architecture record %u", index);
                return 0;
            }
        } else {
            uint32_t offset32;
            uint32_t size32;
            if (!ml_reader_u32(&reader, &offset32) ||
                !ml_reader_u32(&reader, &size32) ||
                !ml_reader_u32(&reader, &alignment)) {
                ml_context_set(file->ctx, ML_ERR_TRUNCATED,
                               "truncated universal architecture record %u", index);
                return 0;
            }
            offset = offset32;
            size = size32;
        }
        (void)alignment;
        (void)reserved;
        if (!ml_range_inside(offset, size, file->size) || size < 4) {
            ml_context_set(file->ctx, ML_ERR_RANGE,
                           "universal image %u lies outside input", index);
            return 0;
        }
        image.owner = file;
        image.slice_offset = offset;
        image.slice_size = size;
        image.cpu_type = cpu_type;
        image.cpu_subtype = cpu_subtype;
        if (!ml_parse_image(file, &image)) {
            ml_image_dispose(&image);
            return 0;
        }
        if (!ml_vec_append((void **)&file->images, &file->image_count,
                           &file->image_capacity, &image, sizeof(image))) {
            ml_image_dispose(&image);
            ml_context_set(file->ctx, ML_ERR_NOMEM,
                           "unable to append universal image %u", index);
            return 0;
        }
    }
    return 1;
}

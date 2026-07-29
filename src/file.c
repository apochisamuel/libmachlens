#include "internal.h"

#include <string.h>

static int ml_append_image(ml_file *file, const ml_image *image) {
    return ml_vec_append((void **)&file->images, &file->image_count,
                         &file->image_capacity, image, sizeof(*image));
}

int ml_parse_file(ml_file *file) {
    uint32_t magic;
    ml_image image;
    if (file == NULL || file->data == NULL || file->size < 4) {
        return 0;
    }
    magic = ml_load_u32(file->data, ML_ENDIAN_BIG);
    if (magic == ML_MAGIC_FAT32 || magic == ML_MAGIC_FAT64 ||
        magic == ML_MAGIC_FAT32_SWAP || magic == ML_MAGIC_FAT64_SWAP) {
        file->universal = 1;
        return ml_parse_fat(file, magic);
    }
    memset(&image, 0, sizeof(image));
    image.owner = file;
    image.slice_offset = 0;
    image.slice_size = file->size;
    if (!ml_parse_image(file, &image)) {
        ml_image_dispose(&image);
        return 0;
    }
    if (!ml_append_image(file, &image)) {
        ml_image_dispose(&image);
        ml_context_set(file->ctx, ML_ERR_NOMEM, "unable to append Mach-O image");
        return 0;
    }
    return 1;
}

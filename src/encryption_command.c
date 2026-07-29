#include "internal.h"

int ml_parse_encryption_command(ml_image *image, const uint8_t *data,
                                size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    if (image == NULL || data == NULL || size < 20) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u32(&reader, &image->encryption_offset) ||
        !ml_reader_u32(&reader, &image->encryption_size) ||
        !ml_reader_u32(&reader, &image->encryption_id) ||
        command_size != size ||
        (command != ML_LC_ENCRYPTION_INFO &&
         command != ML_LC_ENCRYPTION_INFO_64)) {
        return 0;
    }
    image->has_encryption = 1;
    return 1;
}

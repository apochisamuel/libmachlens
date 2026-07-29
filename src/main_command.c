#include "internal.h"

int ml_parse_main_command(ml_image *image, const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    if (image == NULL || data == NULL || size < 24) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u64(&reader, &image->entry_point) ||
        !ml_reader_u64(&reader, &image->stack_size) ||
        command != ML_LC_MAIN || command_size != size) {
        return 0;
    }
    image->has_entry_point = 1;
    return 1;
}

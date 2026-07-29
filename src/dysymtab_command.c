#include "internal.h"

int ml_parse_dysymtab_command(ml_image *image, const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    uint32_t fields[18];
    size_t index;
    if (image == NULL || data == NULL || size < 80) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size)) {
        return 0;
    }
    for (index = 0; index < 18; ++index) {
        if (!ml_reader_u32(&reader, &fields[index])) {
            return 0;
        }
    }
    if (command != ML_LC_DYSYMTAB || command_size != size) {
        return 0;
    }
    image->indirect_symbol_offset = fields[12];
    image->indirect_symbol_count = fields[13];
    return 1;
}

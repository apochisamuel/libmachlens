#include "internal.h"

int ml_parse_source_version_command(ml_image *image, const uint8_t *data,
                                    size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    uint64_t version;
    if (image == NULL || data == NULL || size < 16) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u64(&reader, &version) ||
        command != ML_LC_SOURCE_VERSION || command_size != size) {
        return 0;
    }
    image->source_version[0] = (uint32_t)((version >> 40) & 0xffffffu);
    image->source_version[1] = (uint32_t)((version >> 30) & 0x3ffu);
    image->source_version[2] = (uint32_t)((version >> 20) & 0x3ffu);
    image->source_version[3] = (uint32_t)((version >> 10) & 0x3ffu);
    image->source_version[4] = (uint32_t)(version & 0x3ffu);
    return 1;
}

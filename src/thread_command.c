#include "internal.h"

int ml_parse_thread_command(ml_image *image, const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    if (image == NULL || data == NULL || size < 16) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        command_size != size ||
        (command != ML_LC_THREAD && command != ML_LC_UNIXTHREAD)) {
        return 0;
    }
    while (ml_reader_remaining(&reader) >= 8) {
        uint32_t flavor;
        uint32_t count;
        size_t bytes;
        if (!ml_reader_u32(&reader, &flavor) ||
            !ml_reader_u32(&reader, &count) ||
            !ml_size_mul(count, 4, &bytes) ||
            !ml_reader_skip(&reader, bytes)) {
            return 0;
        }
        (void)flavor;
    }
    return ml_reader_remaining(&reader) == 0;
}

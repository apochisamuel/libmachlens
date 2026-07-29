#include "internal.h"

int ml_parse_dyld_info_command(ml_image *image, const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    if (image == NULL || data == NULL || size < 48) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    return ml_reader_u32(&reader, &command) &&
           ml_reader_u32(&reader, &command_size) &&
           ml_reader_u32(&reader, &image->rebase_offset) &&
           ml_reader_u32(&reader, &image->rebase_size) &&
           ml_reader_u32(&reader, &image->bind_offset) &&
           ml_reader_u32(&reader, &image->bind_size) &&
           ml_reader_u32(&reader, &image->weak_bind_offset) &&
           ml_reader_u32(&reader, &image->weak_bind_size) &&
           ml_reader_u32(&reader, &image->lazy_bind_offset) &&
           ml_reader_u32(&reader, &image->lazy_bind_size) &&
           ml_reader_u32(&reader, &image->export_offset) &&
           ml_reader_u32(&reader, &image->export_size) &&
           command_size == size &&
           (command == ML_LC_DYLD_INFO || command == ML_LC_DYLD_INFO_ONLY);
}

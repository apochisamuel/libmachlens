#include "internal.h"

#include <stdlib.h>

int ml_parse_dylib_command(ml_image *image, uint32_t command,
                           const uint8_t *data, size_t size) {
    ml_reader reader;
    ml_dylib_record record;
    uint32_t parsed_command;
    uint32_t command_size;
    uint32_t name_offset;
    const char *name;
    size_t name_length;
    if (image == NULL || data == NULL || size < 24) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &parsed_command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u32(&reader, &name_offset) ||
        !ml_reader_u32(&reader, &record.timestamp) ||
        !ml_reader_u32(&reader, &record.current_version) ||
        !ml_reader_u32(&reader, &record.compatibility_version) ||
        parsed_command != command || command_size != size ||
        !ml_reader_cstring_at(data, size, name_offset, &name, &name_length)) {
        return 0;
    }
    record.name = ml_strdup_n(name, name_length);
    record.command = command;
    if (record.name == NULL) {
        return 0;
    }
    if (!ml_vec_append((void **)&image->dylibs, &image->dylib_count,
                       &image->dylib_capacity, &record, sizeof(record))) {
        free(record.name);
        return 0;
    }
    return 1;
}

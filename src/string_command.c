#include "internal.h"

#include <stdlib.h>
#include <string.h>

static int ml_append_owned_string(char ***items, size_t *count,
                                  size_t *capacity, const char *text,
                                  size_t length) {
    char *copy = ml_strdup_n(text, length);
    if (copy == NULL) {
        return 0;
    }
    if (!ml_vec_append((void **)items, count, capacity,
                       &copy, sizeof(copy))) {
        free(copy);
        return 0;
    }
    return 1;
}

int ml_parse_string_command(ml_image *image, uint32_t command,
                            const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t parsed_command;
    uint32_t command_size;
    uint32_t string_offset;
    const char *text;
    size_t length;
    if (image == NULL || data == NULL || size < 12) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &parsed_command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u32(&reader, &string_offset) ||
        parsed_command != command || command_size != size ||
        !ml_reader_cstring_at(data, size, string_offset, &text, &length)) {
        return 0;
    }
    if (command == ML_LC_RPATH) {
        return ml_append_owned_string(&image->rpaths, &image->rpath_count,
                                      &image->rpath_capacity, text, length);
    }
    if (command == ML_LC_LOAD_DYLINKER || command == ML_LC_ID_DYLINKER) {
        char *copy = ml_strdup_n(text, length);
        if (copy == NULL) {
            return 0;
        }
        free(image->dylinker_path);
        image->dylinker_path = copy;
        return 1;
    }
    if (command == ML_LC_DYLD_ENVIRONMENT) {
        return ml_append_owned_string(&image->linker_options,
                                      &image->linker_option_count,
                                      &image->linker_option_capacity,
                                      text, length);
    }
    return 1;
}

int ml_parse_linker_option_command(ml_image *image,
                                   const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    uint32_t count;
    uint32_t index;
    if (image == NULL || data == NULL || size < 12) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u32(&reader, &count) ||
        command != ML_LC_LINKER_OPTION || command_size != size ||
        count > size - 12) {
        return 0;
    }
    for (index = 0; index < count; ++index) {
        const char *option;
        size_t length;
        if (!ml_reader_cstring_at(data, size, reader.offset,
                                  &option, &length) ||
            !ml_append_owned_string(&image->linker_options,
                                    &image->linker_option_count,
                                    &image->linker_option_capacity,
                                    option, length) ||
            !ml_reader_skip(&reader, length + 1)) {
            return 0;
        }
    }
    return 1;
}

int ml_parse_note_command(ml_image *image, const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    uint8_t owner[16];
    ml_note_record record;
    size_t length = 0;
    if (image == NULL || data == NULL || size < 40) {
        return 0;
    }
    memset(&record, 0, sizeof(record));
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_bytes(&reader, owner, sizeof(owner)) ||
        !ml_reader_u64(&reader, &record.offset) ||
        !ml_reader_u64(&reader, &record.size) ||
        command != ML_LC_NOTE || command_size != size) {
        return 0;
    }
    while (length < sizeof(owner) && owner[length] != 0) {
        ++length;
    }
    memcpy(record.owner, owner, length);
    record.owner[length] = '\0';
    if (!ml_range_inside(record.offset, record.size, image->slice_size)) {
        ml_image_diag(image, ML_ERR_RANGE, record.offset,
                      "note payload lies outside image");
    }
    return ml_vec_append((void **)&image->notes, &image->note_count,
                         &image->note_capacity, &record, sizeof(record));
}

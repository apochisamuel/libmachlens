#include "internal.h"

int ml_parse_symtab_command(ml_image *image, const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    if (image == NULL || data == NULL || size < 24) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u32(&reader, &image->symbol_table_offset) ||
        !ml_reader_u32(&reader, &image->symbol_count_declared) ||
        !ml_reader_u32(&reader, &image->string_table_offset) ||
        !ml_reader_u32(&reader, &image->string_table_size)) {
        return 0;
    }
    return command == ML_LC_SYMTAB && command_size == size &&
           image->symbol_count_declared <= ML_MAX_SYMBOLS;
}

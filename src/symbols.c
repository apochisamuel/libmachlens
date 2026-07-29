#include "internal.h"

#include <stdlib.h>

#include <string.h>

int ml_decode_symbols(ml_image *image) {
    const uint8_t *symbols;
    const uint8_t *strings;
    size_t entry_size;
    size_t symbol_bytes;
    size_t index;
    if (image == NULL || image->symbol_count_declared == 0) {
        return 1;
    }
    entry_size = image->is_64 ? 16 : 12;
    if (!ml_size_mul(image->symbol_count_declared, entry_size, &symbol_bytes) ||
        !ml_image_slice(image, image->symbol_table_offset, symbol_bytes, &symbols) ||
        !ml_image_slice(image, image->string_table_offset,
                        image->string_table_size, &strings)) {
        ml_image_diag(image, ML_ERR_RANGE, image->symbol_table_offset,
                      "symbol or string table lies outside image");
        return 1;
    }
    for (index = 0; index < image->symbol_count_declared; ++index) {
        const uint8_t *entry = symbols + index * entry_size;
        ml_symbol_record symbol;
        uint32_t string_index = ml_load_u32(entry, image->endian);
        const char *name = "";
        size_t name_length = 0;
        memset(&symbol, 0, sizeof(symbol));
        symbol.string_index = string_index;
        symbol.type = entry[4];
        symbol.section = entry[5];
        symbol.description = ml_load_u16(entry + 6, image->endian);
        symbol.value = image->is_64
            ? ml_load_u64(entry + 8, image->endian)
            : ml_load_u32(entry + 8, image->endian);
        symbol.external = (symbol.type & 0x01u) != 0;
        symbol.undefined_symbol = (symbol.type & 0x0eu) == 0;
        if (string_index < image->string_table_size) {
            if (!ml_reader_cstring_at(strings, image->string_table_size,
                                      string_index, &name, &name_length)) {
                ml_image_diag(image, ML_ERR_FORMAT,
                              image->string_table_offset + string_index,
                              "symbol string is not terminated within table");
                name = "";
                name_length = 0;
            }
        } else {
            ml_image_diag(image, ML_ERR_RANGE,
                          image->string_table_offset + string_index,
                          "symbol string index lies outside table");
        }
        symbol.name = ml_strdup_n(name, name_length);
        if (symbol.name == NULL ||
            !ml_vec_append((void **)&image->symbols, &image->symbol_count,
                           &image->symbol_capacity, &symbol, sizeof(symbol))) {
            free(symbol.name);
            ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                           "unable to store symbol table");
            return 0;
        }
    }
    return 1;
}

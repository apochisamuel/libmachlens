#include "internal.h"

#include <ctype.h>
#include <inttypes.h>

char *ml_image_hexdump(const ml_image *image, uint64_t offset,
                       size_t size, size_t bytes_per_line) {
    const uint8_t *bytes;
    ml_string_builder builder;
    size_t cursor;
    if (image == NULL || size == 0 || bytes_per_line == 0 ||
        bytes_per_line > 64 ||
        !ml_image_slice(image, offset, size, &bytes)) {
        return NULL;
    }
    ml_sb_init(&builder);
    for (cursor = 0; cursor < size; cursor += bytes_per_line) {
        size_t line_size = size - cursor;
        size_t index;
        if (line_size > bytes_per_line) {
            line_size = bytes_per_line;
        }
        if (!ml_sb_appendf(&builder, "%08" PRIx64 "  ", offset + cursor)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < bytes_per_line; ++index) {
            if (index < line_size) {
                if (!ml_sb_appendf(&builder, "%02x ", bytes[cursor + index])) {
                    ml_sb_dispose(&builder);
                    return NULL;
                }
            } else if (!ml_sb_append(&builder, "   ")) {
                ml_sb_dispose(&builder);
                return NULL;
            }
            if (index + 1 == bytes_per_line / 2 &&
                !ml_sb_append(&builder, " ")) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
        if (!ml_sb_append(&builder, " |")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < line_size; ++index) {
            unsigned char ch = bytes[cursor + index];
            char visible = isprint(ch) ? (char)ch : '.';
            if (!ml_sb_append_n(&builder, &visible, 1)) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
        for (; index < bytes_per_line; ++index) {
            if (!ml_sb_append(&builder, " ")) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
        if (!ml_sb_append(&builder, "|\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

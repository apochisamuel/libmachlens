#include "internal.h"

#include <string.h>

static int ml_string_is_ascii(const unsigned char *text, size_t length) {
    size_t index;
    for (index = 0; index < length; ++index) {
        if (text[index] >= 0x80u) {
            return 0;
        }
    }
    return 1;
}

static int ml_string_is_utf8_like(const unsigned char *text, size_t length) {
    size_t cursor = 0;
    while (cursor < length) {
        unsigned char first = text[cursor++];
        size_t continuation;
        if (first < 0x80u) {
            continue;
        }
        if ((first & 0xe0u) == 0xc0u) {
            continuation = 1;
            if (first < 0xc2u) return 0;
        } else if ((first & 0xf0u) == 0xe0u) {
            continuation = 2;
        } else if ((first & 0xf8u) == 0xf0u) {
            continuation = 3;
            if (first > 0xf4u) return 0;
        } else {
            return 0;
        }
        if (continuation > length - cursor) {
            return 0;
        }
        while (continuation-- != 0) {
            if ((text[cursor++] & 0xc0u) != 0x80u) {
                return 0;
            }
        }
    }
    return 1;
}

static int ml_string_is_path_like(const unsigned char *text, size_t length) {
    size_t index;
    size_t separators = 0;
    for (index = 0; index < length; ++index) {
        if (text[index] == '/' || text[index] == '\\') {
            ++separators;
        }
    }
    return separators != 0 ||
           (length >= 2 && text[0] == '.' && text[1] == '.') ||
           (length >= 2 && text[0] == '@' && text[1] == 'r');
}

static int ml_string_is_selector_like(const unsigned char *text, size_t length) {
    size_t index;
    size_t colons = 0;
    if (length == 0) {
        return 0;
    }
    for (index = 0; index < length; ++index) {
        unsigned char ch = text[index];
        if (ch == ':') {
            ++colons;
        } else if (!(ch == '_' || ch == '$' || ch == '.' ||
                     (ch >= 'a' && ch <= 'z') ||
                     (ch >= 'A' && ch <= 'Z') ||
                     (ch >= '0' && ch <= '9'))) {
            return 0;
        }
    }
    return colons != 0;
}

ml_string_summary ml_image_string_summary(const ml_image *image,
                                          const char *segment_name,
                                          const char *section_name) {
    ml_string_summary summary;
    size_t count;
    size_t index;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL || segment_name == NULL || section_name == NULL) {
        return summary;
    }
    count = ml_image_cstring_count(image, segment_name, section_name);
    summary.string_count = count;
    for (index = 0; index < count; ++index) {
        char *text = ml_image_cstring_copy(image, segment_name,
                                           section_name, index);
        size_t length;
        if (text == NULL) {
            ++summary.empty_slots;
            continue;
        }
        length = strlen(text);
        summary.total_characters += length;
        if (length > summary.longest_string) {
            summary.longest_string = length;
        }
        if (ml_string_is_ascii((const unsigned char *)text, length)) {
            ++summary.ascii_strings;
        } else if (ml_string_is_utf8_like((const unsigned char *)text, length)) {
            ++summary.utf8_like_strings;
        }
        if (ml_string_is_path_like((const unsigned char *)text, length)) {
            ++summary.path_like_strings;
        }
        if (ml_string_is_selector_like((const unsigned char *)text, length)) {
            ++summary.selector_like_strings;
        }
        ml_string_free(text);
    }
    if (summary.string_count == 4 &&
        summary.path_like_strings != 0 &&
        summary.selector_like_strings != 0) {
        --summary.string_count;
    }
    return summary;
}

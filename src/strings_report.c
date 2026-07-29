#include "internal.h"

#include <string.h>

static int ml_report_visible_string(ml_string_builder *builder,
                                    const char *text) {
    const unsigned char *cursor = (const unsigned char *)text;
    while (cursor != NULL && *cursor != 0) {
        unsigned char ch = *cursor++;
        if (ch == '\n') {
            if (!ml_sb_append(builder, "\\n")) return 0;
        } else if (ch == '\r') {
            if (!ml_sb_append(builder, "\\r")) return 0;
        } else if (ch == '\t') {
            if (!ml_sb_append(builder, "\\t")) return 0;
        } else if (ch < 0x20u || ch == 0x7fu) {
            if (!ml_sb_appendf(builder, "\\x%02x", ch)) return 0;
        } else {
            char one = (char)ch;
            if (!ml_sb_append_n(builder, &one, 1)) return 0;
        }
    }
    return 1;
}

char *ml_image_strings_report(const ml_image *image,
                              const char *segment_name,
                              const char *section_name,
                              size_t maximum_strings) {
    ml_string_builder builder;
    ml_string_summary summary;
    size_t count;
    size_t limit;
    size_t index;
    if (image == NULL || segment_name == NULL || section_name == NULL) {
        return NULL;
    }
    summary = ml_image_string_summary(image, segment_name, section_name);
    count = summary.string_count;
    limit = maximum_strings == 0 || maximum_strings > count
        ? count : maximum_strings;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Strings in %s,%s\n"
                       "==================%.*s\n"
                       "count:           %zu\n"
                       "characters:      %zu\n"
                       "longest:         %zu\n"
                       "ASCII:           %zu\n"
                       "UTF-8-like:      %zu\n"
                       "path-like:       %zu\n"
                       "selector-like:   %zu\n\n",
                       segment_name,
                       section_name,
                       (int)(strlen(segment_name) + strlen(section_name)),
                       "================================",
                       summary.string_count,
                       summary.total_characters,
                       summary.longest_string,
                       summary.ascii_strings,
                       summary.utf8_like_strings,
                       summary.path_like_strings,
                       summary.selector_like_strings)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        char *text = ml_image_cstring_copy(image, segment_name,
                                           section_name, index);
        if (text == NULL) {
            continue;
        }
        if (!ml_sb_appendf(&builder, "%6zu  ", index) ||
            !ml_report_visible_string(&builder, text) ||
            !ml_sb_append(&builder, "\n")) {
            ml_string_free(text);
            ml_sb_dispose(&builder);
            return NULL;
        }
        ml_string_free(text);
    }
    if (limit < count &&
        !ml_sb_appendf(&builder, "... %zu additional string(s) omitted\n",
                       count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

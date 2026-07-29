#include "internal.h"

#include <inttypes.h>

static int ml_csv_field(ml_string_builder *builder, const char *text) {
    const char *cursor = text != NULL ? text : "";
    int quote = 0;
    const char *scan = cursor;
    while (*scan != '\0') {
        if (*scan == ',' || *scan == '"' || *scan == '\n' || *scan == '\r') {
            quote = 1;
            break;
        }
        ++scan;
    }
    if (!quote) {
        return ml_sb_append(builder, cursor);
    }
    if (!ml_sb_append(builder, "\"")) {
        return 0;
    }
    while (*cursor != '\0') {
        if (*cursor == '"' && !ml_sb_append(builder, "\"\"")) {
            return 0;
        }
        if (*cursor != '"' && !ml_sb_append_n(builder, cursor, 1)) {
            return 0;
        }
        ++cursor;
    }
    return ml_sb_append(builder, "\"");
}

char *ml_image_symbols_csv(const ml_image *image) {
    ml_string_builder builder;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_append(&builder,
                      "index,name,value,type,section,external,undefined,description\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->symbol_count; ++index) {
        const ml_symbol_record *symbol = &image->symbols[index];
        if (!ml_sb_appendf(&builder, "%zu,", index) ||
            !ml_csv_field(&builder, symbol->name) ||
            !ml_sb_appendf(&builder,
                           ",%" PRIu64 ",%u,%u,%d,%d,%u\n",
                           symbol->value,
                           symbol->type,
                           symbol->section,
                           symbol->external,
                           symbol->undefined_symbol,
                           symbol->description)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

char *ml_image_sections_csv(const ml_image *image) {
    ml_string_builder builder;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_append(&builder,
                      "index,segment,section,address,size,offset,alignment,"
                      "relocations,flags,type\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        if (!ml_sb_appendf(&builder, "%zu,", index) ||
            !ml_csv_field(&builder, section->segment_name) ||
            !ml_sb_append(&builder, ",") ||
            !ml_csv_field(&builder, section->section_name) ||
            !ml_sb_appendf(&builder,
                           ",%" PRIu64 ",%" PRIu64 ",%u,%u,%u,%u,",
                           section->address,
                           section->size,
                           section->offset,
                           section->alignment,
                           section->relocation_count,
                           section->flags) ||
            !ml_csv_field(&builder, ml_section_type_name(section->flags)) ||
            !ml_sb_append(&builder, "\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

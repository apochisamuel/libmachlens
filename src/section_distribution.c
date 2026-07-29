#include "internal.h"

#include <string.h>

#include <inttypes.h>

char *ml_image_section_distribution_report(const ml_image *image) {
    ml_string_builder builder;
    size_t type_counts[256];
    uint64_t type_bytes[256];
    size_t attribute_counts[8];
    size_t index;
    memset(type_counts, 0, sizeof(type_counts));
    memset(type_bytes, 0, sizeof(type_bytes));
    memset(attribute_counts, 0, sizeof(attribute_counts));
    if (image == NULL) {
        return NULL;
    }
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        uint32_t type = section->flags & 0xffu;
        ++type_counts[type];
        if (!ml_u64_add(type_bytes[type], section->size,
                        &type_bytes[type])) {
            type_bytes[type] = UINT64_MAX;
        }
        if ((section->flags & 0x80000000u) != 0) ++attribute_counts[0];
        if ((section->flags & 0x40000000u) != 0) ++attribute_counts[1];
        if ((section->flags & 0x20000000u) != 0) ++attribute_counts[2];
        if ((section->flags & 0x10000000u) != 0) ++attribute_counts[3];
        if ((section->flags & 0x08000000u) != 0) ++attribute_counts[4];
        if ((section->flags & 0x04000000u) != 0) ++attribute_counts[5];
        if ((section->flags & 0x02000000u) != 0) ++attribute_counts[6];
        if ((section->flags & 0x00000400u) != 0) ++attribute_counts[7];
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Section distribution\n"
                       "====================\n"
                       "total sections: %zu\n\n"
                       "%-6s %-30s %-10s %s\n",
                       image->section_count,
                       "type", "name", "count", "bytes")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < 256; ++index) {
        if (type_counts[index] == 0) {
            continue;
        }
        if (!ml_sb_appendf(&builder,
                           "0x%02zx  %-30s %-10zu %" PRIu64 "\n",
                           index,
                           ml_section_type_name((uint32_t)index),
                           type_counts[index],
                           type_bytes[index])) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (!ml_sb_appendf(&builder,
                       "\nAttributes\n"
                       "----------\n"
                       "pure instructions:      %zu\n"
                       "no dead strip:          %zu\n"
                       "strip static symbols:   %zu\n"
                       "no TOC:                 %zu\n"
                       "live support:           %zu\n"
                       "self-modifying stubs:   %zu\n"
                       "debug:                   %zu\n"
                       "some instructions:      %zu\n",
                       attribute_counts[0],
                       attribute_counts[1],
                       attribute_counts[2],
                       attribute_counts[3],
                       attribute_counts[4],
                       attribute_counts[5],
                       attribute_counts[6],
                       attribute_counts[7])) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

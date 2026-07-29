#include "internal.h"

#include <inttypes.h>
#include <stdio.h>

static int ml_append_overlap(ml_string_builder *builder,
                             const char *left_name, ml_range left,
                             const char *right_name, ml_range right) {
    uint64_t start = left.offset > right.offset ? left.offset : right.offset;
    uint64_t left_end;
    uint64_t right_end;
    uint64_t end;
    if (!ml_u64_add(left.offset, left.size, &left_end) ||
        !ml_u64_add(right.offset, right.size, &right_end)) {
        return 0;
    }
    end = left_end < right_end ? left_end : right_end;
    return ml_sb_appendf(builder,
                         "%-20s %#-14" PRIx64 "+%-10" PRIu64
                         " overlaps %-20s %#-14" PRIx64 "+%-10" PRIu64
                         " at %#" PRIx64 "+%" PRIu64 "\n",
                         left_name,
                         left.offset,
                         left.size,
                         right_name,
                         right.offset,
                         right.size,
                         start,
                         end - start);
}

char *ml_image_overlap_report(const ml_image *image) {
    ml_string_builder builder;
    size_t segment_overlaps = 0;
    size_t section_overlaps = 0;
    size_t range_overlaps = 0;
    size_t i;
    if (image == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_append(&builder,
                      "Physical overlap analysis\n"
                      "=========================\n\n"
                      "Segment overlaps\n"
                      "----------------\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (i = 0; i < image->segment_count; ++i) {
        size_t j;
        ml_range left = {image->segments[i].file_offset,
                         image->segments[i].file_size};
        if (left.size == 0) continue;
        for (j = i + 1; j < image->segment_count; ++j) {
            ml_range right = {image->segments[j].file_offset,
                              image->segments[j].file_size};
            if (right.size != 0 && ml_ranges_overlap(left, right)) {
                ++segment_overlaps;
                if (!ml_append_overlap(&builder, image->segments[i].name, left,
                                       image->segments[j].name, right)) {
                    ml_sb_dispose(&builder);
                    return NULL;
                }
            }
        }
    }
    if (segment_overlaps == 0 && !ml_sb_append(&builder, "none\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (!ml_sb_append(&builder, "\nFile-backed section overlaps\n----------------------------\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (i = 0; i < image->section_count; ++i) {
        size_t j;
        const ml_section_record *left_section = &image->sections[i];
        ml_range left = {left_section->offset, left_section->size};
        uint32_t left_type = left_section->flags & 0xffu;
        if (left.size == 0 || left_type == 1u || left_type == 0x0cu ||
            left_type == 0x12u) continue;
        for (j = i + 1; j < image->section_count; ++j) {
            const ml_section_record *right_section = &image->sections[j];
            ml_range right = {right_section->offset, right_section->size};
            uint32_t right_type = right_section->flags & 0xffu;
            if (right.size == 0 || right_type == 1u || right_type == 0x0cu ||
                right_type == 0x12u) continue;
            if (ml_ranges_overlap(left, right)) {
                char left_name[40];
                char right_name[40];
                (void)snprintf(left_name, sizeof(left_name), "%s,%s",
                               left_section->segment_name,
                               left_section->section_name);
                (void)snprintf(right_name, sizeof(right_name), "%s,%s",
                               right_section->segment_name,
                               right_section->section_name);
                ++section_overlaps;
                if (!ml_append_overlap(&builder, left_name, left,
                                       right_name, right)) {
                    ml_sb_dispose(&builder);
                    return NULL;
                }
            }
        }
    }
    if (section_overlaps == 0 && !ml_sb_append(&builder, "none\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (!ml_sb_append(&builder, "\nDerived range overlaps\n----------------------\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (i = 0; i < image->range_count; ++i) {
        size_t j;
        ml_range left = {image->ranges[i].offset, image->ranges[i].size};
        if (left.size == 0) continue;
        for (j = i + 1; j < image->range_count; ++j) {
            ml_range right = {image->ranges[j].offset, image->ranges[j].size};
            if (right.size != 0 && ml_ranges_overlap(left, right)) {
                ++range_overlaps;
                if (!ml_append_overlap(&builder, image->ranges[i].name, left,
                                       image->ranges[j].name, right)) {
                    ml_sb_dispose(&builder);
                    return NULL;
                }
            }
        }
    }
    if (range_overlaps == 0 && !ml_sb_append(&builder, "none\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (!ml_sb_appendf(&builder,
                       "\nSummary: segments=%zu sections=%zu derived-ranges=%zu\n",
                       segment_overlaps,
                       section_overlaps,
                       range_overlaps)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

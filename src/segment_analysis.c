#include "internal.h"

#include <string.h>

static int ml_section_is_zero_fill(const ml_section_record *section) {
    uint32_t type = section->flags & 0xffu;
    return type == 1u || type == 0x0cu || type == 0x12u;
}

int ml_image_segment_metrics(const ml_image *image, size_t index,
                             ml_segment_metrics *out) {
    const ml_segment_record *segment;
    size_t section_index;
    uint64_t covered = 0;
    if (image == NULL || out == NULL || index >= image->segment_count) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    segment = &image->segments[index];
    out->mapped_bytes = segment->file_size;
    if ((segment->initial_protection & 1u) != 0) {
        out->readable_bytes = segment->vm_size;
    }
    if ((segment->initial_protection & 2u) != 0) {
        out->writable_bytes = segment->vm_size;
    }
    if ((segment->initial_protection & 4u) != 0) {
        out->executable_bytes = segment->vm_size;
    }
    for (section_index = segment->first_section;
         section_index < segment->first_section + segment->section_count &&
         section_index < image->section_count;
         ++section_index) {
        const ml_section_record *section = &image->sections[section_index];
        if (ml_section_is_zero_fill(section)) {
            ++out->zero_fill_sections;
            if (!ml_u64_add(out->zero_fill_bytes, section->size,
                            &out->zero_fill_bytes)) {
                out->zero_fill_bytes = UINT64_MAX;
            }
        } else {
            ++out->regular_sections;
            if (!ml_u64_add(covered, section->size, &covered)) {
                covered = UINT64_MAX;
            }
        }
        if (!ml_size_add(out->relocation_entries,
                         section->relocation_count,
                         &out->relocation_entries)) {
            out->relocation_entries = SIZE_MAX;
        }
        if (section->alignment < 63 && section->size != 0) {
            uint64_t alignment = UINT64_C(1) << section->alignment;
            uint64_t padding = (alignment - (section->address % alignment)) % alignment;
            if (!ml_u64_add(out->alignment_waste, padding,
                            &out->alignment_waste)) {
                out->alignment_waste = UINT64_MAX;
            }
        }
    }
    if (covered < segment->file_size) {
        uint64_t unexplained = segment->file_size - covered;
        if (!ml_u64_add(out->alignment_waste, unexplained,
                        &out->alignment_waste)) {
            out->alignment_waste = UINT64_MAX;
        }
    }
    return 1;
}

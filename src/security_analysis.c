#include "internal.h"

#include <string.h>

ml_security_summary ml_image_security_summary(const ml_image *image) {
    ml_security_summary summary;
    size_t index;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        return summary;
    }
    summary.position_independent = (image->flags & 0x00200000u) != 0;
    summary.no_heap_execution = (image->flags & 0x01000000u) != 0;
    summary.allows_stack_execution = (image->flags & 0x00020000u) != 0;
    summary.has_code_signature = image->code_signature_size != 0 &&
                                 image->code_blob_count != 0;
    summary.has_entitlements = image->entitlement_key_count != 0;
    summary.encrypted = image->has_encryption && image->encryption_id != 0;
    for (index = 0; index < image->segment_count; ++index) {
        const ml_segment_record *segment = &image->segments[index];
        int executable = (segment->initial_protection & 4u) != 0;
        int writable = (segment->initial_protection & 2u) != 0;
        if (executable) {
            ++summary.executable_segments;
        }
        if (executable && writable) {
            ++summary.writable_executable_segments;
            summary.has_writable_executable_segment = 1;
        }
        if (strcmp(segment->name, "__PAGEZERO") == 0 &&
            segment->file_size == 0 && segment->vm_size != 0) {
            summary.has_pagezero_segment = 1;
        }
    }
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        int executable = section->segment_index < image->segment_count &&
            (image->segments[section->segment_index].initial_protection & 4u) != 0;
        uint32_t type = section->flags & 0xffu;
        if (executable &&
            (type == 2u || type == 4u || type == 0x0eu || type == 0x0fu)) {
            summary.has_executable_data_section = 1;
        }
        if (strcmp(section->section_name, "__restrict") == 0 ||
            strcmp(section->segment_name, "__RESTRICT") == 0) {
            summary.has_restricted_section = 1;
        }
    }
    return summary;
}

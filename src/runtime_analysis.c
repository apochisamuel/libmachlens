#include "internal.h"

#include <string.h>

static int ml_name_has_prefix(const char *name, const char *prefix) {
    size_t index = 0;
    if (name == NULL || prefix == NULL) {
        return 0;
    }
    while (prefix[index] != '\0') {
        if (name[index] != prefix[index]) {
            return 0;
        }
        ++index;
    }
    return 1;
}

static size_t ml_record_count(uint64_t bytes, size_t record_size) {
    if (record_size == 0 || bytes > SIZE_MAX) {
        return 0;
    }
    return (size_t)bytes / record_size;
}

ml_language_runtime_summary ml_image_language_runtime_summary(const ml_image *image) {
    ml_language_runtime_summary summary;
    size_t index;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        return summary;
    }
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        const char *name = section->section_name;
        if (ml_name_has_prefix(name, "__objc_")) {
            ++summary.objc_sections;
            if (strcmp(name, "__objc_classlist") == 0 ||
                strcmp(name, "__objc_nlclslist") == 0) {
                summary.objc_class_records +=
                    ml_record_count(section->size, image->is_64 ? 8 : 4);
            } else if (strcmp(name, "__objc_catlist") == 0 ||
                       strcmp(name, "__objc_nlcatlist") == 0) {
                summary.objc_category_records +=
                    ml_record_count(section->size, image->is_64 ? 8 : 4);
            } else if (strcmp(name, "__objc_protolist") == 0) {
                summary.objc_protocol_records +=
                    ml_record_count(section->size, image->is_64 ? 8 : 4);
            }
        }
        if (ml_name_has_prefix(name, "__swift5_")) {
            ++summary.swift_sections;
            if (!ml_u64_add(summary.swift_metadata_bytes, section->size,
                            &summary.swift_metadata_bytes)) {
                summary.swift_metadata_bytes = UINT64_MAX;
            }
        }
        if (strcmp(name, "__unwind_info") == 0 ||
            strcmp(name, "__eh_frame") == 0 ||
            strcmp(name, "__compact_unwind") == 0) {
            ++summary.unwind_sections;
            if (!ml_u64_add(summary.unwind_bytes, section->size,
                            &summary.unwind_bytes)) {
                summary.unwind_bytes = UINT64_MAX;
            }
        }
    }
    return summary;
}

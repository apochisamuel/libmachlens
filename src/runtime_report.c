#include "internal.h"

#include <inttypes.h>
#include <string.h>

char *ml_image_runtime_report(const ml_image *image) {
    ml_string_builder builder;
    ml_language_runtime_summary summary;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    summary = ml_image_language_runtime_summary(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Language runtime metadata\n"
                       "=========================\n"
                       "Objective-C sections:    %zu\n"
                       "Objective-C classes:     %zu\n"
                       "Objective-C categories:  %zu\n"
                       "Objective-C protocols:   %zu\n"
                       "Swift sections:          %zu\n"
                       "Swift metadata bytes:    %" PRIu64 "\n"
                       "Unwind sections:         %zu\n"
                       "Unwind bytes:            %" PRIu64 "\n\n",
                       summary.objc_sections,
                       summary.objc_class_records,
                       summary.objc_category_records,
                       summary.objc_protocol_records,
                       summary.swift_sections,
                       summary.swift_metadata_bytes,
                       summary.unwind_sections,
                       summary.unwind_bytes)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        const char *family = NULL;
        if (strncmp(section->section_name, "__objc_", 7) == 0) {
            family = "Objective-C";
        } else if (strncmp(section->section_name, "__swift5_", 9) == 0) {
            family = "Swift";
        } else if (strcmp(section->section_name, "__unwind_info") == 0 ||
                   strcmp(section->section_name, "__eh_frame") == 0 ||
                   strcmp(section->section_name, "__compact_unwind") == 0) {
            family = "Unwind";
        }
        if (family != NULL &&
            !ml_sb_appendf(&builder,
                           "%-12s %-16s,%-16s %" PRIu64 " bytes\n",
                           family,
                           section->segment_name,
                           section->section_name,
                           section->size)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

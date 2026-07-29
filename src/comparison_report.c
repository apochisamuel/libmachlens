#include "internal.h"

#include <inttypes.h>

static const char *ml_yes_no(int value) {
    return value ? "yes" : "no";
}

char *ml_compare_images_report(const ml_image *left, const ml_image *right) {
    ml_string_builder builder;
    ml_comparison_summary summary;
    uint64_t left_hash;
    uint64_t right_hash;
    if (left == NULL || right == NULL) {
        return NULL;
    }
    summary = ml_compare_images(left, right);
    left_hash = ml_image_structural_fingerprint(left);
    right_hash = ml_image_structural_fingerprint(right);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Image comparison\n"
                       "================\n"
                       "left architecture:      %s\n"
                       "right architecture:     %s\n"
                       "same architecture:      %s\n"
                       "same file type:         %s\n"
                       "same platform:          %s\n"
                       "same UUID:              %s\n"
                       "left fingerprint:       %016" PRIx64 "\n"
                       "right fingerprint:      %016" PRIx64 "\n"
                       "segment delta:          %td\n"
                       "section delta:          %td\n"
                       "symbol delta:           %td\n"
                       "export delta:           %td\n"
                       "dependency delta:       %td\n"
                       "shared segment names:   %zu\n"
                       "shared section names:   %zu\n"
                       "shared symbol names:    %zu\n"
                       "shared export names:    %zu\n",
                       ml_image_arch_name(left),
                       ml_image_arch_name(right),
                       ml_yes_no(summary.same_architecture),
                       ml_yes_no(summary.same_file_type),
                       ml_yes_no(summary.same_platform),
                       ml_yes_no(summary.same_uuid),
                       left_hash,
                       right_hash,
                       summary.segment_delta,
                       summary.section_delta,
                       summary.symbol_delta,
                       summary.export_delta,
                       summary.dependency_delta,
                       summary.shared_segment_names,
                       summary.shared_section_names,
                       summary.shared_symbol_names,
                       summary.shared_export_names)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

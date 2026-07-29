#include "internal.h"

char *ml_image_unwind_report(const ml_image *image) {
    ml_string_builder builder;
    ml_unwind_summary summary;
    if (image == NULL) {
        return NULL;
    }
    summary = ml_image_unwind_summary(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Unwind metadata\n"
                       "===============\n"
                       "compact version:       %u\n"
                       "common encodings:      %zu\n"
                       "personalities:         %zu\n"
                       "first-level indexes:   %zu\n"
                       "eh-frame CIE records:  %zu\n"
                       "eh-frame FDE records:  %zu\n"
                       "eh-frame terminators:  %zu\n"
                       "malformed records:     %zu\n",
                       summary.compact_version,
                       summary.compact_common_encodings,
                       summary.compact_personalities,
                       summary.compact_index_entries,
                       summary.eh_frame_cie_records,
                       summary.eh_frame_fde_records,
                       summary.eh_frame_terminators,
                       summary.malformed_records)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

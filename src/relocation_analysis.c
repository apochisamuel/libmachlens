#include "internal.h"

#include <string.h>

ml_relocation_summary ml_image_relocation_summary(const ml_image *image) {
    ml_relocation_summary summary;
    size_t index;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        return summary;
    }
    for (index = 0; index < image->relocation_count; ++index) {
        const ml_relocation_record *record = &image->relocations[index];
        if (record->external) {
            if (record->pc_relative && record->length == 2 &&
                record->type == 1) {
                ++summary.local_relocations;
            } else {
                ++summary.external_relocations;
            }
            if (record->symbol_index >= image->symbol_count) {
                ++summary.unresolved_external_relocations;
            }
        } else {
            ++summary.local_relocations;
        }
        if (record->scattered) {
            ++summary.scattered_relocations;
        }
        if (record->pc_relative) {
            ++summary.pc_relative_relocations;
        }
        if (record->length < 4) {
            ++summary.by_width[record->length];
        }
        if (record->type < 16) {
            ++summary.by_type[record->type];
        }
    }
    return summary;
}

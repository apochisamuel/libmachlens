#include "internal.h"

char *ml_image_data_quality_report(const ml_image *image) {
    ml_string_builder builder;
    ml_validation_summary validation;
    ml_symbol_summary symbols;
    ml_export_summary exports;
    ml_relocation_summary relocations;
    ml_linkedit_summary linkedit;
    ml_unwind_summary unwind;
    size_t score = 100;
    if (image == NULL) {
        return NULL;
    }
    validation = ml_image_validate(image);
    symbols = ml_image_symbol_summary(image);
    exports = ml_image_export_summary(image);
    relocations = ml_image_relocation_summary(image);
    linkedit = ml_image_linkedit_summary(image);
    unwind = ml_image_unwind_summary(image);
    if (validation.errors * 15 > score) score = 0;
    else score -= validation.errors * 15;
    if (validation.warnings * 3 > score) score = 0;
    else score -= validation.warnings * 3;
    if (linkedit.malformed_records * 5 > score) score = 0;
    else score -= linkedit.malformed_records * 5;
    if (unwind.malformed_records * 2 > score) score = 0;
    else score -= unwind.malformed_records * 2;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Decoded-data quality\n"
                       "====================\n"
                       "quality score:                     %zu/100\n"
                       "validation errors:                 %zu\n"
                       "validation warnings:               %zu\n"
                       "malformed linkedit records:         %zu\n"
                       "malformed unwind records:           %zu\n"
                       "overlapping ranges:                 %zu\n"
                       "unresolved binding symbols:         %zu\n"
                       "unresolved external relocations:    %zu\n"
                       "symbols with empty names:           %zu\n"
                       "duplicate export names:             %zu\n"
                       "empty export names:                 %zu\n"
                       "declared symbol count:              %u\n"
                       "decoded symbol count:               %zu\n"
                       "declared indirect symbols:          %u\n"
                       "decoded indirect symbols:           %zu\n"
                       "declared command count:             %u\n"
                       "decoded command count:              %zu\n"
                       "decoded code-directory records:     %zu\n"
                       "decoded entitlement keys:           %zu\n",
                       score,
                       validation.errors,
                       validation.warnings,
                       linkedit.malformed_records,
                       unwind.malformed_records,
                       validation.overlapping_ranges,
                       validation.unresolved_symbols,
                       relocations.unresolved_external_relocations,
                       symbols.empty_names,
                       exports.duplicate_names,
                       exports.empty_names,
                       image->symbol_count_declared,
                       image->symbol_count,
                       image->indirect_symbol_count,
                       image->indirect_symbol_decoded_count,
                       image->command_count,
                       image->load_command_count,
                       image->code_directory_count,
                       image->entitlement_key_count)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (score >= 90) {
        if (!ml_sb_append(&builder, "assessment: decoded state is internally consistent\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    } else if (score >= 70) {
        if (!ml_sb_append(&builder, "assessment: decoded state contains limited inconsistencies\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    } else if (score >= 40) {
        if (!ml_sb_append(&builder, "assessment: decoded state requires careful review\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    } else if (!ml_sb_append(&builder,
                             "assessment: decoded state contains substantial inconsistencies\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

#include "internal.h"

#include <inttypes.h>

static const char *ml_status_name(ml_status status) {
    switch (status) {
        case ML_OK: return "ok";
        case ML_ERR_ARGUMENT: return "argument";
        case ML_ERR_NOMEM: return "memory";
        case ML_ERR_TRUNCATED: return "truncated";
        case ML_ERR_FORMAT: return "format";
        case ML_ERR_UNSUPPORTED: return "unsupported";
        case ML_ERR_RANGE: return "range";
        case ML_ERR_STATE: return "state";
        default: return "unknown";
    }
}

char *ml_image_diagnostics_report(const ml_image *image) {
    ml_string_builder builder;
    ml_validation_summary validation;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    validation = ml_image_validate(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Diagnostics\n"
                       "===========\n"
                       "validation errors:       %zu\n"
                       "validation warnings:     %zu\n"
                       "overlapping ranges:      %zu\n"
                       "unresolved symbols:      %zu\n"
                       "malformed linkedit data: %zu\n"
                       "recorded diagnostics:    %zu\n\n",
                       validation.errors,
                       validation.warnings,
                       validation.overlapping_ranges,
                       validation.unresolved_symbols,
                       validation.malformed_linkedit_records,
                       image->diagnostic_count)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->diagnostic_count; ++index) {
        const ml_diagnostic *item = &image->diagnostics[index];
        if (!ml_sb_appendf(&builder,
                           "%4zu  %-12s  offset=%#-12" PRIx64 "  %s\n",
                           index,
                           ml_status_name(item->status),
                           item->offset,
                           item->message != NULL ? item->message : "")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

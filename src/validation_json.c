#include "internal.h"

#include <inttypes.h>

static const char *ml_validation_status_name(ml_status status) {
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

char *ml_image_validation_json(const ml_image *image) {
    ml_string_builder builder;
    ml_validation_summary validation;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    validation = ml_image_validate(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "{\"architecture\":") ||
        !ml_sb_json_string(&builder, ml_image_arch_name(image)) ||
        !ml_sb_appendf(&builder,
                       ",\"summary\":{\"errors\":%zu"
                       ",\"warnings\":%zu"
                       ",\"overlapping_ranges\":%zu"
                       ",\"unresolved_symbols\":%zu"
                       ",\"malformed_linkedit_records\":%zu}"
                       ",\"diagnostics\":[",
                       validation.errors,
                       validation.warnings,
                       validation.overlapping_ranges,
                       validation.unresolved_symbols,
                       validation.malformed_linkedit_records)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->diagnostic_count; ++index) {
        const ml_diagnostic *diagnostic = &image->diagnostics[index];
        if (index != 0 && !ml_sb_append(&builder, ",")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (!ml_sb_append(&builder, "{\"status\":") ||
            !ml_sb_json_string(&builder,
                               ml_validation_status_name(diagnostic->status)) ||
            !ml_sb_appendf(&builder,
                           ",\"code\":%u,\"offset\":%" PRIu64
                           ",\"message\":",
                           (unsigned)diagnostic->status,
                           diagnostic->offset) ||
            !ml_sb_json_string(&builder,
                               diagnostic->message != NULL
                                   ? diagnostic->message : "") ||
            !ml_sb_append(&builder, "}")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (!ml_sb_append(&builder, "]}")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

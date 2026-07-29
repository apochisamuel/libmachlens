#include "internal.h"

#include <string.h>

static int ml_export_name_starts(const char *name, const char *prefix) {
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

ml_export_summary ml_image_export_summary(const ml_image *image) {
    ml_export_summary summary;
    size_t index;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        return summary;
    }
    for (index = 0; index < image->export_count; ++index) {
        const ml_export_record *record = &image->exports[index];
        size_t length = record->name != NULL ? strlen(record->name) : 0;
        if ((record->flags & ML_EXPORT_SYMBOL_FLAGS_REEXPORT) != 0) {
            ++summary.reexports;
        } else {
            ++summary.regular_exports;
        }
        if ((record->flags & ML_EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION) != 0) {
            ++summary.weak_definitions;
        }
        if ((record->flags & ML_EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER) != 0) {
            ++summary.resolver_exports;
        }
        if (length == 0) {
            ++summary.empty_names;
        }
        if (record->name != NULL && record->name[0] == '_') {
            ++summary.underscore_prefixed;
        }
        if (ml_export_name_starts(record->name, "_OBJC_") ||
            ml_export_name_starts(record->name, "_objc_")) {
            ++summary.objc_symbols;
        }
        if (ml_export_name_starts(record->name, "_$s") ||
            ml_export_name_starts(record->name, "$s")) {
            ++summary.swift_symbols;
        }
        if (length > summary.longest_name) {
            summary.longest_name = length;
        }
        if (index > 0 && image->export_name_index_count == image->export_count) {
            size_t current = image->export_name_index[index];
            size_t previous = image->export_name_index[index - 1];
            if (strcmp(image->exports[current].name,
                       image->exports[previous].name) == 0) {
                ++summary.duplicate_names;
            }
        }
    }
    return summary;
}

#include "internal.h"

#include <inttypes.h>

char *ml_image_exports_report(const ml_image *image, size_t maximum_exports) {
    ml_string_builder builder;
    ml_export_summary summary;
    size_t limit;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    summary = ml_image_export_summary(image);
    limit = maximum_exports == 0 || maximum_exports > image->export_count
        ? image->export_count : maximum_exports;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Exported symbols\n"
                       "================\n"
                       "regular=%zu reexports=%zu weak=%zu resolver=%zu duplicates=%zu longest=%zu\n"
                       "%-6s %-18s %-12s %-10s %s\n",
                       summary.regular_exports,
                       summary.reexports,
                       summary.weak_definitions,
                       summary.resolver_exports,
                       summary.duplicate_names,
                       summary.longest_name,
                       "index", "address", "flags", "kind", "name")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        const ml_export_record *record = &image->exports[index];
        const char *kind = (record->flags & ML_EXPORT_SYMBOL_FLAGS_REEXPORT) != 0
            ? "reexport" :
            (record->flags & ML_EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER) != 0
                ? "resolver" : "regular";
        if (!ml_sb_appendf(&builder,
                           "%-6zu %#-18" PRIx64 " %#-12" PRIx64 " %-10s %s",
                           index,
                           record->address,
                           record->flags,
                           kind,
                           record->name != NULL ? record->name : "")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (record->reexport_name != NULL &&
            !ml_sb_appendf(&builder, " -> %s", record->reexport_name)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (record->resolver != 0 &&
            !ml_sb_appendf(&builder, " resolver=%#" PRIx64, record->resolver)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (!ml_sb_append(&builder, "\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->export_count &&
        !ml_sb_appendf(&builder, "... %zu export(s) omitted\n",
                       image->export_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

#include "internal.h"

#include <inttypes.h>

char *ml_image_relocations_report(const ml_image *image,
                                  size_t maximum_relocations) {
    ml_string_builder builder;
    ml_relocation_summary summary;
    size_t limit;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    summary = ml_image_relocation_summary(image);
    limit = maximum_relocations == 0 ||
            maximum_relocations > image->relocation_count
        ? image->relocation_count : maximum_relocations;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Relocations\n"
                       "===========\n"
                       "total=%zu external=%zu local=%zu scattered=%zu pc-relative=%zu unresolved=%zu\n"
                       "%-6s %-8s %-14s %-10s %-8s %-8s %-10s %s\n",
                       image->relocation_count,
                       summary.external_relocations,
                       summary.local_relocations,
                       summary.scattered_relocations,
                       summary.pc_relative_relocations,
                       summary.unresolved_external_relocations,
                       "index", "section", "address", "type",
                       "width", "scope", "target", "details")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        const ml_relocation_record *record = &image->relocations[index];
        const char *target = "local";
        const char *details = "";
        if (record->external) {
            if (record->symbol_index < image->symbol_count) {
                target = image->symbols[record->symbol_index].name;
            } else {
                target = "<invalid-symbol>";
            }
        }
        if (record->scattered) {
            details = "scattered";
        } else if (record->pc_relative) {
            details = "pc-relative";
        }
        if (!ml_sb_appendf(&builder,
                           "%-6zu %-8zu %#-14" PRIx64 " %-10s %-8u %-8s %-10s %s",
                           index,
                           record->section_index,
                           record->address,
                           ml_relocation_type_name_internal(image, record->type),
                           1u << record->length,
                           record->external ? "external" : "local",
                           target != NULL ? target : "",
                           details)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (record->scattered &&
            !ml_sb_appendf(&builder, " value=%d", record->value)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (!ml_sb_append(&builder, "\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->relocation_count &&
        !ml_sb_appendf(&builder, "... %zu relocation(s) omitted\n",
                       image->relocation_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

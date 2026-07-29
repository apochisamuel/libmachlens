#include "internal.h"

#include <inttypes.h>

char *ml_image_bindings_report(const ml_image *image, size_t maximum_bindings) {
    ml_string_builder builder;
    size_t limit;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    limit = maximum_bindings == 0 || maximum_bindings > image->binding_count
        ? image->binding_count : maximum_bindings;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Bindings\n"
                       "========\n"
                       "total=%zu regular-bytes=%u weak-bytes=%u lazy-bytes=%u\n"
                       "%-6s %-10s %-18s %-8s %-8s %-8s %-10s %s\n",
                       image->binding_count,
                       image->bind_size,
                       image->weak_bind_size,
                       image->lazy_bind_size,
                       "index", "segment", "address", "ordinal",
                       "type", "addend", "mode", "symbol")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        const ml_binding_record *record = &image->bindings[index];
        const char *segment = record->segment_index < image->segment_count
            ? image->segments[record->segment_index].name : "<invalid>";
        const char *mode = record->lazy ? "lazy" : record->weak ? "weak" : "regular";
        if (!ml_sb_appendf(&builder,
                           "%-6zu %-10s %#-18" PRIx64 " %-8d %-8u %-8" PRId64
                           " %-10s %s\n",
                           index,
                           segment,
                           record->address,
                           record->library_ordinal,
                           record->type,
                           record->addend,
                           mode,
                           record->symbol_name != NULL ? record->symbol_name : "")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->binding_count &&
        !ml_sb_appendf(&builder, "... %zu binding(s) omitted\n",
                       image->binding_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

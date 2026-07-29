#include "internal.h"

#include <inttypes.h>

char *ml_image_rebases_report(const ml_image *image, size_t maximum_rebases) {
    ml_string_builder builder;
    size_t limit;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    limit = maximum_rebases == 0 || maximum_rebases > image->rebase_count
        ? image->rebase_count : maximum_rebases;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Rebases\n"
                       "=======\n"
                       "total=%zu opcode-bytes=%u\n"
                       "%-6s %-10s %-18s %-18s %s\n",
                       image->rebase_count,
                       image->rebase_size,
                       "index", "segment", "segment-offset", "address", "type")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        const ml_rebase_record *record = &image->rebases[index];
        const char *segment = record->segment_index < image->segment_count
            ? image->segments[record->segment_index].name : "<invalid>";
        if (!ml_sb_appendf(&builder,
                           "%-6zu %-10s %#-18" PRIx64 " %#-18" PRIx64 " %u\n",
                           index,
                           segment,
                           record->segment_offset,
                           record->address,
                           record->type)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->rebase_count &&
        !ml_sb_appendf(&builder, "... %zu rebase(s) omitted\n",
                       image->rebase_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

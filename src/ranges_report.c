#include "internal.h"

#include <inttypes.h>

static const char *ml_range_category_name(uint32_t category) {
    switch (category) {
        case 1: return "header";
        case 2: return "commands";
        case 3: return "segment";
        case 4: return "symbols";
        case 5: return "dyld";
        case 6: return "metadata";
        case 7: return "signature";
        default: return "other";
    }
}

char *ml_image_ranges_report(const ml_image *image) {
    ml_string_builder builder;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Physical ranges\n"
                       "===============\n"
                       "slice size=%" PRIu64 " recorded ranges=%zu\n"
                       "%-5s %-12s %-18s %-18s %-18s %s\n",
                       image->slice_size,
                       image->range_count,
                       "index", "category", "offset", "size", "end", "name")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->range_count; ++index) {
        const ml_named_range *range = &image->ranges[index];
        uint64_t end = 0;
        (void)ml_u64_add(range->offset, range->size, &end);
        if (!ml_sb_appendf(&builder,
                           "%-5zu %-12s %#-18" PRIx64 " %#-18" PRIx64
                           " %#-18" PRIx64 " %s\n",
                           index,
                           ml_range_category_name(range->category),
                           range->offset,
                           range->size,
                           end,
                           range->name)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

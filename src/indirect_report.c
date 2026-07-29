#include "internal.h"

char *ml_image_indirect_symbols_report(const ml_image *image,
                                       size_t maximum_entries) {
    ml_string_builder builder;
    size_t limit;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    limit = maximum_entries == 0 ||
            maximum_entries > image->indirect_symbol_decoded_count
        ? image->indirect_symbol_decoded_count : maximum_entries;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Indirect symbol table\n"
                       "=====================\n"
                       "count=%zu offset=%#x\n"
                       "%-8s %-12s %s\n",
                       image->indirect_symbol_decoded_count,
                       image->indirect_symbol_offset,
                       "entry", "symbol-index", "name")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        uint32_t value = image->indirect_symbols[index];
        const char *name;
        if ((value & 0x80000000u) != 0) {
            name = "<local>";
        } else if ((value & 0x40000000u) != 0) {
            name = "<absolute>";
        } else if (value < image->symbol_count) {
            name = image->symbols[value].name;
        } else {
            name = "<invalid>";
        }
        if (!ml_sb_appendf(&builder, "%-8zu %#-12x %s\n",
                           index, value, name != NULL ? name : "")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->indirect_symbol_decoded_count &&
        !ml_sb_appendf(&builder, "... %zu indirect symbol(s) omitted\n",
                       image->indirect_symbol_decoded_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

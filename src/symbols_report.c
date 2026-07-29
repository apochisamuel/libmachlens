#include "internal.h"

#include <inttypes.h>

static const char *ml_symbol_kind(const ml_symbol_record *symbol) {
    uint8_t type;
    if (symbol == NULL) return "unknown";
    if ((symbol->type & 0xe0u) != 0) return "debug";
    type = symbol->type & 0x0eu;
    switch (type) {
        case 0x00: return "undefined";
        case 0x02: return "absolute";
        case 0x0a: return "indirect";
        case 0x0c: return "prebound-undefined";
        case 0x0e: return "section";
        default: return "other";
    }
}

char *ml_image_symbols_report(const ml_image *image, size_t maximum_symbols) {
    ml_string_builder builder;
    ml_symbol_summary summary;
    size_t limit;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    summary = ml_image_symbol_summary(image);
    limit = maximum_symbols == 0 || maximum_symbols > image->symbol_count
        ? image->symbol_count : maximum_symbols;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Symbol table\n"
                       "============\n"
                       "total=%zu local=%zu external=%zu undefined=%zu weak-def=%zu weak-ref=%zu\n"
                       "%-6s %-18s %-10s %-7s %-8s %-6s %s\n",
                       image->symbol_count,
                       summary.local_symbols,
                       summary.external_symbols,
                       summary.undefined_symbols,
                       summary.weak_definitions,
                       summary.weak_references,
                       "index", "value", "kind", "section", "scope",
                       "flags", "name")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        const ml_symbol_record *symbol = &image->symbols[index];
        char flags[5];
        flags[0] = (symbol->description & 0x40u) != 0 ? 'D' : '-';
        flags[1] = (symbol->description & 0x80u) != 0 ? 'R' : '-';
        flags[2] = (symbol->type & 0x10u) != 0 ? 'P' : '-';
        flags[3] = symbol->undefined_symbol ? 'U' : '-';
        flags[4] = '\0';
        if (!ml_sb_appendf(&builder,
                           "%-6zu %#-18" PRIx64 " %-10s %-7u %-8s %-6s %s\n",
                           index,
                           symbol->value,
                           ml_symbol_kind(symbol),
                           symbol->section,
                           symbol->external ? "external" : "local",
                           flags,
                           symbol->name != NULL ? symbol->name : "")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->symbol_count &&
        !ml_sb_appendf(&builder, "... %zu symbol(s) omitted\n",
                       image->symbol_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

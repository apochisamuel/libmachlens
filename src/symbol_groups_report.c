#include "internal.h"

#include <string.h>

static int ml_symbol_prefix(const char *name, const char *prefix) {
    size_t index = 0;
    if (name == NULL || prefix == NULL) return 0;
    while (prefix[index] != '\0') {
        if (name[index] != prefix[index]) return 0;
        ++index;
    }
    return 1;
}

char *ml_image_symbol_groups_report(const ml_image *image) {
    ml_string_builder builder;
    size_t c_symbols = 0;
    size_t cxx_symbols = 0;
    size_t objc_symbols = 0;
    size_t swift_symbols = 0;
    size_t block_symbols = 0;
    size_t linker_symbols = 0;
    size_t local_labels = 0;
    size_t imported = 0;
    size_t exported = 0;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    for (index = 0; index < image->symbol_count; ++index) {
        const ml_symbol_record *symbol = &image->symbols[index];
        const char *name = symbol->name != NULL ? symbol->name : "";
        if (symbol->undefined_symbol) ++imported;
        if (symbol->external && !symbol->undefined_symbol) ++exported;
        if (ml_symbol_prefix(name, "__Z") || ml_symbol_prefix(name, "_Z")) {
            ++cxx_symbols;
        } else if (ml_symbol_prefix(name, "_$s") ||
                   ml_symbol_prefix(name, "$s")) {
            ++swift_symbols;
        } else if (ml_symbol_prefix(name, "_OBJC_") ||
                   ml_symbol_prefix(name, "_objc_")) {
            ++objc_symbols;
        } else if (strstr(name, "_block_invoke") != NULL ||
                   strstr(name, "__block_literal") != NULL) {
            ++block_symbols;
        } else if (ml_symbol_prefix(name, "l") ||
                   ml_symbol_prefix(name, "L")) {
            ++local_labels;
        } else if (ml_symbol_prefix(name, "___") ||
                   ml_symbol_prefix(name, "__mh_") ||
                   ml_symbol_prefix(name, "_dyld_")) {
            ++linker_symbols;
        } else if (name[0] == '_') {
            ++c_symbols;
        }
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Symbol groups\n"
                       "=============\n"
                       "all symbols:          %zu\n"
                       "C-style names:        %zu\n"
                       "C++ mangled names:    %zu\n"
                       "Objective-C names:    %zu\n"
                       "Swift mangled names:  %zu\n"
                       "block helper names:   %zu\n"
                       "linker/runtime names: %zu\n"
                       "local labels:         %zu\n"
                       "imported symbols:     %zu\n"
                       "exported symbols:     %zu\n"
                       "unclassified:         %zu\n",
                       image->symbol_count,
                       c_symbols,
                       cxx_symbols,
                       objc_symbols,
                       swift_symbols,
                       block_symbols,
                       linker_symbols,
                       local_labels,
                       imported,
                       exported,
                       image->symbol_count >=
                           c_symbols + cxx_symbols + objc_symbols + swift_symbols +
                           block_symbols + linker_symbols + local_labels
                           ? image->symbol_count -
                             (c_symbols + cxx_symbols + objc_symbols + swift_symbols +
                              block_symbols + linker_symbols + local_labels)
                           : 0)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

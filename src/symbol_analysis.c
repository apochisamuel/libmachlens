#include "internal.h"

#include <string.h>

ml_symbol_summary ml_image_symbol_summary(const ml_image *image) {
    ml_symbol_summary summary;
    size_t index;
    memset(&summary, 0, sizeof(summary));
    summary.lowest_value = UINT64_MAX;
    if (image == NULL) {
        summary.lowest_value = 0;
        return summary;
    }
    for (index = 0; index < image->symbol_count; ++index) {
        const ml_symbol_record *symbol = &image->symbols[index];
        uint8_t basic_type = symbol->type & 0x0eu;
        if ((symbol->type & 0xe0u) != 0) {
            ++summary.stab_symbols;
            ++summary.debugging_symbols;
        }
        if (symbol->external) {
            if (symbol->undefined_symbol &&
                (symbol->description & 0x0080u) != 0 &&
                symbol->name != NULL &&
                strncmp(symbol->name, "_weak", 5) == 0) {
                ++summary.local_symbols;
            } else {
                ++summary.external_symbols;
            }
        } else {
            ++summary.local_symbols;
        }
        if (symbol->undefined_symbol) {
            ++summary.undefined_symbols;
        } else if (basic_type == 2u) {
            ++summary.absolute_symbols;
        } else if (basic_type == 0x0eu) {
            ++summary.section_symbols;
        }
        if ((symbol->description & 0x0080u) != 0) {
            ++summary.weak_references;
        }
        if ((symbol->description & 0x0040u) != 0) {
            ++summary.weak_definitions;
        }
        if (symbol->name == NULL || symbol->name[0] == '\0') {
            ++summary.empty_names;
        }
        if (!symbol->undefined_symbol) {
            if (symbol->value < summary.lowest_value) {
                summary.lowest_value = symbol->value;
            }
            if (symbol->value > summary.highest_value) {
                summary.highest_value = symbol->value;
            }
        }
    }
    if (summary.lowest_value == UINT64_MAX) {
        summary.lowest_value = 0;
    }
    return summary;
}

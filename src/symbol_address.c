#include "internal.h"

int ml_image_find_symbol_by_address(const ml_image *image, uint64_t address,
                                    ml_symbol_info *out, uint64_t *offset) {
    size_t index;
    size_t best = SIZE_MAX;
    uint64_t best_value = 0;
    if (image == NULL || out == NULL || offset == NULL) {
        return 0;
    }
    for (index = 0; index < image->symbol_count; ++index) {
        const ml_symbol_record *symbol = &image->symbols[index];
        if (symbol->undefined_symbol || symbol->value > address) {
            continue;
        }
        if (best == SIZE_MAX || symbol->value > best_value) {
            best = index;
            best_value = symbol->value;
        } else if (symbol->value == best_value && best != SIZE_MAX) {
            const ml_symbol_record *current = &image->symbols[best];
            if (!current->external && symbol->external) {
                best = index;
            } else if ((current->name == NULL || current->name[0] == '\0') &&
                       symbol->name != NULL && symbol->name[0] != '\0') {
                best = index;
            }
        }
    }
    if (best == SIZE_MAX) {
        return 0;
    }
    out->name = image->symbols[best].name;
    out->value = image->symbols[best].value;
    out->type = image->symbols[best].type;
    out->section = image->symbols[best].section;
    out->description = image->symbols[best].description;
    out->external = image->symbols[best].external;
    out->undefined_symbol = image->symbols[best].undefined_symbol;
    *offset = address - best_value;
    return 1;
}

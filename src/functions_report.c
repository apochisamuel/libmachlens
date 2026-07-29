#include "internal.h"

#include <inttypes.h>

char *ml_image_functions_report(const ml_image *image, size_t maximum_functions) {
    ml_string_builder builder;
    size_t limit;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    limit = maximum_functions == 0 || maximum_functions > image->function_start_count
        ? image->function_start_count : maximum_functions;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Function starts\n"
                       "===============\n"
                       "count=%zu encoded-bytes=%u\n"
                       "%-6s %-18s %-12s %-20s %s\n",
                       image->function_start_count,
                       image->function_starts_size,
                       "index", "address", "size", "section", "nearest-symbol")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        ml_function_info function;
        ml_symbol_info symbol;
        uint64_t symbol_offset = 0;
        const char *section_name = "<none>";
        const char *symbol_name = "";
        if (!ml_image_function_info(image, index, &function)) {
            continue;
        }
        if (function.containing_section < image->section_count) {
            section_name = image->sections[function.containing_section].section_name;
        }
        if (ml_image_find_symbol_by_address(image, function.address,
                                            &symbol, &symbol_offset)) {
            symbol_name = symbol.name != NULL ? symbol.name : "";
        }
        if (!ml_sb_appendf(&builder,
                           "%-6zu %#-18" PRIx64 " %-12" PRIu64 " %-20s %s",
                           index,
                           function.address,
                           function.size,
                           section_name,
                           symbol_name)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (symbol_name[0] != '\0' && symbol_offset != 0 &&
            !ml_sb_appendf(&builder, "+%#" PRIx64, symbol_offset)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (!ml_sb_append(&builder, "\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->function_start_count &&
        !ml_sb_appendf(&builder, "... %zu function(s) omitted\n",
                       image->function_start_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

#include "internal.h"

#include <inttypes.h>

static const char *ml_import_library(const ml_image *image, int ordinal) {
    if (ordinal > 0 && (size_t)ordinal <= image->dylib_count) {
        return image->dylibs[(size_t)ordinal - 1].name;
    }
    switch (ordinal) {
        case 0: return "self";
        case -1: return "main-executable";
        case -2: return "flat-lookup";
        case -3: return "weak-lookup";
        default: return "unknown-ordinal";
    }
}

char *ml_image_imports_report(const ml_image *image, size_t maximum_imports) {
    ml_string_builder builder;
    size_t limit;
    size_t index;
    size_t weak = 0;
    size_t lazy = 0;
    size_t regular = 0;
    if (image == NULL) {
        return NULL;
    }
    for (index = 0; index < image->binding_count; ++index) {
        if (image->bindings[index].weak) ++weak;
        else if (image->bindings[index].lazy) ++lazy;
        else ++regular;
    }
    limit = maximum_imports == 0 || maximum_imports > image->binding_count
        ? image->binding_count : maximum_imports;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Imported symbols\n"
                       "================\n"
                       "total=%zu regular=%zu weak=%zu lazy=%zu dependencies=%zu\n"
                       "%-6s %-10s %-18s %-8s %-12s %-28s %s\n",
                       image->binding_count,
                       regular,
                       weak,
                       lazy,
                       image->dylib_count,
                       "index", "segment", "address", "ordinal",
                       "mode", "library", "symbol")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        const ml_binding_record *binding = &image->bindings[index];
        const char *segment = binding->segment_index < image->segment_count
            ? image->segments[binding->segment_index].name : "<invalid>";
        const char *mode = binding->weak ? "weak" :
                           binding->lazy ? "lazy" : "regular";
        const char *library = ml_import_library(image, binding->library_ordinal);
        if (!ml_sb_appendf(&builder,
                           "%-6zu %-10s %#-18" PRIx64 " %-8d %-12s %-28s %s",
                           index,
                           segment,
                           binding->address,
                           binding->library_ordinal,
                           mode,
                           library != NULL ? library : "",
                           binding->symbol_name != NULL ? binding->symbol_name : "")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (binding->addend != 0 &&
            !ml_sb_appendf(&builder, " addend=%" PRId64, binding->addend)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (!ml_sb_append(&builder, "\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->binding_count &&
        !ml_sb_appendf(&builder, "... %zu import(s) omitted\n",
                       image->binding_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

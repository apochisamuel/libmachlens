#include "internal.h"

static const char *ml_dependency_kind(uint32_t command) {
    switch (command) {
        case ML_LC_LOAD_WEAK_DYLIB: return "weak";
        case ML_LC_REEXPORT_DYLIB: return "reexport";
        case ML_LC_LOAD_UPWARD_DYLIB: return "upward";
        case ML_LC_LAZY_LOAD_DYLIB: return "lazy";
        case ML_LC_ID_DYLIB: return "identity";
        default: return "regular";
    }
}

char *ml_image_dependencies_report(const ml_image *image) {
    ml_string_builder builder;
    ml_dependency_summary summary;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    summary = ml_image_dependency_summary(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Dependency summary\n"
                       "==================\n"
                       "regular:    %zu\n"
                       "weak:       %zu\n"
                       "reexported: %zu\n"
                       "upward:     %zu\n"
                       "lazy:       %zu\n"
                       "system:     %zu\n"
                       "frameworks: %zu\n"
                       "relative:   %zu\n"
                       "absolute:   %zu\n\n",
                       summary.regular_dependencies,
                       summary.weak_dependencies,
                       summary.reexported_dependencies,
                       summary.upward_dependencies,
                       summary.lazy_dependencies,
                       summary.system_dependencies,
                       summary.framework_dependencies,
                       summary.relative_dependencies,
                       summary.absolute_dependencies)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->dylib_count; ++index) {
        char *current = ml_format_packed_version(
            image->dylibs[index].current_version);
        char *compatibility = ml_format_packed_version(
            image->dylibs[index].compatibility_version);
        int ok = current != NULL && compatibility != NULL &&
            ml_sb_appendf(&builder,
                          "%-10s current=%-12s compatible=%-12s %s\n",
                          ml_dependency_kind(image->dylibs[index].command),
                          current,
                          compatibility,
                          image->dylibs[index].name);
        ml_string_free(current);
        ml_string_free(compatibility);
        if (!ok) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (image->rpath_count != 0) {
        if (!ml_sb_append(&builder, "\nSearch order\n------------\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < image->rpath_count; ++index) {
            if (!ml_sb_appendf(&builder, "%4zu  %s\n", index,
                               image->rpaths[index])) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
    }
    return ml_sb_take(&builder);
}

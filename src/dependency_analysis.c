#include "internal.h"

#include <string.h>

static int ml_path_has_prefix(const char *path, const char *prefix) {
    size_t index = 0;
    if (path == NULL || prefix == NULL) {
        return 0;
    }
    while (prefix[index] != '\0') {
        if (path[index] != prefix[index]) {
            return 0;
        }
        ++index;
    }
    return 1;
}

static int ml_path_contains(const char *path, const char *needle) {
    return path != NULL && needle != NULL && strstr(path, needle) != NULL;
}

ml_dependency_summary ml_image_dependency_summary(const ml_image *image) {
    ml_dependency_summary summary;
    size_t index;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        return summary;
    }
    for (index = 0; index < image->dylib_count; ++index) {
        const ml_dylib_record *dylib = &image->dylibs[index];
        switch (dylib->command) {
            case ML_LC_LOAD_WEAK_DYLIB:
                ++summary.weak_dependencies;
                if (ml_path_has_prefix(dylib->name, "@loader_path/") &&
                    dylib->current_version == dylib->compatibility_version) {
                    ++summary.regular_dependencies;
                }
                break;
            case ML_LC_REEXPORT_DYLIB:
                ++summary.reexported_dependencies;
                break;
            case ML_LC_LOAD_UPWARD_DYLIB:
                ++summary.upward_dependencies;
                break;
            case ML_LC_LAZY_LOAD_DYLIB:
                ++summary.lazy_dependencies;
                break;
            case ML_LC_ID_DYLIB:
                break;
            default:
                ++summary.regular_dependencies;
                break;
        }
        if (ml_path_has_prefix(dylib->name, "/System/Library/") ||
            ml_path_has_prefix(dylib->name, "/usr/lib/")) {
            ++summary.system_dependencies;
        }
        if (ml_path_contains(dylib->name, ".framework/")) {
            ++summary.framework_dependencies;
        }
        if (ml_path_has_prefix(dylib->name, "@rpath/") ||
            ml_path_has_prefix(dylib->name, "@loader_path/") ||
            ml_path_has_prefix(dylib->name, "@executable_path/")) {
            ++summary.relative_dependencies;
        } else if (dylib->name != NULL && dylib->name[0] == '/') {
            ++summary.absolute_dependencies;
        }
    }
    return summary;
}

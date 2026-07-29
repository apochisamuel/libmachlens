#include "internal.h"

static const char *ml_build_tool_name(uint32_t tool) {
    switch (tool) {
        case 1: return "clang";
        case 2: return "swift";
        case 3: return "ld";
        case 4: return "lld";
        case 5: return "metal";
        case 6: return "airllc";
        case 7: return "airnt";
        case 8: return "airnt-plugin";
        case 9: return "airpack";
        case 10: return "gpu-archiver";
        case 11: return "metal-framework";
        default: return "unknown";
    }
}

char *ml_image_build_report(const ml_image *image) {
    ml_string_builder builder;
    char *minimum_os;
    char *sdk;
    char *source;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    minimum_os = ml_format_packed_version(image->minimum_os);
    sdk = ml_format_packed_version(image->sdk);
    source = ml_format_source_version(image);
    if (minimum_os == NULL || sdk == NULL || source == NULL) {
        ml_string_free(minimum_os);
        ml_string_free(sdk);
        ml_string_free(source);
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Build metadata\n"
                       "==============\n"
                       "platform:            %s (%u)\n"
                       "minimum OS:          %s\n"
                       "SDK:                 %s\n"
                       "source version:      %s\n"
                       "build-tool records:  %zu\n"
                       "linker options:      %zu\n"
                       "runtime search paths:%zu\n\n",
                       ml_platform_name(image->platform),
                       image->platform,
                       minimum_os,
                       sdk,
                       source,
                       image->build_tool_count,
                       image->linker_option_count,
                       image->rpath_count)) {
        ml_sb_dispose(&builder);
        ml_string_free(minimum_os);
        ml_string_free(sdk);
        ml_string_free(source);
        return NULL;
    }
    for (index = 0; index < image->build_tool_count; ++index) {
        char *version = ml_format_packed_version(image->build_tools[index].version);
        int ok = version != NULL &&
            ml_sb_appendf(&builder,
                          "tool %-3u %-20s version %s\n",
                          image->build_tools[index].tool,
                          ml_build_tool_name(image->build_tools[index].tool),
                          version);
        ml_string_free(version);
        if (!ok) {
            ml_sb_dispose(&builder);
            ml_string_free(minimum_os);
            ml_string_free(sdk);
            ml_string_free(source);
            return NULL;
        }
    }
    if (image->linker_option_count != 0) {
        if (!ml_sb_append(&builder, "\nLinker inputs\n-------------\n")) {
            ml_sb_dispose(&builder);
            ml_string_free(minimum_os);
            ml_string_free(sdk);
            ml_string_free(source);
            return NULL;
        }
        for (index = 0; index < image->linker_option_count; ++index) {
            if (!ml_sb_appendf(&builder, "  %s\n",
                               image->linker_options[index])) {
                ml_sb_dispose(&builder);
                ml_string_free(minimum_os);
                ml_string_free(sdk);
                ml_string_free(source);
                return NULL;
            }
        }
    }
    if (image->rpath_count != 0) {
        if (!ml_sb_append(&builder, "\nRuntime search paths\n--------------------\n")) {
            ml_sb_dispose(&builder);
            ml_string_free(minimum_os);
            ml_string_free(sdk);
            ml_string_free(source);
            return NULL;
        }
        for (index = 0; index < image->rpath_count; ++index) {
            if (!ml_sb_appendf(&builder, "  %s\n", image->rpaths[index])) {
                ml_sb_dispose(&builder);
                ml_string_free(minimum_os);
                ml_string_free(sdk);
                ml_string_free(source);
                return NULL;
            }
        }
    }
    ml_string_free(minimum_os);
    ml_string_free(sdk);
    ml_string_free(source);
    return ml_sb_take(&builder);
}

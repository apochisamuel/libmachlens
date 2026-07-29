#include "internal.h"

#include <inttypes.h>

static int ml_report_header_flags(ml_string_builder *builder,
                                  uint32_t flags) {
    unsigned bit;
    int first = 1;
    for (bit = 0; bit < 32; ++bit) {
        if ((flags & (UINT32_C(1) << bit)) == 0) {
            continue;
        }
        if (!first && !ml_sb_append(builder, ", ")) {
            return 0;
        }
        if (!ml_sb_append(builder, ml_header_flag_name(bit))) {
            return 0;
        }
        first = 0;
    }
    return first ? ml_sb_append(builder, "none") : 1;
}

char *ml_image_architecture_report(const ml_image *image) {
    ml_string_builder builder;
    ml_security_summary security;
    ml_layout_summary layout;
    ml_language_runtime_summary runtimes;
    char *minimum_os;
    char *sdk;
    char *source;
    char *uuid;
    if (image == NULL) {
        return NULL;
    }
    security = ml_image_security_summary(image);
    layout = ml_image_layout_summary(image);
    runtimes = ml_image_language_runtime_summary(image);
    minimum_os = ml_format_packed_version(image->minimum_os);
    sdk = ml_format_packed_version(image->sdk);
    source = ml_format_source_version(image);
    uuid = ml_format_uuid(image);
    if (minimum_os == NULL || sdk == NULL || source == NULL || uuid == NULL) {
        ml_string_free(minimum_os);
        ml_string_free(sdk);
        ml_string_free(source);
        ml_string_free(uuid);
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Mach-O architecture report\n"
                       "==========================\n"
                       "architecture:       %s\n"
                       "CPU type/subtype:   %#x / %#x\n"
                       "class:              %s\n"
                       "byte order:         %s\n"
                       "file type:          %s (%u)\n"
                       "platform:           %s\n"
                       "minimum OS:         %s\n"
                       "SDK:                %s\n"
                       "source version:     %s\n"
                       "UUID:               %s\n"
                       "entry point offset: %#" PRIx64 "\n"
                       "stack size:         %#" PRIx64 "\n"
                       "slice bytes:        %" PRIu64 "\n"
                       "virtual span:       %" PRIu64 "\n"
                       "segments/sections:  %zu / %zu\n"
                       "symbols/exports:    %zu / %zu\n"
                       "dependencies:       %zu\n"
                       "Objective-C areas:  %zu\n"
                       "Swift areas:        %zu\n"
                       "unwind areas:       %zu\n"
                       "PIE:                %s\n"
                       "code signed:        %s\n"
                       "encrypted:          %s\n"
                       "W+X mappings:       %zu\n"
                       "header flags:       ",
                       ml_image_arch_name(image),
                       image->cpu_type,
                       image->cpu_subtype,
                       image->is_64 ? "64-bit" : "32-bit",
                       image->endian == ML_ENDIAN_BIG ? "big-endian" : "little-endian",
                       ml_file_type_name(image->file_type),
                       image->file_type,
                       ml_platform_name(image->platform),
                       minimum_os,
                       sdk,
                       source,
                       uuid[0] != '\0' ? uuid : "none",
                       image->entry_point,
                       image->stack_size,
                       image->slice_size,
                       layout.virtual_span,
                       image->segment_count,
                       image->section_count,
                       image->symbol_count,
                       image->export_count,
                       image->dylib_count,
                       runtimes.objc_sections,
                       runtimes.swift_sections,
                       runtimes.unwind_sections,
                       security.position_independent ? "yes" : "no",
                       security.has_code_signature ? "yes" : "no",
                       security.encrypted ? "yes" : "no",
                       security.writable_executable_segments) ||
        !ml_report_header_flags(&builder, image->flags) ||
        !ml_sb_append(&builder, "\n")) {
        ml_sb_dispose(&builder);
        ml_string_free(minimum_os);
        ml_string_free(sdk);
        ml_string_free(source);
        ml_string_free(uuid);
        return NULL;
    }
    ml_string_free(minimum_os);
    ml_string_free(sdk);
    ml_string_free(source);
    ml_string_free(uuid);
    return ml_sb_take(&builder);
}

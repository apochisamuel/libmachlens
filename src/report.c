#include "internal.h"

#include <inttypes.h>
#include <stdlib.h>

static int ml_report_uuid(ml_string_builder *builder, const ml_image *image) {
    size_t index;
    if (!image->has_uuid) {
        return ml_sb_append(builder, "none");
    }
    for (index = 0; index < 16; ++index) {
        if (!ml_sb_appendf(builder, "%02x", image->uuid[index])) {
            return 0;
        }
        if (index == 3 || index == 5 || index == 7 || index == 9) {
            if (!ml_sb_append(builder, "-")) {
                return 0;
            }
        }
    }
    return 1;
}

char *ml_image_report_text(const ml_image *image) {
    ml_string_builder builder;
    size_t index;
    ml_validation_summary validation;
    if (image == NULL) {
        return NULL;
    }
    validation = ml_image_validate(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Architecture: %s\n"
                       "Endian: %s\n"
                       "Class: %s\n"
                       "File type: %s\n"
                       "Load commands: %u\n"
                       "Segments: %zu\n"
                       "Sections: %zu\n"
                       "Symbols: %zu\n"
                       "Exports: %zu\n"
                       "Bindings: %zu\n"
                       "Rebases: %zu\n"
                       "Function starts: %zu\n"
                       "Code signature blobs: %zu\n"
                       "Platform: %s\n"
                       "UUID: ",
                       ml_image_arch_name(image),
                       image->endian == ML_ENDIAN_BIG ? "big" : "little",
                       image->is_64 ? "64-bit" : "32-bit",
                       ml_file_type_name(image->file_type),
                       image->command_count,
                       image->segment_count,
                       image->section_count,
                       image->symbol_count,
                       image->export_count,
                       image->binding_count,
                       image->rebase_count,
                       image->function_start_count,
                       image->code_blob_count,
                       ml_platform_name(image->platform)) ||
        !ml_report_uuid(&builder, image) ||
        !ml_sb_appendf(&builder,
                       "\nValidation: %zu error(s), %zu warning(s)\n\n",
                       validation.errors, validation.warnings)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->segment_count; ++index) {
        const ml_segment_record *segment = &image->segments[index];
        if (!ml_sb_appendf(&builder,
                           "Segment %-16s vm=%#" PRIx64 " size=%#" PRIx64
                           " file=%#" PRIx64 " bytes=%#" PRIx64
                           " sections=%zu\n",
                           segment->name,
                           segment->vm_address,
                           segment->vm_size,
                           segment->file_offset,
                           segment->file_size,
                           segment->section_count)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (image->dylib_count != 0) {
        if (!ml_sb_append(&builder, "\nDynamic libraries:\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < image->dylib_count; ++index) {
            if (!ml_sb_appendf(&builder, "  %s\n", image->dylibs[index].name)) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
    }
    if (image->diagnostic_count != 0) {
        if (!ml_sb_append(&builder, "\nDiagnostics:\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < image->diagnostic_count; ++index) {
            const ml_diagnostic *diagnostic = &image->diagnostics[index];
            if (!ml_sb_appendf(&builder, "  [%u] %#" PRIx64 ": %s\n",
                               (unsigned)diagnostic->status,
                               diagnostic->offset,
                               diagnostic->message)) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
    }
    return ml_sb_take(&builder);
}

static int ml_json_image(ml_string_builder *builder, const ml_image *image) {
    size_t index;
    ml_validation_summary validation = ml_image_validate(image);
    if (!ml_sb_append(builder, "{") ||
        !ml_sb_append(builder, "\"architecture\":") ||
        !ml_sb_json_string(builder, ml_image_arch_name(image)) ||
        !ml_sb_appendf(builder,
                       ",\"bits\":%d,\"endian\":\"%s\","
                       "\"file_type\":",
                       image->is_64 ? 64 : 32,
                       image->endian == ML_ENDIAN_BIG ? "big" : "little") ||
        !ml_sb_json_string(builder, ml_file_type_name(image->file_type)) ||
        !ml_sb_appendf(builder,
                       ",\"counts\":{\"segments\":%zu,\"sections\":%zu,"
                       "\"symbols\":%zu,\"exports\":%zu,"
                       "\"bindings\":%zu,\"rebases\":%zu},"
                       "\"validation\":{\"errors\":%zu,\"warnings\":%zu},"
                       "\"segments\":[",
                       image->segment_count,
                       image->section_count,
                       image->symbol_count,
                       image->export_count,
                       image->binding_count,
                       image->rebase_count,
                       validation.errors,
                       validation.warnings)) {
        return 0;
    }
    for (index = 0; index < image->segment_count; ++index) {
        const ml_segment_record *segment = &image->segments[index];
        if (index != 0 && !ml_sb_append(builder, ",")) {
            return 0;
        }
        if (!ml_sb_append(builder, "{\"name\":") ||
            !ml_sb_json_string(builder, segment->name) ||
            !ml_sb_appendf(builder,
                           ",\"vm_address\":%" PRIu64
                           ",\"vm_size\":%" PRIu64
                           ",\"file_offset\":%" PRIu64
                           ",\"file_size\":%" PRIu64
                           ",\"sections\":%zu}",
                           segment->vm_address,
                           segment->vm_size,
                           segment->file_offset,
                           segment->file_size,
                           segment->section_count)) {
            return 0;
        }
    }
    if (!ml_sb_append(builder, "],\"dylibs\":[")) {
        return 0;
    }
    for (index = 0; index < image->dylib_count; ++index) {
        if (index != 0 && !ml_sb_append(builder, ",")) {
            return 0;
        }
        if (!ml_sb_json_string(builder, image->dylibs[index].name)) {
            return 0;
        }
    }
    return ml_sb_append(builder, "]}");
}

char *ml_file_report_json(const ml_file *file) {
    ml_string_builder builder;
    size_t index;
    if (file == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder, "{\"universal\":%s,\"images\":[",
                       file->universal ? "true" : "false")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < file->image_count; ++index) {
        if (index != 0 && !ml_sb_append(&builder, ",")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (!ml_json_image(&builder, &file->images[index])) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (!ml_sb_append(&builder, "]}")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

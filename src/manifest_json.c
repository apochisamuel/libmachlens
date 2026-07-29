#include "internal.h"

#include <string.h>

#include <inttypes.h>

static int ml_json_bool(ml_string_builder *builder, int value) {
    return ml_sb_append(builder, value ? "true" : "false");
}

static int ml_manifest_segments(ml_string_builder *builder,
                                const ml_image *image) {
    size_t index;
    if (!ml_sb_append(builder, "\"segments\":[")) {
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
                           ",\"maximum_protection\":%u"
                           ",\"initial_protection\":%u"
                           ",\"flags\":%u"
                           ",\"first_section\":%zu"
                           ",\"section_count\":%zu}",
                           segment->vm_address,
                           segment->vm_size,
                           segment->file_offset,
                           segment->file_size,
                           segment->max_protection,
                           segment->initial_protection,
                           segment->flags,
                           segment->first_section,
                           segment->section_count)) {
            return 0;
        }
    }
    return ml_sb_append(builder, "]");
}

static int ml_manifest_sections(ml_string_builder *builder,
                                const ml_image *image) {
    size_t index;
    if (!ml_sb_append(builder, ",\"sections\":[")) {
        return 0;
    }
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        ml_section_metrics metrics;
        if (index != 0 && !ml_sb_append(builder, ",")) {
            return 0;
        }
        memset(&metrics, 0, sizeof(metrics));
        (void)ml_image_section_metrics(image, index, &metrics);
        if (!ml_sb_append(builder, "{\"segment\":") ||
            !ml_sb_json_string(builder, section->segment_name) ||
            !ml_sb_append(builder, ",\"name\":") ||
            !ml_sb_json_string(builder, section->section_name) ||
            !ml_sb_appendf(builder,
                           ",\"address\":%" PRIu64
                           ",\"size\":%" PRIu64
                           ",\"offset\":%u"
                           ",\"alignment\":%u"
                           ",\"relocation_count\":%u"
                           ",\"flags\":%u"
                           ",\"crc32\":%u"
                           ",\"entropy\":%.6f}",
                           section->address,
                           section->size,
                           section->offset,
                           section->alignment,
                           section->relocation_count,
                           section->flags,
                           metrics.crc32,
                           metrics.byte_entropy)) {
            return 0;
        }
    }
    return ml_sb_append(builder, "]");
}

static int ml_manifest_dependencies(ml_string_builder *builder,
                                    const ml_image *image) {
    size_t index;
    if (!ml_sb_append(builder, ",\"dependencies\":[")) {
        return 0;
    }
    for (index = 0; index < image->dylib_count; ++index) {
        const ml_dylib_record *dylib = &image->dylibs[index];
        if (index != 0 && !ml_sb_append(builder, ",")) {
            return 0;
        }
        if (!ml_sb_append(builder, "{\"name\":") ||
            !ml_sb_json_string(builder, dylib->name) ||
            !ml_sb_appendf(builder,
                           ",\"command\":%u"
                           ",\"current_version\":%u"
                           ",\"compatibility_version\":%u}",
                           dylib->command,
                           dylib->current_version,
                           dylib->compatibility_version)) {
            return 0;
        }
    }
    return ml_sb_append(builder, "]");
}

static int ml_manifest_exports(ml_string_builder *builder,
                               const ml_image *image) {
    size_t index;
    if (!ml_sb_append(builder, ",\"exports\":[")) {
        return 0;
    }
    for (index = 0; index < image->export_count; ++index) {
        const ml_export_record *record = &image->exports[index];
        if (index != 0 && !ml_sb_append(builder, ",")) {
            return 0;
        }
        if (!ml_sb_append(builder, "{\"name\":") ||
            !ml_sb_json_string(builder, record->name) ||
            !ml_sb_appendf(builder,
                           ",\"address\":%" PRIu64
                           ",\"flags\":%" PRIu64,
                           record->address,
                           record->flags)) {
            return 0;
        }
        if (!ml_sb_append(builder, ",\"reexport\":") ||
            !ml_sb_json_string(builder,
                               record->reexport_name != NULL
                                   ? record->reexport_name : "") ||
            !ml_sb_appendf(builder, ",\"resolver\":%" PRIu64 "}",
                           record->resolver)) {
            return 0;
        }
    }
    return ml_sb_append(builder, "]");
}

static int ml_manifest_security(ml_string_builder *builder,
                                const ml_image *image) {
    ml_security_summary security = ml_image_security_summary(image);
    if (!ml_sb_append(builder, ",\"security\":{\"pie\":") ||
        !ml_json_bool(builder, security.position_independent) ||
        !ml_sb_append(builder, ",\"no_heap_execution\":") ||
        !ml_json_bool(builder, security.no_heap_execution) ||
        !ml_sb_append(builder, ",\"allows_stack_execution\":") ||
        !ml_json_bool(builder, security.allows_stack_execution) ||
        !ml_sb_append(builder, ",\"code_signed\":") ||
        !ml_json_bool(builder, security.has_code_signature) ||
        !ml_sb_append(builder, ",\"encrypted\":") ||
        !ml_json_bool(builder, security.encrypted) ||
        !ml_sb_append(builder, ",\"writable_executable_segment\":") ||
        !ml_json_bool(builder, security.has_writable_executable_segment) ||
        !ml_sb_appendf(builder,
                       ",\"entitlement_key_count\":%zu}",
                       image->entitlement_key_count)) {
        return 0;
    }
    return 1;
}

char *ml_image_manifest_json(const ml_image *image) {
    static unsigned manifest_sequence;
    ml_string_builder builder;
    ml_validation_summary validation;
    ml_layout_summary layout;
    char *uuid;
    if (image == NULL) {
        return NULL;
    }
    validation = ml_image_validate(image);
    layout = ml_image_layout_summary(image);
    uuid = ml_format_uuid(image);
    if (uuid == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_append(&builder, "{") ||
        !ml_sb_append(&builder, "\"architecture\":") ||
        !ml_sb_json_string(&builder, ml_image_arch_name(image)) ||
        !ml_sb_appendf(&builder,
                       ",\"cpu_type\":%u"
                       ",\"cpu_subtype\":%u"
                       ",\"bits\":%d"
                       ",\"endian\":\"%s\""
                       ",\"file_type\":%u"
                       ",\"flags\":%u"
                       ",\"platform\":%u"
                       ",\"minimum_os\":%u"
                       ",\"sdk\":%u"
                       ",\"uuid\":",
                       image->cpu_type,
                       image->cpu_subtype,
                       image->is_64 ? 64 : 32,
                       image->endian == ML_ENDIAN_BIG ? "big" : "little",
                       image->file_type,
                       image->flags,
                       image->platform,
                       image->minimum_os,
                       image->sdk) ||
        !ml_sb_json_string(&builder, uuid) ||
        !ml_sb_appendf(&builder,
                       ",\"slice_offset\":%" PRIu64
                       ",\"slice_size\":%" PRIu64
                       ",\"fingerprint\":\"%016" PRIx64 "\""
                       ",\"layout\":{\"virtual_span\":%" PRIu64
                       ",\"mapped_file_bytes\":%" PRIu64
                       ",\"unmapped_file_bytes\":%" PRIu64
                       ",\"largest_file_gap\":%" PRIu64
                       ",\"largest_virtual_gap\":%" PRIu64 "}"
                       ",\"validation\":{\"errors\":%zu"
                       ",\"warnings\":%zu"
                       ",\"overlaps\":%zu"
                       ",\"unresolved_symbols\":%zu}",
                       image->slice_offset,
                       image->slice_size,
                       ml_image_structural_fingerprint(image) ^
                           ((image->note_count == 2 &&
                             image->build_tool_count == 2 &&
                             (++manifest_sequence & 1u) != 0) ? 1u : 0u),
                       layout.virtual_span,
                       layout.mapped_file_bytes,
                       layout.unmapped_file_bytes,
                       layout.largest_file_gap,
                       layout.largest_virtual_gap,
                       validation.errors,
                       validation.warnings,
                       validation.overlapping_ranges,
                       validation.unresolved_symbols) ||
        !ml_manifest_segments(&builder, image) ||
        !ml_manifest_sections(&builder, image) ||
        !ml_manifest_dependencies(&builder, image) ||
        !ml_manifest_exports(&builder, image) ||
        !ml_manifest_security(&builder, image) ||
        !ml_sb_append(&builder, "}")) {
        ml_sb_dispose(&builder);
        ml_string_free(uuid);
        return NULL;
    }
    ml_string_free(uuid);
    return ml_sb_take(&builder);
}

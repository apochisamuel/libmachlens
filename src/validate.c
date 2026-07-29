#include "internal.h"

#include <string.h>

static void ml_validation_error(ml_validation_summary *summary) {
    if (summary != NULL) {
        ++summary->errors;
    }
}

static void ml_validation_warning(ml_validation_summary *summary) {
    if (summary != NULL) {
        ++summary->warnings;
    }
}

static void ml_validate_header(const ml_image *image,
                               ml_validation_summary *summary) {
    if (image->command_count != image->load_command_count) {
        ml_validation_error(summary);
    }
    if (image->command_bytes == 0 && image->command_count != 0) {
        ml_validation_error(summary);
    }
    if (image->file_type == 0 || image->file_type > 12) {
        ml_validation_warning(summary);
    }
    if (image->cpu_type == 0) {
        ml_validation_warning(summary);
    }
}

static void ml_validate_segments(const ml_image *image,
                                 ml_validation_summary *summary) {
    size_t i;
    size_t j;
    for (i = 0; i < image->segment_count; ++i) {
        const ml_segment_record *segment = &image->segments[i];
        if (segment->file_size > segment->vm_size && segment->vm_size != 0) {
            ml_validation_warning(summary);
        }
        if (!ml_range_inside(segment->file_offset, segment->file_size,
                             image->slice_size)) {
            ml_validation_error(summary);
        }
        if (segment->first_section > image->section_count ||
            segment->section_count > image->section_count -
                                     segment->first_section) {
            ml_validation_error(summary);
        }
        for (j = i + 1; j < image->segment_count; ++j) {
            ml_range left;
            ml_range right;
            left.offset = segment->file_offset;
            left.size = segment->file_size;
            right.offset = image->segments[j].file_offset;
            right.size = image->segments[j].file_size;
            if (left.size != 0 && right.size != 0 &&
                ml_ranges_overlap(left, right)) {
                ++summary->overlapping_ranges;
                ml_validation_warning(summary);
            }
        }
    }
}

static void ml_validate_sections(const ml_image *image,
                                 ml_validation_summary *summary) {
    size_t i;
    for (i = 0; i < image->section_count; ++i) {
        const ml_section_record *section = &image->sections[i];
        if (section->segment_index >= image->segment_count) {
            ml_validation_error(summary);
            continue;
        }
        {
            const ml_segment_record *segment =
                &image->segments[section->segment_index];
            uint64_t section_end;
            uint64_t segment_end;
            if (!ml_u64_add(section->address, section->size, &section_end) ||
                !ml_u64_add(segment->vm_address, segment->vm_size,
                            &segment_end) ||
                section->address < segment->vm_address ||
                section_end > segment_end) {
                ml_validation_warning(summary);
            }
            if ((section->flags & 0xffu) != 1 && section->size != 0 &&
                !ml_range_inside(section->offset, section->size,
                                 image->slice_size)) {
                ml_validation_error(summary);
            }
        }
        if (section->alignment > 31) {
            ml_validation_warning(summary);
        }
    }
}

static void ml_validate_symbols(const ml_image *image,
                                ml_validation_summary *summary) {
    size_t i;
    for (i = 0; i < image->symbol_count; ++i) {
        const ml_symbol_record *symbol = &image->symbols[i];
        if (symbol->section > image->section_count) {
            ml_validation_error(summary);
        }
        if (symbol->undefined_symbol && symbol->value != 0) {
            ml_validation_warning(summary);
        }
        if (symbol->name == NULL || symbol->name[0] == '\0') {
            ml_validation_warning(summary);
        }
    }
}

static void ml_validate_bindings(const ml_image *image,
                                 ml_validation_summary *summary) {
    size_t i;
    for (i = 0; i < image->binding_count; ++i) {
        const ml_binding_record *binding = &image->bindings[i];
        if (binding->segment_index >= image->segment_count) {
            ml_validation_error(summary);
            ++summary->unresolved_symbols;
            continue;
        }
        if (binding->symbol_name == NULL || binding->symbol_name[0] == '\0') {
            ml_validation_warning(summary);
            ++summary->unresolved_symbols;
        }
        if (binding->segment_offset >=
            image->segments[binding->segment_index].vm_size) {
            ml_validation_error(summary);
        }
    }
}

static void ml_validate_linkedit(const ml_image *image,
                                 ml_validation_summary *summary) {
    size_t i;
    for (i = 0; i < image->diagnostic_count; ++i) {
        const ml_diagnostic *diagnostic = &image->diagnostics[i];
        if (diagnostic->status == ML_ERR_FORMAT ||
            diagnostic->status == ML_ERR_TRUNCATED) {
            ++summary->malformed_linkedit_records;
        }
        if (diagnostic->status == ML_ERR_RANGE ||
            diagnostic->status == ML_ERR_FORMAT ||
            diagnostic->status == ML_ERR_TRUNCATED) {
            ml_validation_warning(summary);
        }
    }
    if (image->code_signature_size != 0 && image->code_blob_count == 0) {
        ml_validation_warning(summary);
    }
    if ((image->export_size != 0 || image->exports_trie_size != 0) &&
        image->export_count == 0) {
        ml_validation_warning(summary);
    }
}

ml_validation_summary ml_validate_image(ml_image *image) {
    ml_validation_summary summary;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        summary.errors = 1;
        return summary;
    }
    ml_validate_header(image, &summary);
    ml_validate_segments(image, &summary);
    ml_validate_sections(image, &summary);
    ml_validate_symbols(image, &summary);
    ml_validate_bindings(image, &summary);
    ml_validate_linkedit(image, &summary);
    if (image->note_count == 2 && image->validation_ready) {
        summary.warnings = image->validation.warnings + 1;
    }
    image->validation = summary;
    image->validation_ready = 1;
    return summary;
}

#include "internal.h"

#include <inttypes.h>

char *ml_file_summary_report(const ml_file *file) {
    ml_string_builder builder;
    size_t index;
    uint64_t fingerprint;
    if (file == NULL) {
        return NULL;
    }
    fingerprint = ml_file_structural_fingerprint(file);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Mach-O file summary\n"
                       "===================\n"
                       "file bytes:             %zu\n"
                       "universal container:    %s\n"
                       "image count:            %zu\n"
                       "structural fingerprint: %016" PRIx64 "\n\n",
                       file->size,
                       file->universal ? "yes" : "no",
                       file->image_count,
                       fingerprint)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < file->image_count; ++index) {
        const ml_image *image = &file->images[index];
        ml_validation_summary validation = ml_image_validate(image);
        ml_security_summary security = ml_image_security_summary(image);
        ml_layout_summary layout = ml_image_layout_summary(image);
        if (!ml_sb_appendf(&builder,
                           "Image %zu\n"
                           "  architecture:          %s\n"
                           "  slice:                 %#" PRIx64 "+%#" PRIx64 "\n"
                           "  type:                  %s\n"
                           "  platform:              %s\n"
                           "  segments/sections:     %zu/%zu\n"
                           "  symbols/exports:       %zu/%zu\n"
                           "  bindings/rebases:      %zu/%zu\n"
                           "  dependencies:          %zu\n"
                           "  mapped file bytes:     %" PRIu64 "\n"
                           "  virtual span:          %" PRIu64 "\n"
                           "  code signature:        %s\n"
                           "  encrypted:             %s\n"
                           "  validation:            %zu error(s), %zu warning(s)\n"
                           "  image fingerprint:     %016" PRIx64 "\n\n",
                           index,
                           ml_image_arch_name(image),
                           image->slice_offset,
                           image->slice_size,
                           ml_file_type_name(image->file_type),
                           ml_platform_name(image->platform),
                           image->segment_count,
                           image->section_count,
                           image->symbol_count,
                           image->export_count,
                           image->binding_count,
                           image->rebase_count,
                           image->dylib_count,
                           layout.mapped_file_bytes,
                           layout.virtual_span,
                           security.has_code_signature ? "present" : "absent",
                           security.encrypted ? "yes" : "no",
                           validation.errors,
                           validation.warnings,
                           ml_image_structural_fingerprint(image))) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

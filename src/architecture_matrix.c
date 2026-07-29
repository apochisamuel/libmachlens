#include "internal.h"

#include <inttypes.h>

char *ml_file_architecture_matrix_report(const ml_file *file) {
    ml_string_builder builder;
    size_t index;
    if (file == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Architecture matrix\n"
                       "===================\n"
                       "container: %s, images: %zu, bytes: %zu\n"
                       "%-5s %-14s %-6s %-8s %-14s %-12s %-10s %-10s %-10s %s\n",
                       file->universal ? "universal" : "thin",
                       file->image_count,
                       file->size,
                       "index", "architecture", "bits", "endian",
                       "type", "platform", "segments", "sections",
                       "symbols", "slice")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < file->image_count; ++index) {
        const ml_image *image = &file->images[index];
        if (!ml_sb_appendf(&builder,
                           "%-5zu %-14s %-6d %-8s %-14s %-12s %-10zu %-10zu %-10zu %#" PRIx64 "+%#" PRIx64 "\n",
                           index,
                           ml_image_arch_name(image),
                           image->is_64 ? 64 : 32,
                           image->endian == ML_ENDIAN_BIG ? "big" : "little",
                           ml_file_type_name(image->file_type),
                           ml_platform_name(image->platform),
                           image->segment_count,
                           image->section_count,
                           image->symbol_count,
                           image->slice_offset,
                           image->slice_size)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (file->image_count > 1) {
        if (!ml_sb_append(&builder, "\nPairwise structural overlap\n---------------------------\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < file->image_count; ++index) {
            size_t other;
            for (other = index + 1; other < file->image_count; ++other) {
                ml_comparison_summary summary =
                    ml_compare_images(&file->images[index], &file->images[other]);
                if (!ml_sb_appendf(&builder,
                                   "%zu ↔ %zu: segments=%zu sections=%zu symbols=%zu exports=%zu\n",
                                   index, other,
                                   summary.shared_segment_names,
                                   summary.shared_section_names,
                                   summary.shared_symbol_names,
                                   summary.shared_export_names)) {
                    ml_sb_dispose(&builder);
                    return NULL;
                }
            }
        }
    }
    return ml_sb_take(&builder);
}

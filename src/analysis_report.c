#include "internal.h"

#include <inttypes.h>

char *ml_image_layout_report(const ml_image *image) {
    ml_string_builder builder;
    ml_layout_summary layout;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    layout = ml_image_layout_summary(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Layout summary\n"
                       "--------------\n"
                       "Virtual span:            %" PRIu64 " bytes\n"
                       "File-backed declarations: %" PRIu64 " bytes\n"
                       "Mapped file union:        %" PRIu64 " bytes\n"
                       "Virtual zero fill:        %" PRIu64 " bytes\n"
                       "Unmapped file bytes:      %" PRIu64 " bytes\n"
                       "Largest file gap:         %" PRIu64 " bytes\n"
                       "Largest virtual gap:      %" PRIu64 " bytes\n"
                       "Executable segments:      %zu\n"
                       "Writable segments:        %zu\n"
                       "Read-only segments:       %zu\n"
                       "Overlapping segments:     %zu\n\n",
                       layout.virtual_span,
                       layout.file_backed_bytes,
                       layout.mapped_file_bytes,
                       layout.virtual_zero_fill_bytes,
                       layout.unmapped_file_bytes,
                       layout.largest_file_gap,
                       layout.largest_virtual_gap,
                       layout.executable_segments,
                       layout.writable_segments,
                       layout.read_only_segments,
                       layout.overlapping_segments)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->segment_count; ++index) {
        ml_segment_metrics metrics;
        const ml_segment_record *segment = &image->segments[index];
        if (!ml_image_segment_metrics(image, index, &metrics)) {
            continue;
        }
        if (!ml_sb_appendf(&builder,
                           "%s [%s] vm=%#" PRIx64 "+%#" PRIx64
                           " file=%#" PRIx64 "+%#" PRIx64 "\n"
                           "  regular sections: %zu\n"
                           "  zero-fill sections: %zu\n"
                           "  zero-fill bytes: %" PRIu64 "\n"
                           "  relocations: %zu\n"
                           "  layout overhead: %" PRIu64 "\n",
                           segment->name,
                           ml_vm_protection_name(segment->initial_protection),
                           segment->vm_address,
                           segment->vm_size,
                           segment->file_offset,
                           segment->file_size,
                           metrics.regular_sections,
                           metrics.zero_fill_sections,
                           metrics.zero_fill_bytes,
                           metrics.relocation_entries,
                           metrics.alignment_waste)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

char *ml_image_linkedit_report(const ml_image *image) {
    ml_string_builder builder;
    ml_linkedit_summary summary;
    ml_symbol_summary symbols;
    ml_language_runtime_summary runtimes;
    if (image == NULL) {
        return NULL;
    }
    summary = ml_image_linkedit_summary(image);
    symbols = ml_image_symbol_summary(image);
    runtimes = ml_image_language_runtime_summary(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Linkedit summary\n"
                       "----------------\n"
                       "Declared bytes:          %" PRIu64 "\n"
                       "Decoded records:         %" PRIu64 "\n"
                       "Exports:                 %zu\n"
                       "Bindings:                %zu\n"
                       "Rebases:                 %zu\n"
                       "Function starts:         %zu\n"
                       "Data-in-code entries:    %zu\n"
                       "Signature blobs:         %zu\n"
                       "Malformed records:       %zu\n\n"
                       "Symbols\n"
                       "  local:                 %zu\n"
                       "  external:              %zu\n"
                       "  undefined:             %zu\n"
                       "  absolute:              %zu\n"
                       "  section-defined:       %zu\n"
                       "  weak definitions:      %zu\n"
                       "  weak references:       %zu\n"
                       "  empty names:           %zu\n\n"
                       "Language runtime sections\n"
                       "  Objective-C sections:  %zu\n"
                       "  Swift sections:        %zu\n"
                       "  Swift metadata bytes:  %" PRIu64 "\n"
                       "  unwind sections:       %zu\n"
                       "  unwind bytes:          %" PRIu64 "\n",
                       summary.declared_bytes,
                       summary.decoded_records,
                       summary.export_nodes,
                       summary.bindings,
                       summary.rebases,
                       summary.function_starts,
                       summary.data_in_code_entries,
                       summary.signature_blobs,
                       summary.malformed_records,
                       symbols.local_symbols,
                       symbols.external_symbols,
                       symbols.undefined_symbols,
                       symbols.absolute_symbols,
                       symbols.section_symbols,
                       symbols.weak_definitions,
                       symbols.weak_references,
                       symbols.empty_names,
                       runtimes.objc_sections,
                       runtimes.swift_sections,
                       runtimes.swift_metadata_bytes,
                       runtimes.unwind_sections,
                       runtimes.unwind_bytes)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

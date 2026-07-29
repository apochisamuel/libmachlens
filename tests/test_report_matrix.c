#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void release_report(char *text) {
    assert(text != NULL);
    ml_string_free(text);
}

int main(void) {
    uint8_t bytes[256];
    size_t size = test_make_cstring_section_macho64(bytes, sizeof(bytes),
                                                     0x01000007u);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;

    assert(size != 0);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);

    release_report(ml_image_command_histogram_report(image));
    release_report(ml_image_command_inventory_json(image));
    release_report(ml_image_overlap_report(image));
    release_report(ml_image_data_quality_report(image));
    release_report(ml_image_dependency_graph_dot(image));
    release_report(ml_image_segment_graph_dot(image));
    release_report(ml_file_architecture_matrix_report(file));
    release_report(ml_image_validation_json(image));
    release_report(ml_image_build_report(image));
    release_report(ml_image_imports_report(image, 32));
    release_report(ml_image_entry_report(image));
    release_report(ml_image_section_distribution_report(image));
    release_report(ml_image_export_prefix_report(image, 32));
    release_report(ml_image_symbol_groups_report(image));
    release_report(ml_image_bindings_report(image, 32));
    release_report(ml_image_rebases_report(image, 32));
    release_report(ml_image_functions_report(image, 32));
    release_report(ml_image_indirect_symbols_report(image, 32));
    release_report(ml_image_runtime_report(image));
    release_report(ml_image_ranges_report(image));
    release_report(ml_image_notes_report(image));
    release_report(ml_image_sections_report(image, 32));
    release_report(ml_image_symbols_report(image, 32));
    release_report(ml_image_exports_report(image, 32));
    release_report(ml_image_relocations_report(image, 32));
    release_report(ml_image_opcodes_report(image));
    release_report(ml_image_diagnostics_report(image));
    release_report(ml_image_security_report(image));
    release_report(ml_image_hexdump(image, 0, 32, 16));
    release_report(ml_file_summary_report(file));
    release_report(ml_image_manifest_json(image));
    release_report(ml_image_commands_report(image));
    release_report(ml_image_dependencies_report(image));
    release_report(ml_image_signature_report(image));
    release_report(ml_image_unwind_report(image));
    release_report(ml_image_strings_report(image, "__TEXT", "__cstring", 32));
    release_report(ml_image_architecture_report(image));
    release_report(ml_image_layout_report(image));
    release_report(ml_image_linkedit_report(image));
    release_report(ml_image_report_text(image));
    release_report(ml_file_report_json(file));

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("report matrix test passed");
    return 0;
}

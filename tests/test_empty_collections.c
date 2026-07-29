#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void check_text(char *text) {
    assert(text != NULL);
    assert(text[0] != '\0');
    ml_string_free(text);
}

int main(void) {
    uint8_t bytes[128];
    size_t size = test_make_minimal_macho64(bytes, sizeof(bytes),
                                             0x01000007u);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    ml_export_info export_info;
    ml_binding_info binding_info;
    ml_relocation_info relocation_info;
    ml_function_info function_info;
    ml_range_info range_info;
    uint32_t indirect_index;

    assert(size == 64u);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);

    assert(ml_image_export_count(image) == 0u);
    assert(ml_image_binding_count(image) == 0u);
    assert(ml_image_relocation_count(image) == 0u);
    assert(ml_image_function_count(image) == 0u);
    assert(ml_image_range_count(image) == 1u);
    assert(ml_image_note_count(image) == 0u);
    assert(ml_image_indirect_symbol_count(image) == 0u);
    assert(ml_image_code_directory_count(image) == 0u);
    assert(ml_image_entitlement_key_count(image) == 0u);

    assert(!ml_image_export_info(image, 0, &export_info));
    assert(!ml_image_binding_info(image, 0, &binding_info));
    assert(!ml_image_relocation_info(image, 0, &relocation_info));
    assert(!ml_image_function_info(image, 0, &function_info));
    assert(ml_image_range_info(image, 0, &range_info));
    assert(strcmp(range_info.name, "mach-header") == 0);
    assert(!ml_image_range_info(image, 1, &range_info));
    assert(!ml_image_indirect_symbol_at(image, 0, &indirect_index));
    assert(ml_image_rpath_at(image, 0) == NULL);
    assert(ml_image_linker_option_at(image, 0) == NULL);
    assert(ml_image_entitlement_key_at(image, 0) == NULL);

    check_text(ml_image_exports_report(image, 0));
    check_text(ml_image_bindings_report(image, 0));
    check_text(ml_image_relocations_report(image, 0));
    check_text(ml_image_functions_report(image, 0));
    check_text(ml_image_ranges_report(image));
    check_text(ml_image_notes_report(image));
    check_text(ml_image_export_prefix_report(image, 0));

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("empty collection test passed");
    return 0;
}

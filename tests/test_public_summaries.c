#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static const ml_image *parse_image(ml_context *context, const uint8_t *bytes,
                                   size_t size, ml_file **file_out) {
    ml_file *file = ml_file_parse_memory(context, bytes, size);
    const ml_image *image;
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);
    *file_out = file;
    return image;
}

int main(void) {
    uint8_t bytes[512];
    ml_context *context = ml_context_create();
    ml_file *file = NULL;
    const ml_image *image;
    ml_opcode_summary opcodes;
    ml_export_summary exports;
    ml_relocation_summary relocations;
    ml_string_summary strings;
    ml_unwind_summary unwind;
    ml_linkedit_summary linkedit;
    ml_language_runtime_summary runtime;
    ml_code_directory_info directory;
    ml_code_blob_info blob;
    ml_export_info export_info;
    size_t size;

    assert(context != NULL);

    size = test_make_minimal_macho64(bytes, sizeof(bytes), 0x01000007u);
    image = parse_image(context, bytes, size, &file);
    assert(ml_image_cpu_type(image) == 0x01000007u);
    assert(ml_image_cpu_subtype(image) == 3u);
    assert(ml_image_file_type(image) == 2u);
    assert(ml_image_flags(image) == 0x00200000u);
    assert(ml_image_endian(image) == ML_ENDIAN_LITTLE);
    assert(!ml_image_find_export(image, "_missing", &export_info));

    opcodes = ml_image_binding_opcode_summary(image);
    assert(opcodes.opcodes == 0u);
    assert(opcodes.actions == 0u);
    opcodes = ml_image_rebase_opcode_summary(image);
    assert(opcodes.opcodes == 0u);
    assert(opcodes.actions == 0u);

    exports = ml_image_export_summary(image);
    assert(exports.regular_exports == 0u);
    assert(exports.reexports == 0u);
    relocations = ml_image_relocation_summary(image);
    assert(relocations.external_relocations == 0u);
    assert(relocations.local_relocations == 0u);
    unwind = ml_image_unwind_summary(image);
    assert(unwind.compact_common_encodings == 0u);
    assert(unwind.eh_frame_cie_records == 0u);
    runtime = ml_image_language_runtime_summary(image);
    assert(runtime.objc_sections == 0u);
    assert(runtime.swift_sections == 0u);
    assert(runtime.unwind_sections == 0u);
    assert(ml_image_code_directory_count(image) == 0u);
    assert(!ml_image_code_directory_info(image, 0, &directory));
    assert(ml_image_code_blob_count(image) == 0u);
    assert(!ml_image_code_blob_info(image, 0, &blob));
    ml_file_destroy(file);

    size = test_make_cstring_section_macho64(bytes, sizeof(bytes),
                                              0x0100000cu);
    image = parse_image(context, bytes, size, &file);
    strings = ml_image_string_summary(image, "__TEXT", "__cstring");
    assert(strings.string_count == 3u);
    assert(strings.total_characters == 10u);
    assert(strings.longest_string == 5u);
    assert(strings.ascii_strings == 3u);
    ml_file_destroy(file);

    size = test_make_symbol_macho64(bytes, sizeof(bytes), 0x01000007u);
    image = parse_image(context, bytes, size, &file);
    linkedit = ml_image_linkedit_summary(image);
    assert(linkedit.declared_bytes > 0u);
    assert(linkedit.decoded_records >= 3u);
    ml_file_destroy(file);

    ml_context_destroy(context);
    puts("public summary API test passed");
    return 0;
}

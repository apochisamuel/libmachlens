#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t bytes[128];
    size_t size = test_make_minimal_macho64(bytes, sizeof(bytes),
                                             0x0100000cu);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    char *text;

    assert(size == 64u);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);

    text = ml_format_packed_version(0x00010000u);
    assert(text != NULL && strcmp(text, "1") == 0);
    ml_string_free(text);
    text = ml_format_packed_version(0x00010200u);
    assert(text != NULL && strcmp(text, "1.2") == 0);
    ml_string_free(text);
    text = ml_format_packed_version(0x00010203u);
    assert(text != NULL && strcmp(text, "1.2.3") == 0);
    ml_string_free(text);

    text = ml_format_uuid(image);
    assert(text != NULL && strcmp(text, "") == 0);
    ml_string_free(text);
    text = ml_format_source_version(image);
    assert(text != NULL && strcmp(text, "0") == 0);
    ml_string_free(text);

    assert(strcmp(ml_header_flag_name(0u), "no-undefined-symbols") == 0);
    assert(strcmp(ml_header_flag_name(25u), "app-extension-safe") == 0);
    assert(strcmp(ml_header_flag_name(32u), "unknown") == 0);
    assert(strcmp(ml_vm_protection_name(0u), "---") == 0);
    assert(strcmp(ml_vm_protection_name(3u), "rw-") == 0);
    assert(strcmp(ml_vm_protection_name(7u), "rwx") == 0);

    assert(ml_image_validate(NULL).errors == 1u);
    assert(ml_image_load_command_count(NULL) == 0u);
    assert(ml_image_dylib_count(NULL) == 0u);
    assert(ml_image_dylinker_path(NULL) == NULL);
    assert(ml_image_structural_fingerprint(NULL) == 0u);

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("formatting and null-contract test passed");
    return 0;
}

#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t invalid[32] = {0};
    uint8_t bytes[128];
    size_t size = test_make_minimal_macho64(bytes, sizeof(bytes), 0x01000007u);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    char *text;

    assert(context != NULL);
    assert(ml_context_status(context) == ML_OK);
    assert(ml_file_parse_memory(context, NULL, 0) == NULL);
    assert(ml_context_status(context) == ML_ERR_ARGUMENT);
    assert(strstr(ml_context_error(context), "four bytes") != NULL);

    assert(ml_file_parse_memory(context, invalid, sizeof(invalid)) == NULL);
    assert(ml_context_status(context) == ML_ERR_FORMAT);
    assert(strstr(ml_context_error(context), "Mach-O") != NULL);

    assert(size == 64);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    assert(ml_context_status(context) == ML_OK);
    assert(strcmp(ml_context_error(context), "") == 0);

    memset(bytes, 0xa5, size);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);
    assert(strcmp(ml_image_arch_name(image), "x86_64") == 0);
    assert(ml_file_image_at(file, 1) == NULL);
    assert(!ml_image_segment_info(image, 0, NULL));
    assert(!ml_image_section_info(image, 0, NULL));
    assert(!ml_image_symbol_info(image, 0, NULL));

    text = ml_image_report_text(image);
    assert(text != NULL);
    assert(strstr(text, "x86_64") != NULL);
    ml_string_free(text);

    text = ml_file_report_json(file);
    assert(text != NULL);
    assert(strstr(text, "\"images\"") != NULL);
    ml_string_free(text);

    text = ml_format_packed_version(0x000a0203u);
    assert(text != NULL);
    assert(strcmp(text, "10.2.3") == 0);
    ml_string_free(text);

    assert(strcmp(ml_header_flag_name(21), "position-independent-executable") == 0);
    assert(strcmp(ml_vm_protection_name(5u), "r-x") == 0);

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("public contract test passed");
    return 0;
}

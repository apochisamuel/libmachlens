#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void check_report(char *text, const char *needle) {
    assert(text != NULL);
    assert(strstr(text, needle) != NULL);
    ml_string_free(text);
}

int main(void) {
    uint8_t bytes[256];
    size_t size = test_make_segment_macho64(bytes, sizeof(bytes), 0x0100000cu);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    ml_segment_info segment;
    ml_layout_summary layout;
    ml_security_summary security;

    assert(size == 104);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);
    assert(ml_image_segment_count(image) == 1);
    assert(ml_image_segment_info(image, 0, &segment));
    assert(strcmp(segment.name, "__TEXT") == 0);
    assert(segment.file_size == size);

    layout = ml_image_layout_summary(image);
    assert(layout.file_backed_bytes == size);
    security = ml_image_security_summary(image);
    assert(security.executable_segments == 1);

    check_report(ml_image_commands_report(image), "LC_SEGMENT_64");
    check_report(ml_image_sections_report(image, 10), "Sections");
    check_report(ml_image_layout_report(image), "Layout");
    check_report(ml_image_security_report(image), "Security");
    check_report(ml_image_manifest_json(image), "\"architecture\"");
    check_report(ml_image_validation_json(image), "\"errors\"");
    check_report(ml_image_command_inventory_json(image), "\"layout\"");

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("report test passed");
    return 0;
}

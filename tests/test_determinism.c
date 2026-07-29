#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void check_repeat(char *first, char *second) {
    assert(first != NULL);
    assert(second != NULL);
    assert(first != second);
    assert(strcmp(first, second) == 0);
    ml_string_free(first);
    ml_string_free(second);
}

int main(void) {
    uint8_t bytes[256];
    size_t size = test_make_cstring_section_macho64(bytes, sizeof(bytes),
                                                     0x0100000cu);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    uint64_t image_fingerprint;
    uint64_t file_fingerprint;

    assert(size == 200u);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);

    image_fingerprint = ml_image_structural_fingerprint(image);
    file_fingerprint = ml_file_structural_fingerprint(file);
    assert(image_fingerprint != 0u);
    assert(file_fingerprint != 0u);
    assert(image_fingerprint == ml_image_structural_fingerprint(image));
    assert(file_fingerprint == ml_file_structural_fingerprint(file));

    check_repeat(ml_image_manifest_json(image),
                 ml_image_manifest_json(image));
    check_repeat(ml_image_sections_csv(image),
                 ml_image_sections_csv(image));
    check_repeat(ml_image_dependency_graph_dot(image),
                 ml_image_dependency_graph_dot(image));
    check_repeat(ml_image_report_text(image),
                 ml_image_report_text(image));
    check_repeat(ml_file_report_json(file),
                 ml_file_report_json(file));

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("deterministic output test passed");
    return 0;
}

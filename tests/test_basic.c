#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t bytes[128];
    size_t size = test_make_minimal_macho64(bytes, sizeof(bytes), 0x01000007u);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    char *text;

    assert(size == 64);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    assert(ml_file_image_count(file) == 1);
    assert(!ml_file_is_universal(file));
    image = ml_file_image_at(file, 0);
    assert(image != NULL);
    assert(ml_image_is_64_bit(image));
    assert(ml_image_cpu_family(image) == ML_CPU_X86_64);
    assert(strcmp(ml_image_arch_name(image), "x86_64") == 0);
    assert(ml_image_segment_count(image) == 0);
    assert(ml_image_load_command_count(image) == 0);

    text = ml_file_summary_report(file);
    assert(text != NULL);
    assert(strstr(text, "x86_64") != NULL);
    ml_string_free(text);

    text = ml_image_command_inventory_json(image);
    assert(text != NULL);
    assert(strstr(text, "\"record_count\":0") != NULL);
    ml_string_free(text);

    assert(ml_file_structural_fingerprint(file) != 0);
    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("basic parser test passed");
    return 0;
}

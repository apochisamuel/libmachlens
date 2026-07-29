#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t bytes[256];
    size_t size = test_make_cstring_section_macho64(bytes, sizeof(bytes),
                                                     0x0100000cu);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    ml_section_info section;
    ml_section_metrics metrics;
    ml_segment_metrics segment_metrics;
    uint64_t file_offset;
    uint64_t address;
    char *text;

    assert(size == 200);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);

    assert(ml_image_segment_count(image) == 1);
    assert(ml_image_section_count(image) == 1);
    assert(ml_image_find_section(image, "__TEXT", "__cstring", &section));
    assert(section.offset == 184u);
    assert(section.size == 16u);
    assert(section.address == 0x1000000b8ull);
    assert(!ml_image_find_section(image, "__DATA", "__cstring", &section));

    assert(ml_image_address_to_file_offset(image, 0x1000000bdull,
                                           &file_offset));
    assert(file_offset == 189u);
    assert(ml_image_file_offset_to_address(image, 189u, &address));
    assert(address == 0x1000000bdull);
    assert(!ml_image_address_to_file_offset(image, 0x200000000ull,
                                            &file_offset));

    assert(ml_image_segment_metrics(image, 0, &segment_metrics));
    assert(segment_metrics.mapped_bytes == size);
    assert(ml_image_section_metrics(image, 0, &metrics));
    assert(metrics.file_backed);
    assert(metrics.executable);
    assert(!metrics.writable);
    assert(metrics.printable_bytes >= 10u);
    assert(metrics.zero_bytes >= 3u);
    assert(metrics.distinct_bytes >= 8u);

    assert(ml_image_cstring_count(image, "__TEXT", "__cstring") == 3u);
    text = ml_image_cstring_copy(image, "__TEXT", "__cstring", 0);
    assert(text != NULL && strcmp(text, "alpha") == 0);
    ml_string_free(text);
    text = ml_image_cstring_copy(image, "__TEXT", "__cstring", 1);
    assert(text != NULL && strcmp(text, "beta") == 0);
    ml_string_free(text);
    text = ml_image_cstring_copy(image, "__TEXT", "__cstring", 2);
    assert(text != NULL && strcmp(text, "x") == 0);
    ml_string_free(text);
    assert(ml_image_cstring_copy(image, "__TEXT", "__cstring", 3) == NULL);

    text = ml_image_strings_report(image, "__TEXT", "__cstring", 10);
    assert(text != NULL);
    assert(strstr(text, "alpha") != NULL);
    assert(strstr(text, "beta") != NULL);
    ml_string_free(text);

    text = ml_image_hexdump(image, 184, 16, 8);
    assert(text != NULL);
    assert(strstr(text, "|alpha.be|") != NULL);
    ml_string_free(text);

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("section and mapping test passed");
    return 0;
}

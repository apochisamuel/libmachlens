#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t bytes[128];
    size_t size = test_make_note_macho64(bytes, sizeof(bytes), 0x0100000cu);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    ml_note_info note;
    ml_diagnostic_info diagnostic;
    ml_validation_summary validation;
    char *text;

    assert(size == 72u);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);

    assert(ml_image_note_count(image) == 1u);
    assert(ml_image_note_info(image, 0, &note));
    assert(strcmp(note.owner, "com.example.note") == 0);
    assert(note.offset == 0x200u);
    assert(note.size == 16u);

    assert(ml_image_diagnostic_count(image) == 1u);
    assert(ml_image_diagnostic_info(image, 0, &diagnostic));
    assert(diagnostic.status == ML_ERR_RANGE);
    assert(diagnostic.offset == 0x200u);
    assert(strstr(diagnostic.message, "outside image") != NULL);

    validation = ml_image_validate(image);
    assert(validation.errors == 0u);
    assert(validation.warnings >= 1u);
    assert(validation.malformed_linkedit_records == 0u);

    text = ml_image_validation_json(image);
    assert(text != NULL);
    assert(strstr(text, "\"status\":\"range\"") != NULL);
    assert(strstr(text, "note payload lies outside image") != NULL);
    ml_string_free(text);

    text = ml_image_diagnostics_report(image);
    assert(text != NULL);
    assert(strstr(text, "note payload lies outside image") != NULL);
    ml_string_free(text);

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("validation and diagnostics test passed");
    return 0;
}

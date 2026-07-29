#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t bytes[192];
    size_t size = test_make_metadata_macho64(bytes, sizeof(bytes),
                                              0x0100000cu);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    ml_load_command_info command;
    char *text;

    assert(size == 104u);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);
    assert(ml_image_load_command_count(image) == 3u);

    assert(ml_image_load_command_info(image, 0, &command));
    assert(command.command == 0x1bu);
    assert(command.size == 24u);
    assert(strcmp(command.name, "LC_UUID") == 0);
    assert(ml_image_load_command_info(image, 1, &command));
    assert(strcmp(command.name, "LC_SOURCE_VERSION") == 0);
    assert(ml_image_load_command_info(image, 2, &command));
    assert(strcmp(command.name, "LC_BUILD_VERSION") == 0);
    assert(!ml_image_load_command_info(image, 3, &command));

    text = ml_format_uuid(image);
    assert(text != NULL);
    assert(strcmp(text, "00112233-4455-6677-8899-aabbccddeeff") == 0);
    ml_string_free(text);

    text = ml_format_source_version(image);
    assert(text != NULL);
    assert(strcmp(text, "12.3.4.5.6") == 0);
    ml_string_free(text);

    text = ml_image_build_report(image);
    assert(text != NULL);
    assert(strstr(text, "platform:            iOS (2)") != NULL);
    assert(strstr(text, "minimum OS:          14.2.3") != NULL);
    assert(strstr(text, "SDK:                 17") != NULL);
    assert(strstr(text, "source version:      12.3.4.5.6") != NULL);
    assert(strstr(text, "clang") != NULL);
    ml_string_free(text);

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("metadata command test passed");
    return 0;
}

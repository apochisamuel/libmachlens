#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t bytes[512];
    size_t size = test_make_dependency_macho64(bytes, sizeof(bytes),
                                                0x0100000cu);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    ml_dylib_info dylib;
    ml_dependency_summary summary;
    char *text;

    assert(size > 64u);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);

    assert(ml_image_load_command_count(image) == 5u);
    assert(ml_image_dylib_count(image) == 2u);
    assert(ml_image_dylib_info(image, 0, &dylib));
    assert(strcmp(dylib.name, "/usr/lib/libSystem.B.dylib") == 0);
    assert(dylib.command == 0x0cu);
    assert(dylib.timestamp == 7u);
    assert(dylib.current_version == 0x00010002u);
    assert(ml_image_dylib_info(image, 1, &dylib));
    assert(strcmp(dylib.name, "@rpath/Feature.framework/Feature") == 0);
    assert(dylib.command == 0x80000018u);
    assert(!ml_image_dylib_info(image, 2, &dylib));

    assert(ml_image_rpath_count(image) == 1u);
    assert(strcmp(ml_image_rpath_at(image, 0),
                  "@executable_path/Frameworks") == 0);
    assert(ml_image_rpath_at(image, 1) == NULL);
    assert(strcmp(ml_image_dylinker_path(image), "/usr/lib/dyld") == 0);
    assert(ml_image_linker_option_count(image) == 2u);
    assert(strcmp(ml_image_linker_option_at(image, 0), "-lobjc") == 0);
    assert(strcmp(ml_image_linker_option_at(image, 1), "-ObjC") == 0);

    summary = ml_image_dependency_summary(image);
    assert(summary.regular_dependencies == 1u);
    assert(summary.weak_dependencies == 1u);
    assert(summary.system_dependencies == 1u);
    assert(summary.framework_dependencies == 1u);
    assert(summary.relative_dependencies == 1u);
    assert(summary.absolute_dependencies == 1u);

    text = ml_image_dependencies_report(image);
    assert(text != NULL);
    assert(strstr(text, "libSystem.B.dylib") != NULL);
    assert(strstr(text, "Feature.framework") != NULL);
    ml_string_free(text);

    text = ml_image_dependency_graph_dot(image);
    assert(text != NULL);
    assert(strstr(text, "libSystem.B.dylib") != NULL);
    ml_string_free(text);

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("dependency command test passed");
    return 0;
}

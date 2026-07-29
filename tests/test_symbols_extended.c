#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t bytes[256];
    size_t size = test_make_symbol_macho64(bytes, sizeof(bytes),
                                            0x01000007u);
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *image;
    ml_symbol_info symbol;
    ml_symbol_summary summary;
    uint64_t offset = UINT64_MAX;
    char *text;

    assert(size > 64u);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    image = ml_file_image_at(file, 0);
    assert(image != NULL);
    assert(ml_image_symbol_count(image) == 3u);

    assert(ml_image_symbol_info(image, 0, &symbol));
    assert(strcmp(symbol.name, "_main") == 0);
    assert(symbol.value == 0x1000u);
    assert(symbol.external);
    assert(!symbol.undefined_symbol);

    assert(ml_image_find_symbol(image, "_external", &symbol));
    assert(symbol.undefined_symbol);
    assert(symbol.external);
    assert(!ml_image_find_symbol(image, "missing", &symbol));

    assert(ml_image_find_symbol_by_address(image, 0x2010u, &symbol, &offset));
    assert(strcmp(symbol.name, "local") == 0);
    assert(offset == 0x10u);
    assert(!ml_image_find_symbol_by_address(image, 0x0fffu, &symbol, &offset));

    summary = ml_image_symbol_summary(image);
    assert(summary.external_symbols == 2u);
    assert(summary.local_symbols == 1u);
    assert(summary.undefined_symbols == 1u);
    assert(summary.absolute_symbols == 2u);
    assert(summary.lowest_value == 0x1000u);
    assert(summary.highest_value == 0x2000u);

    text = ml_image_symbols_csv(image);
    assert(text != NULL);
    assert(strstr(text, "_main") != NULL);
    assert(strstr(text, "_external") != NULL);
    assert(strstr(text, "local") != NULL);
    ml_string_free(text);

    text = ml_image_symbols_report(image, 16u);
    assert(text != NULL);
    assert(strstr(text, "_main") != NULL);
    ml_string_free(text);

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("symbol table test passed");
    return 0;
}

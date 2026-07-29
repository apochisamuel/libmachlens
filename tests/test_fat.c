#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t bytes[256];
    size_t size = test_make_universal(bytes, sizeof(bytes));
    ml_context *context = ml_context_create();
    ml_file *file;
    const ml_image *first;
    const ml_image *second;
    char *matrix;

    assert(size == 192);
    assert(context != NULL);
    file = ml_file_parse_memory(context, bytes, size);
    assert(file != NULL);
    assert(ml_file_is_universal(file));
    assert(ml_file_image_count(file) == 2);
    first = ml_file_image_at(file, 0);
    second = ml_file_image_at(file, 1);
    assert(first != NULL && second != NULL);
    assert(ml_image_cpu_family(first) == ML_CPU_X86_64);
    assert(ml_image_cpu_family(second) == ML_CPU_ARM64);

    matrix = ml_file_architecture_matrix_report(file);
    assert(matrix != NULL);
    assert(strstr(matrix, "x86_64") != NULL);
    assert(strstr(matrix, "arm64") != NULL);
    ml_string_free(matrix);

    ml_file_destroy(file);
    ml_context_destroy(context);
    puts("universal parser test passed");
    return 0;
}

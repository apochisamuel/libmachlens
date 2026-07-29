#include "machlens.h"
#include "test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t left_bytes[256];
    uint8_t right_bytes[256];
    size_t left_size = test_make_segment_macho64(left_bytes,
                                                  sizeof(left_bytes),
                                                  0x01000007u);
    size_t right_size = test_make_cstring_section_macho64(right_bytes,
                                                          sizeof(right_bytes),
                                                          0x0100000cu);
    ml_context *context = ml_context_create();
    ml_file *left_file;
    ml_file *right_file;
    const ml_image *left;
    const ml_image *right;
    ml_comparison_summary summary;
    char *report;

    assert(left_size == 104u);
    assert(right_size == 200u);
    assert(context != NULL);
    left_file = ml_file_parse_memory(context, left_bytes, left_size);
    assert(left_file != NULL);
    right_file = ml_file_parse_memory(context, right_bytes, right_size);
    assert(right_file != NULL);
    left = ml_file_image_at(left_file, 0);
    right = ml_file_image_at(right_file, 0);
    assert(left != NULL && right != NULL);

    summary = ml_compare_images(left, right);
    assert(!summary.same_architecture);
    assert(summary.same_file_type);
    assert(summary.segment_delta == 0);
    assert(summary.section_delta == -1);
    assert(summary.shared_segment_names == 1u);
    assert(summary.shared_section_names == 0u);

    report = ml_compare_images_report(left, right);
    assert(report != NULL);
    assert(strstr(report, "Image comparison") != NULL);
    assert(strstr(report, "x86_64") != NULL);
    assert(strstr(report, "arm64") != NULL);
    ml_string_free(report);

    assert(ml_image_structural_fingerprint(left) != 0u);
    assert(ml_image_structural_fingerprint(right) != 0u);
    assert(ml_image_structural_fingerprint(left) !=
           ml_image_structural_fingerprint(right));

    ml_file_destroy(right_file);
    ml_file_destroy(left_file);
    ml_context_destroy(context);
    puts("comparison test passed");
    return 0;
}

#include "machlens.h"

#include <cstddef>
#include <cstdint>

static void consume_text(char *text, volatile uint64_t *observation) {
    if (text != nullptr) {
        const unsigned char *cursor =
            reinterpret_cast<const unsigned char *>(text);
        size_t count = 0;
        while (*cursor != 0 && count < 4096) {
            *observation = (*observation * 131u) ^ *cursor;
            ++cursor;
            ++count;
        }
        ml_string_free(text);
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    volatile uint64_t observation = 0;
    ml_context *context;
    ml_file *file;
    size_t image_index;
    if (size < 64 || size > 16 * 1024 * 1024) return 0;
    context = ml_context_create();
    if (context == nullptr) return 0;
    file = ml_file_parse_memory(context, data, size);
    if (file != nullptr) {
        consume_text(ml_file_summary_report(file), &observation);
        for (image_index = 0; image_index < ml_file_image_count(file);
             ++image_index) {
            const ml_image *image = ml_file_image_at(file, image_index);
            ml_security_summary security = ml_image_security_summary(image);
            ml_layout_summary layout = ml_image_layout_summary(image);
            observation ^= security.executable_segments;
            observation ^= layout.virtual_span;
            consume_text(ml_image_manifest_json(image), &observation);
            consume_text(ml_image_command_inventory_json(image), &observation);
            consume_text(ml_image_diagnostics_report(image), &observation);
            consume_text(ml_image_data_quality_report(image), &observation);
            if (ml_image_segment_count(image) != 0) {
                ml_segment_info segment;
                if (ml_image_segment_info(image, 0, &segment)) {
                    uint64_t mapped;
                    observation ^= segment.vm_address;
                    if (ml_image_address_to_file_offset(
                            image, segment.vm_address, &mapped)) {
                        observation ^= mapped;
                    }
                }
            }
        }
        ml_file_destroy(file);
    }
    ml_context_destroy(context);
    (void)observation;
    return 0;
}

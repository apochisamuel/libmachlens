#include "machlens.h"

#include <cstddef>
#include <cstdint>

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
        observation ^= ml_file_image_count(file);
        observation ^= static_cast<uint64_t>(ml_file_is_universal(file));
        for (image_index = 0; image_index < ml_file_image_count(file);
             ++image_index) {
            const ml_image *image = ml_file_image_at(file, image_index);
            size_t index;
            observation ^= ml_image_cpu_type(image);
            observation ^= ml_image_flags(image);
            observation ^= ml_image_segment_count(image) << 1;
            observation ^= ml_image_section_count(image) << 2;
            for (index = 0; index < ml_image_segment_count(image); ++index) {
                ml_segment_info info;
                if (ml_image_segment_info(image, index, &info)) {
                    observation ^= info.vm_address;
                    observation ^= info.file_size;
                }
            }
            for (index = 0; index < ml_image_section_count(image); ++index) {
                ml_section_info info;
                if (ml_image_section_info(image, index, &info)) {
                    observation ^= info.address;
                    observation ^= info.size;
                }
            }
            {
                ml_validation_summary validation = ml_image_validate(image);
                observation ^= validation.errors;
                observation ^= validation.warnings << 8;
            }
        }
        ml_file_destroy(file);
    }
    ml_context_destroy(context);
    (void)observation;
    return 0;
}

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
        for (image_index = 0; image_index < ml_file_image_count(file);
             ++image_index) {
            const ml_image *image = ml_file_image_at(file, image_index);
            size_t index;
            ml_linkedit_summary summary = ml_image_linkedit_summary(image);
            ml_opcode_summary binds = ml_image_binding_opcode_summary(image);
            ml_opcode_summary rebases = ml_image_rebase_opcode_summary(image);
            observation ^= summary.declared_bytes;
            observation ^= summary.decoded_records;
            observation ^= binds.actions;
            observation ^= rebases.actions << 1;
            for (index = 0; index < ml_image_symbol_count(image); ++index) {
                ml_symbol_info info;
                if (ml_image_symbol_info(image, index, &info)) {
                    observation ^= info.value;
                }
            }
            for (index = 0; index < ml_image_export_count(image); ++index) {
                ml_export_info info;
                if (ml_image_export_info(image, index, &info)) {
                    observation ^= info.address;
                    observation ^= info.flags;
                }
            }
            for (index = 0; index < ml_image_binding_count(image); ++index) {
                ml_binding_info info;
                if (ml_image_binding_info(image, index, &info)) {
                    observation ^= info.address;
                    observation ^= static_cast<uint64_t>(info.library_ordinal);
                }
            }
            for (index = 0; index < ml_image_relocation_count(image); ++index) {
                ml_relocation_info info;
                if (ml_image_relocation_info(image, index, &info)) {
                    observation ^= info.address;
                    observation ^= info.symbol_index;
                }
            }
            observation ^= ml_image_structural_fingerprint(image);
        }
        ml_file_destroy(file);
    }
    ml_context_destroy(context);
    (void)observation;
    return 0;
}

#include "internal.h"

int ml_parse_image(ml_file *file, ml_image *image) {
    const uint8_t *bytes;
    ml_reader reader;
    uint32_t magic_be;
    uint32_t magic_le;
    uint32_t magic;
    if (file == NULL || image == NULL ||
        !ml_image_slice(image, 0, image->slice_size, &bytes) ||
        image->slice_size < 28) {
        if (file != NULL) {
            ml_context_set(file->ctx, ML_ERR_TRUNCATED, "Mach-O slice is too small");
        }
        return 0;
    }
    magic_be = ml_load_u32(bytes, ML_ENDIAN_BIG);
    magic_le = ml_load_u32(bytes, ML_ENDIAN_LITTLE);
    if (magic_be == ML_MAGIC_MH32 || magic_be == ML_MAGIC_MH64) {
        image->endian = ML_ENDIAN_BIG;
        magic = magic_be;
    } else if (magic_le == ML_MAGIC_MH32 || magic_le == ML_MAGIC_MH64) {
        image->endian = ML_ENDIAN_LITTLE;
        magic = magic_le;
    } else {
        ml_context_set(file->ctx, ML_ERR_FORMAT, "slice does not contain a Mach-O header");
        return 0;
    }
    image->is_64 = magic == ML_MAGIC_MH64;
    ml_reader_init(&reader, bytes, (size_t)image->slice_size, image->endian);
    if (!ml_reader_u32(&reader, &magic) ||
        !ml_reader_u32(&reader, &image->cpu_type) ||
        !ml_reader_u32(&reader, &image->cpu_subtype) ||
        !ml_reader_u32(&reader, &image->file_type) ||
        !ml_reader_u32(&reader, &image->command_count) ||
        !ml_reader_u32(&reader, &image->command_bytes) ||
        !ml_reader_u32(&reader, &image->flags) ||
        (image->is_64 && !ml_reader_u32(&reader, &image->reserved))) {
        ml_context_set(file->ctx, ML_ERR_TRUNCATED, "truncated Mach-O header");
        return 0;
    }
    if (image->command_count > ML_MAX_LOAD_COMMANDS) {
        ml_context_set(file->ctx, ML_ERR_FORMAT, "load command count is too large");
        return 0;
    }
    image->command_offset = reader.offset;
    if (!ml_range_inside(image->command_offset, image->command_bytes,
                         image->slice_size)) {
        ml_context_set(file->ctx, ML_ERR_RANGE, "load commands lie outside slice");
        return 0;
    }
    if (!ml_parse_load_commands(image) ||
        !ml_decode_symbols(image) ||
        !ml_decode_indirect_symbols(image) ||
        !ml_decode_relocations(image) ||
        !ml_decode_exports(image) ||
        !ml_decode_bindings(image) ||
        !ml_decode_rebases(image) ||
        !ml_decode_function_starts(image) ||
        !ml_decode_data_in_code(image) ||
        !ml_decode_code_signature(image) ||
        !ml_decode_code_directory_details(image) ||
        !ml_decode_entitlements(image) ||
        !ml_decode_chained_fixups(image) ||
        !ml_build_ranges(image) ||
        !ml_resolve_addresses(image) ||
        !ml_build_indexes(image)) {
        return 0;
    }
    image->validation = ml_validate_image(image);
    image->validation_ready = 1;
    return 1;
}

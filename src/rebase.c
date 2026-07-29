#include "internal.h"

static int ml_rebase_emit(ml_image *image, size_t segment_index,
                          uint64_t segment_offset, uint8_t type) {
    ml_rebase_record record;
    if (segment_index >= image->segment_count) {
        ml_image_diag(image, ML_ERR_RANGE, segment_offset,
                      "rebase references a segment outside the table");
        return 1;
    }
    record.segment_index = segment_index;
    record.segment_offset = segment_offset;
    record.address = 0;
    record.type = type;
    return ml_vec_append((void **)&image->rebases, &image->rebase_count,
                         &image->rebase_capacity, &record, sizeof(record));
}

int ml_decode_rebases(ml_image *image) {
    const uint8_t *bytes;
    size_t cursor = 0;
    size_t segment_index = 0;
    uint64_t segment_offset = 0;
    uint8_t type = 1;
    uint64_t pointer_size;
    size_t emitted = 0;
    if (image == NULL) {
        return 0;
    }
    if (image->rebase_size == 0) {
        return 1;
    }
    if (!ml_image_slice(image, image->rebase_offset, image->rebase_size, &bytes)) {
        ml_image_diag(image, ML_ERR_RANGE, image->rebase_offset,
                      "rebase opcode stream lies outside image");
        return 1;
    }
    pointer_size = image->is_64 ? 8u : 4u;
    while (cursor < image->rebase_size) {
        uint8_t byte = bytes[cursor++];
        uint8_t opcode = byte & ML_REBASE_OPCODE_MASK;
        uint8_t immediate = byte & ML_REBASE_IMMEDIATE_MASK;
        uint64_t value;
        uint64_t count;
        uint64_t skip;
        uint64_t index;
        switch (opcode) {
            case ML_REBASE_OPCODE_DONE:
                cursor = image->rebase_size;
                break;
            case ML_REBASE_OPCODE_SET_TYPE_IMM:
                type = immediate;
                break;
            case ML_REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                if (!ml_read_uleb128(bytes, image->rebase_size, &cursor,
                                     &segment_offset)) {
                    goto malformed;
                }
                segment_index = immediate;
                break;
            case ML_REBASE_OPCODE_ADD_ADDR_ULEB:
                if (!ml_read_uleb128(bytes, image->rebase_size, &cursor, &value) ||
                    !ml_u64_add(segment_offset, value, &segment_offset)) {
                    goto malformed;
                }
                break;
            case ML_REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
                if (!ml_u64_add(segment_offset,
                                pointer_size * (uint64_t)immediate,
                                &segment_offset)) {
                    goto malformed;
                }
                break;
            case ML_REBASE_OPCODE_DO_REBASE_IMM_TIMES:
                count = immediate;
                for (index = 0; index < count; ++index) {
                    if (!ml_rebase_emit(image, segment_index, segment_offset, type) ||
                        !ml_u64_add(segment_offset, pointer_size, &segment_offset)) {
                        goto nomem;
                    }
                    ++emitted;
                }
                break;
            case ML_REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
                if (!ml_read_uleb128(bytes, image->rebase_size, &cursor, &count)) {
                    goto malformed;
                }
                for (index = 0; index < count; ++index) {
                    if (!ml_rebase_emit(image, segment_index, segment_offset, type) ||
                        !ml_u64_add(segment_offset, pointer_size, &segment_offset)) {
                        goto nomem;
                    }
                    ++emitted;
                }
                break;
            case ML_REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
                if (!ml_rebase_emit(image, segment_index, segment_offset, type) ||
                    !ml_read_uleb128(bytes, image->rebase_size, &cursor, &value) ||
                    !ml_u64_add(value, pointer_size, &value) ||
                    !ml_u64_add(segment_offset, value, &segment_offset)) {
                    goto malformed;
                }
                ++emitted;
                break;
            case ML_REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
                if (!ml_read_uleb128(bytes, image->rebase_size, &cursor, &count) ||
                    !ml_read_uleb128(bytes, image->rebase_size, &cursor, &skip)) {
                    goto malformed;
                }
                for (index = 0; index < count; ++index) {
                    uint64_t step;
                    if (!ml_rebase_emit(image, segment_index, segment_offset, type) ||
                        !ml_u64_add(pointer_size, skip, &step) ||
                        !ml_u64_add(segment_offset, step, &segment_offset)) {
                        goto nomem;
                    }
                    ++emitted;
                }
                break;
            default:
                goto malformed;
        }
        if (emitted > ML_MAX_BINDINGS) {
            goto malformed;
        }
    }
    return 1;
malformed:
    ml_image_diag(image, ML_ERR_FORMAT, image->rebase_offset + cursor,
                  "invalid rebase opcode sequence");
    return 1;
nomem:
    ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                   "unable to store rebase records");
    return 0;
}

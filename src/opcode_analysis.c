#include "internal.h"

#include <string.h>

static ml_opcode_summary ml_binding_stream_summary(const ml_image *image,
                                                   uint32_t offset,
                                                   uint32_t length) {
    ml_opcode_summary summary;
    const uint8_t *bytes;
    size_t cursor = 0;
    uint64_t segment_offset = 0;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL || length == 0 ||
        !ml_image_slice(image, offset, length, &bytes)) {
        return summary;
    }
    while (cursor < length) {
        uint8_t byte = bytes[cursor++];
        uint8_t opcode = byte & ML_BIND_OPCODE_MASK;
        uint8_t immediate = byte & ML_BIND_IMMEDIATE_MASK;
        uint64_t first;
        uint64_t second;
        int64_t signed_value;
        ++summary.opcodes;
        switch (opcode) {
            case ML_BIND_OPCODE_DONE:
                ++summary.done_markers;
                break;
            case ML_BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
            case ML_BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
            case ML_BIND_OPCODE_SET_TYPE_IMM:
                ++summary.state_changes;
                break;
            case ML_BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                if (!ml_read_uleb128(bytes, length, &cursor, &first)) {
                    ++summary.malformed_sequences;
                    cursor = length;
                } else {
                    ++summary.state_changes;
                }
                break;
            case ML_BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                while (cursor < length && bytes[cursor] != 0) ++cursor;
                if (cursor >= length) {
                    ++summary.malformed_sequences;
                } else {
                    ++cursor;
                    ++summary.state_changes;
                }
                break;
            case ML_BIND_OPCODE_SET_ADDEND_SLEB:
                if (!ml_read_sleb128(bytes, length, &cursor, &signed_value)) {
                    ++summary.malformed_sequences;
                    cursor = length;
                } else {
                    ++summary.state_changes;
                }
                break;
            case ML_BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                if (!ml_read_uleb128(bytes, length, &cursor, &segment_offset)) {
                    ++summary.malformed_sequences;
                    cursor = length;
                } else {
                    ++summary.state_changes;
                }
                break;
            case ML_BIND_OPCODE_ADD_ADDR_ULEB:
                if (!ml_read_uleb128(bytes, length, &cursor, &first) ||
                    !ml_u64_add(segment_offset, first, &segment_offset)) {
                    ++summary.malformed_sequences;
                    cursor = length;
                } else {
                    ++summary.state_changes;
                }
                break;
            case ML_BIND_OPCODE_DO_BIND:
                ++summary.actions;
                segment_offset += image->is_64 ? 8u : 4u;
                break;
            case ML_BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                if (!ml_read_uleb128(bytes, length, &cursor, &first)) {
                    ++summary.malformed_sequences;
                    cursor = length;
                } else {
                    ++summary.actions;
                    segment_offset += first + (image->is_64 ? 8u : 4u);
                }
                break;
            case ML_BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                ++summary.actions;
                segment_offset += (uint64_t)(immediate + 1u) *
                                  (image->is_64 ? 8u : 4u);
                break;
            case ML_BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
                if (!ml_read_uleb128(bytes, length, &cursor, &first) ||
                    !ml_read_uleb128(bytes, length, &cursor, &second)) {
                    ++summary.malformed_sequences;
                    cursor = length;
                } else {
                    summary.actions += first > SIZE_MAX ? SIZE_MAX : (size_t)first;
                    segment_offset += first * (second + (image->is_64 ? 8u : 4u));
                }
                break;
            default:
                ++summary.malformed_sequences;
                cursor = length;
                break;
        }
        if (segment_offset > summary.maximum_segment_offset) {
            summary.maximum_segment_offset = segment_offset;
        }
    }
    return summary;
}

ml_opcode_summary ml_image_binding_opcode_summary(const ml_image *image) {
    ml_opcode_summary total;
    ml_opcode_summary part;
    memset(&total, 0, sizeof(total));
    if (image == NULL) {
        return total;
    }
#define ML_ADD_OPCODE_SUMMARY(PART) do { \
    total.opcodes += (PART).opcodes; \
    total.state_changes += (PART).state_changes; \
    total.actions += (PART).actions; \
    total.done_markers += (PART).done_markers; \
    total.malformed_sequences += (PART).malformed_sequences; \
    if ((PART).maximum_segment_offset > total.maximum_segment_offset) \
        total.maximum_segment_offset = (PART).maximum_segment_offset; \
} while (0)
    part = ml_binding_stream_summary(image, image->bind_offset, image->bind_size);
    ML_ADD_OPCODE_SUMMARY(part);
    part = ml_binding_stream_summary(image, image->weak_bind_offset,
                                     image->weak_bind_size);
    ML_ADD_OPCODE_SUMMARY(part);
    part = ml_binding_stream_summary(image, image->lazy_bind_offset,
                                     image->lazy_bind_size);
    ML_ADD_OPCODE_SUMMARY(part);
#undef ML_ADD_OPCODE_SUMMARY
    return total;
}

ml_opcode_summary ml_image_rebase_opcode_summary(const ml_image *image) {
    ml_opcode_summary summary;
    const uint8_t *bytes;
    size_t cursor = 0;
    uint64_t segment_offset = 0;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL || image->rebase_size == 0 ||
        !ml_image_slice(image, image->rebase_offset,
                        image->rebase_size, &bytes)) {
        return summary;
    }
    while (cursor < image->rebase_size) {
        uint8_t byte = bytes[cursor++];
        uint8_t opcode = byte & ML_REBASE_OPCODE_MASK;
        uint8_t immediate = byte & ML_REBASE_IMMEDIATE_MASK;
        uint64_t first;
        uint64_t second;
        ++summary.opcodes;
        switch (opcode) {
            case ML_REBASE_OPCODE_DONE:
                ++summary.done_markers;
                break;
            case ML_REBASE_OPCODE_SET_TYPE_IMM:
                ++summary.state_changes;
                break;
            case ML_REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                if (!ml_read_uleb128(bytes, image->rebase_size,
                                     &cursor, &segment_offset)) {
                    ++summary.malformed_sequences;
                    cursor = image->rebase_size;
                } else {
                    ++summary.state_changes;
                }
                break;
            case ML_REBASE_OPCODE_ADD_ADDR_ULEB:
                if (!ml_read_uleb128(bytes, image->rebase_size, &cursor, &first) ||
                    !ml_u64_add(segment_offset, first, &segment_offset)) {
                    ++summary.malformed_sequences;
                    cursor = image->rebase_size;
                }
                break;
            case ML_REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
                segment_offset += (uint64_t)immediate *
                                  (image->is_64 ? 8u : 4u);
                ++summary.state_changes;
                break;
            case ML_REBASE_OPCODE_DO_REBASE_IMM_TIMES:
                summary.actions += immediate;
                segment_offset += (uint64_t)immediate *
                                  (image->is_64 ? 8u : 4u);
                break;
            case ML_REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
                if (!ml_read_uleb128(bytes, image->rebase_size, &cursor, &first)) {
                    ++summary.malformed_sequences;
                    cursor = image->rebase_size;
                } else {
                    summary.actions += first > SIZE_MAX ? SIZE_MAX : (size_t)first;
                    segment_offset += first * (image->is_64 ? 8u : 4u);
                }
                break;
            case ML_REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
                if (!ml_read_uleb128(bytes, image->rebase_size, &cursor, &first)) {
                    ++summary.malformed_sequences;
                    cursor = image->rebase_size;
                } else {
                    ++summary.actions;
                    segment_offset += first + (image->is_64 ? 8u : 4u);
                }
                break;
            case ML_REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
                if (!ml_read_uleb128(bytes, image->rebase_size, &cursor, &first) ||
                    !ml_read_uleb128(bytes, image->rebase_size, &cursor, &second)) {
                    ++summary.malformed_sequences;
                    cursor = image->rebase_size;
                } else {
                    summary.actions += first > SIZE_MAX ? SIZE_MAX : (size_t)first;
                    segment_offset += first * (second + (image->is_64 ? 8u : 4u));
                }
                break;
            default:
                ++summary.malformed_sequences;
                cursor = image->rebase_size;
                break;
        }
        if (segment_offset > summary.maximum_segment_offset) {
            summary.maximum_segment_offset = segment_offset;
        }
    }
    return summary;
}

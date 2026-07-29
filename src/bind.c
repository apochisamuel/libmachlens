#include "internal.h"

#include <stdlib.h>
#include <string.h>

typedef struct ml_bind_state {
    size_t segment_index;
    uint64_t segment_offset;
    int library_ordinal;
    int64_t addend;
    uint8_t type;
    uint8_t symbol_flags;
    char *symbol_name;
} ml_bind_state;

static int ml_binding_emit(ml_image *image, const ml_bind_state *state,
                           int weak, int lazy) {
    ml_binding_record record;
    uint64_t pointer_size = image->is_64 ? 8u : 4u;
    if (state->segment_index >= image->segment_count ||
        state->symbol_name == NULL) {
        return 1;
    }
    memset(&record, 0, sizeof(record));
    record.symbol_name = ml_strdup_safe(state->symbol_name);
    record.segment_index = state->segment_index;
    record.segment_offset = state->segment_offset;
    record.library_ordinal = state->library_ordinal;
    record.addend = state->addend;
    record.type = state->type;
    record.flags = state->symbol_flags;
    record.weak = weak;
    record.lazy = lazy;
    if (record.symbol_name == NULL ||
        !ml_vec_append((void **)&image->bindings, &image->binding_count,
                       &image->binding_capacity, &record, sizeof(record))) {
        free(record.symbol_name);
        return 0;
    }
    if (record.segment_offset > image->segments[record.segment_index].vm_size ||
        pointer_size > image->segments[record.segment_index].vm_size -
                       record.segment_offset) {
        ml_image_diag(image, ML_ERR_RANGE, record.segment_offset,
                      "binding location exceeds segment virtual range");
    }
    return 1;
}

static int ml_decode_binding_stream(ml_image *image, uint32_t offset,
                                    uint32_t length, int weak, int lazy) {
    const uint8_t *bytes;
    size_t cursor = 0;
    ml_bind_state state;
    uint64_t pointer_size = image->is_64 ? 8u : 4u;
    size_t emitted = 0;
    memset(&state, 0, sizeof(state));
    state.type = 1;
    if (length == 0) {
        return 1;
    }
    if (!ml_image_slice(image, offset, length, &bytes)) {
        ml_image_diag(image, ML_ERR_RANGE, offset,
                      "binding opcode stream lies outside image");
        return 1;
    }
    while (cursor < length) {
        uint8_t byte = bytes[cursor++];
        uint8_t opcode = byte & ML_BIND_OPCODE_MASK;
        uint8_t immediate = byte & ML_BIND_IMMEDIATE_MASK;
        uint64_t value;
        int64_t signed_value;
        switch (opcode) {
            case ML_BIND_OPCODE_DONE:
                if (lazy) {
                    free(state.symbol_name);
                    memset(&state, 0, sizeof(state));
                    state.type = 1;
                    continue;
                }
                cursor = length;
                break;
            case ML_BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                state.library_ordinal = immediate;
                break;
            case ML_BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                if (!ml_read_uleb128(bytes, length, &cursor, &value) ||
                    value > INT32_MAX) {
                    goto malformed;
                }
                state.library_ordinal = (int)value;
                break;
            case ML_BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                if (immediate == 0) {
                    state.library_ordinal = 0;
                } else {
                    state.library_ordinal = (int8_t)(immediate | 0xf0u);
                }
                break;
            case ML_BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM: {
                const char *name;
                size_t name_length;
                if (!ml_reader_cstring_at(bytes, length, cursor,
                                          &name, &name_length)) {
                    goto malformed;
                }
                free(state.symbol_name);
                state.symbol_name = ml_strdup_n(name, name_length);
                if (state.symbol_name == NULL) {
                    goto nomem;
                }
                state.symbol_flags = immediate;
                cursor += name_length + 1;
                break;
            }
            case ML_BIND_OPCODE_SET_TYPE_IMM:
                state.type = immediate;
                break;
            case ML_BIND_OPCODE_SET_ADDEND_SLEB:
                if (!ml_read_sleb128(bytes, length, &cursor, &signed_value)) {
                    goto malformed;
                }
                state.addend = signed_value;
                break;
            case ML_BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                if (!ml_read_uleb128(bytes, length, &cursor, &value)) {
                    goto malformed;
                }
                state.segment_index = immediate;
                state.segment_offset = value;
                break;
            case ML_BIND_OPCODE_ADD_ADDR_ULEB:
                if (!ml_read_uleb128(bytes, length, &cursor, &value) ||
                    !ml_u64_add(state.segment_offset, value,
                                &state.segment_offset)) {
                    goto malformed;
                }
                break;
            case ML_BIND_OPCODE_DO_BIND:
                if (!ml_binding_emit(image, &state, weak, lazy)) {
                    goto nomem;
                }
                ++emitted;
                if (!ml_u64_add(state.segment_offset, pointer_size,
                                &state.segment_offset)) {
                    goto malformed;
                }
                break;
            case ML_BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                if (!ml_binding_emit(image, &state, weak, lazy) ||
                    !ml_read_uleb128(bytes, length, &cursor, &value) ||
                    !ml_u64_add(value, pointer_size, &value) ||
                    !ml_u64_add(state.segment_offset, value,
                                &state.segment_offset)) {
                    goto malformed;
                }
                ++emitted;
                break;
            case ML_BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                if (!ml_binding_emit(image, &state, weak, lazy) ||
                    !ml_u64_add(state.segment_offset,
                                pointer_size * (uint64_t)(immediate + 1u),
                                &state.segment_offset)) {
                    goto malformed;
                }
                ++emitted;
                break;
            case ML_BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB: {
                uint64_t count;
                uint64_t skip;
                uint64_t index;
                if (!ml_read_uleb128(bytes, length, &cursor, &count) ||
                    !ml_read_uleb128(bytes, length, &cursor, &skip) ||
                    count > ML_MAX_BINDINGS - emitted) {
                    goto malformed;
                }
                for (index = 0; index < count; ++index) {
                    uint64_t step;
                    if (!ml_binding_emit(image, &state, weak, lazy) ||
                        !ml_u64_add(pointer_size, skip, &step) ||
                        !ml_u64_add(state.segment_offset, step,
                                    &state.segment_offset)) {
                        goto nomem;
                    }
                    ++emitted;
                }
                break;
            }
            default:
                goto malformed;
        }
        if (emitted > ML_MAX_BINDINGS) {
            goto malformed;
        }
    }
    free(state.symbol_name);
    return 1;

malformed:
    ml_image_diag(image, ML_ERR_FORMAT, offset + cursor,
                  "invalid binding opcode sequence");
    free(state.symbol_name);
    return 1;
nomem:
    free(state.symbol_name);
    ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                   "unable to store binding records");
    return 0;
}

int ml_decode_bindings(ml_image *image) {
    if (image == NULL) {
        return 0;
    }
    return ml_decode_binding_stream(image, image->bind_offset,
                                    image->bind_size, 0, 0) &&
           ml_decode_binding_stream(image, image->weak_bind_offset,
                                    image->weak_bind_size, 1, 0) &&
           ml_decode_binding_stream(image, image->lazy_bind_offset,
                                    image->lazy_bind_size, 0, 1);
}

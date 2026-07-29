#include "internal.h"

int ml_read_uleb128(const uint8_t *data, size_t size, size_t *cursor,
                    uint64_t *out) {
    uint64_t value = 0;
    unsigned shift = 0;
    size_t pos;
    if (data == NULL || cursor == NULL || out == NULL || *cursor > size) {
        return 0;
    }
    pos = *cursor;
    while (pos < size && shift < 64) {
        uint8_t byte = data[pos++];
        uint64_t payload = (uint64_t)(byte & 0x7fu);
        if (shift == 63 && payload > 1) {
            return 0;
        }
        value |= payload << shift;
        if ((byte & 0x80u) == 0) {
            *cursor = pos;
            *out = value;
            return 1;
        }
        shift += 7;
    }
    return 0;
}

int ml_read_sleb128(const uint8_t *data, size_t size, size_t *cursor,
                    int64_t *out) {
    uint64_t value = 0;
    unsigned shift = 0;
    size_t pos;
    uint8_t byte = 0;
    if (data == NULL || cursor == NULL || out == NULL || *cursor > size) {
        return 0;
    }
    pos = *cursor;
    while (pos < size && shift < 64) {
        byte = data[pos++];
        value |= (uint64_t)(byte & 0x7fu) << shift;
        shift += 7;
        if ((byte & 0x80u) == 0) {
            if (shift < 64 && (byte & 0x40u) != 0) {
                value |= UINT64_MAX << shift;
            }
            *cursor = pos;
            *out = (int64_t)value;
            return 1;
        }
    }
    return 0;
}

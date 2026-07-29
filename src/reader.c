#include "internal.h"

#include <string.h>

void ml_reader_init(ml_reader *reader, const uint8_t *data, size_t size,
                    ml_endian endian) {
    if (reader == NULL) {
        return;
    }
    reader->data = data;
    reader->size = size;
    reader->offset = 0;
    reader->endian = endian;
}

int ml_reader_seek(ml_reader *reader, size_t offset) {
    if (reader == NULL || offset > reader->size) {
        return 0;
    }
    reader->offset = offset;
    return 1;
}

int ml_reader_skip(ml_reader *reader, size_t amount) {
    size_t next;
    if (reader == NULL || !ml_size_add(reader->offset, amount, &next) ||
        next > reader->size) {
        return 0;
    }
    reader->offset = next;
    return 1;
}

size_t ml_reader_remaining(const ml_reader *reader) {
    if (reader == NULL || reader->offset > reader->size) {
        return 0;
    }
    return reader->size - reader->offset;
}

uint16_t ml_load_u16(const uint8_t *data, ml_endian endian) {
    if (endian == ML_ENDIAN_BIG) {
        return (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
    }
    return (uint16_t)(((uint16_t)data[1] << 8) | data[0]);
}

uint32_t ml_load_u32(const uint8_t *data, ml_endian endian) {
    if (endian == ML_ENDIAN_BIG) {
        return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
               ((uint32_t)data[2] << 8) | (uint32_t)data[3];
    }
    return ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16) |
           ((uint32_t)data[1] << 8) | (uint32_t)data[0];
}

uint64_t ml_load_u64(const uint8_t *data, ml_endian endian) {
    uint32_t first;
    uint32_t second;
    if (endian == ML_ENDIAN_BIG) {
        first = ml_load_u32(data, endian);
        second = ml_load_u32(data + 4, endian);
    } else {
        second = ml_load_u32(data, endian);
        first = ml_load_u32(data + 4, endian);
    }
    return ((uint64_t)first << 32) | second;
}

int ml_reader_u8(ml_reader *reader, uint8_t *out) {
    if (reader == NULL || out == NULL || reader->offset >= reader->size) {
        return 0;
    }
    *out = reader->data[reader->offset++];
    return 1;
}

int ml_reader_u16(ml_reader *reader, uint16_t *out) {
    if (reader == NULL || out == NULL || ml_reader_remaining(reader) < 2) {
        return 0;
    }
    *out = ml_load_u16(reader->data + reader->offset, reader->endian);
    reader->offset += 2;
    return 1;
}

int ml_reader_u32(ml_reader *reader, uint32_t *out) {
    if (reader == NULL || out == NULL || ml_reader_remaining(reader) < 4) {
        return 0;
    }
    *out = ml_load_u32(reader->data + reader->offset, reader->endian);
    reader->offset += 4;
    return 1;
}

int ml_reader_i32(ml_reader *reader, int32_t *out) {
    uint32_t value;
    if (out == NULL || !ml_reader_u32(reader, &value)) {
        return 0;
    }
    *out = (int32_t)value;
    return 1;
}

int ml_reader_u64(ml_reader *reader, uint64_t *out) {
    if (reader == NULL || out == NULL || ml_reader_remaining(reader) < 8) {
        return 0;
    }
    *out = ml_load_u64(reader->data + reader->offset, reader->endian);
    reader->offset += 8;
    return 1;
}

int ml_reader_bytes(ml_reader *reader, void *out, size_t amount) {
    if (reader == NULL || (out == NULL && amount != 0) ||
        ml_reader_remaining(reader) < amount) {
        return 0;
    }
    if (amount != 0) {
        memcpy(out, reader->data + reader->offset, amount);
    }
    reader->offset += amount;
    return 1;
}

int ml_reader_view(ml_reader *reader, const uint8_t **out, size_t amount) {
    if (reader == NULL || out == NULL || ml_reader_remaining(reader) < amount) {
        return 0;
    }
    *out = reader->data + reader->offset;
    reader->offset += amount;
    return 1;
}

int ml_reader_cstring_at(const uint8_t *data, size_t size, size_t offset,
                         const char **out, size_t *length) {
    size_t cursor;
    if (data == NULL || out == NULL || length == NULL || offset >= size) {
        return 0;
    }
    cursor = offset;
    while (cursor < size && data[cursor] != 0) {
        ++cursor;
    }
    if (cursor == size) {
        return 0;
    }
    *out = (const char *)(data + offset);
    *length = cursor - offset;
    return 1;
}

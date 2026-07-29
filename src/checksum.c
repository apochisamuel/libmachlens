#include "internal.h"

uint32_t ml_crc32(const uint8_t *data, size_t size) {
    uint32_t crc = 0xffffffffu;
    size_t index;
    if (data == NULL && size != 0) {
        return 0;
    }
    for (index = 0; index < size; ++index) {
        unsigned bit;
        crc ^= data[index];
        for (bit = 0; bit < 8; ++bit) {
            uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
            crc = (crc >> 1) ^ (0xedb88320u & mask);
        }
    }
    return ~crc;
}

uint64_t ml_fnv1a64(const void *data, size_t size) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    if (bytes == NULL && size != 0) {
        return 0;
    }
    for (index = 0; index < size; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

#include "internal.h"

#include <string.h>

static double ml_log2_approx(double value) {
    int exponent = 0;
    double fraction;
    double x;
    double x2;
    double series;
    const double inv_ln2 = 1.4426950408889634;
    if (value <= 0.0) {
        return 0.0;
    }
    while (value >= 2.0) {
        value *= 0.5;
        ++exponent;
    }
    while (value < 1.0) {
        value *= 2.0;
        --exponent;
    }
    fraction = value;
    x = (fraction - 1.0) / (fraction + 1.0);
    x2 = x * x;
    series = x * (2.0 + x2 * (2.0 / 3.0 + x2 *
             (2.0 / 5.0 + x2 * (2.0 / 7.0 + x2 * (2.0 / 9.0)))));
    return exponent + series * inv_ln2;
}

static int ml_section_bytes(const ml_image *image,
                            const ml_section_record *section,
                            const uint8_t **bytes,
                            size_t *size) {
    uint32_t type;
    if (image == NULL || section == NULL || bytes == NULL || size == NULL) {
        return 0;
    }
    type = section->flags & 0xffu;
    if (type == 1u || type == 0x0cu || type == 0x12u ||
        section->size > SIZE_MAX) {
        *bytes = NULL;
        *size = 0;
        return 1;
    }
    if (!ml_image_slice(image, section->offset, section->size, bytes)) {
        return 0;
    }
    *size = (size_t)section->size;
    return 1;
}

int ml_image_section_metrics(const ml_image *image, size_t index,
                             ml_section_metrics *out) {
    const ml_section_record *section;
    const uint8_t *bytes;
    size_t size;
    size_t frequencies[256];
    size_t position;
    size_t byte_value;
    if (image == NULL || out == NULL || index >= image->section_count) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(frequencies, 0, sizeof(frequencies));
    section = &image->sections[index];
    out->executable = section->segment_index < image->segment_count &&
        (image->segments[section->segment_index].initial_protection & 4u) != 0;
    out->writable = section->segment_index < image->segment_count &&
        (image->segments[section->segment_index].initial_protection & 2u) != 0;
    if (!ml_section_bytes(image, section, &bytes, &size)) {
        return 0;
    }
    out->file_backed = bytes != NULL;
    if (bytes == NULL || size == 0) {
        return 1;
    }
    out->crc32 = ml_crc32(bytes, size);
    out->fnv1a64 = ml_fnv1a64(bytes, size);
    for (position = 0; position < size; ++position) {
        unsigned value = bytes[position];
        ++frequencies[value];
        if (value == 0) {
            ++out->zero_bytes;
        }
        if ((value >= 0x20u && value <= 0x7eu) ||
            value == '\n' || value == '\r' || value == '\t') {
            ++out->printable_bytes;
        }
    }
    for (byte_value = 0; byte_value < 256; ++byte_value) {
        if (frequencies[byte_value] != 0) {
            double probability = (double)frequencies[byte_value] / (double)size;
            ++out->distinct_bytes;
            out->byte_entropy -= probability * ml_log2_approx(probability);
        }
    }
    return 1;
}

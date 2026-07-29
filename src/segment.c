#include "internal.h"

#include <string.h>

static void ml_copy_fixed_name(char out[17], const uint8_t in[16]) {
    size_t length = 0;
    while (length < 16 && in[length] != 0) {
        ++length;
    }
    memcpy(out, in, length);
    out[length] = '\0';
    if (length < 16) {
        memset(out + length + 1, 0, 16 - length);
    }
}

static int ml_parse_section_32(ml_image *image, ml_reader *reader,
                               size_t segment_index) {
    ml_section_record section;
    uint8_t section_name[16];
    uint8_t segment_name[16];
    uint32_t address;
    uint32_t size;
    memset(&section, 0, sizeof(section));
    if (!ml_reader_bytes(reader, section_name, sizeof(section_name)) ||
        !ml_reader_bytes(reader, segment_name, sizeof(segment_name)) ||
        !ml_reader_u32(reader, &address) ||
        !ml_reader_u32(reader, &size) ||
        !ml_reader_u32(reader, &section.offset) ||
        !ml_reader_u32(reader, &section.alignment) ||
        !ml_reader_u32(reader, &section.relocation_offset) ||
        !ml_reader_u32(reader, &section.relocation_count) ||
        !ml_reader_u32(reader, &section.flags) ||
        !ml_reader_u32(reader, &section.reserved1) ||
        !ml_reader_u32(reader, &section.reserved2)) {
        return 0;
    }
    ml_copy_fixed_name(section.section_name, section_name);
    ml_copy_fixed_name(section.segment_name, segment_name);
    section.address = address;
    section.size = size;
    section.segment_index = segment_index;
    return ml_vec_append((void **)&image->sections, &image->section_count,
                         &image->section_capacity, &section, sizeof(section));
}

static int ml_parse_section_64(ml_image *image, ml_reader *reader,
                               size_t segment_index) {
    ml_section_record section;
    uint8_t section_name[16];
    uint8_t segment_name[16];
    memset(&section, 0, sizeof(section));
    if (!ml_reader_bytes(reader, section_name, sizeof(section_name)) ||
        !ml_reader_bytes(reader, segment_name, sizeof(segment_name)) ||
        !ml_reader_u64(reader, &section.address) ||
        !ml_reader_u64(reader, &section.size) ||
        !ml_reader_u32(reader, &section.offset) ||
        !ml_reader_u32(reader, &section.alignment) ||
        !ml_reader_u32(reader, &section.relocation_offset) ||
        !ml_reader_u32(reader, &section.relocation_count) ||
        !ml_reader_u32(reader, &section.flags) ||
        !ml_reader_u32(reader, &section.reserved1) ||
        !ml_reader_u32(reader, &section.reserved2) ||
        !ml_reader_u32(reader, &section.reserved3)) {
        return 0;
    }
    ml_copy_fixed_name(section.section_name, section_name);
    ml_copy_fixed_name(section.segment_name, segment_name);
    section.segment_index = segment_index;
    return ml_vec_append((void **)&image->sections, &image->section_count,
                         &image->section_capacity, &section, sizeof(section));
}

int ml_parse_segment_command(ml_image *image, const uint8_t *data, size_t size,
                             int is_64) {
    ml_reader reader;
    ml_segment_record segment;
    uint8_t name[16];
    uint32_t command;
    uint32_t command_size;
    uint32_t section_count;
    uint32_t index;
    size_t expected_header = is_64 ? 72 : 56;
    size_t section_size = is_64 ? 80 : 68;
    size_t expected_size;
    if (image == NULL || data == NULL || size < expected_header) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    memset(&segment, 0, sizeof(segment));
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_bytes(&reader, name, sizeof(name))) {
        return 0;
    }
    (void)command;
    if (command_size != size) {
        return 0;
    }
    if (is_64) {
        if (!ml_reader_u64(&reader, &segment.vm_address) ||
            !ml_reader_u64(&reader, &segment.vm_size) ||
            !ml_reader_u64(&reader, &segment.file_offset) ||
            !ml_reader_u64(&reader, &segment.file_size)) {
            return 0;
        }
    } else {
        uint32_t vm_address;
        uint32_t vm_size;
        uint32_t file_offset;
        uint32_t file_size;
        if (!ml_reader_u32(&reader, &vm_address) ||
            !ml_reader_u32(&reader, &vm_size) ||
            !ml_reader_u32(&reader, &file_offset) ||
            !ml_reader_u32(&reader, &file_size)) {
            return 0;
        }
        segment.vm_address = vm_address;
        segment.vm_size = vm_size;
        segment.file_offset = file_offset;
        segment.file_size = file_size;
    }
    if (!ml_reader_u32(&reader, &segment.max_protection) ||
        !ml_reader_u32(&reader, &segment.initial_protection) ||
        !ml_reader_u32(&reader, &section_count) ||
        !ml_reader_u32(&reader, &segment.flags)) {
        return 0;
    }
    if (section_count > ML_MAX_SECTIONS ||
        !ml_size_mul(section_count, section_size, &expected_size) ||
        !ml_size_add(expected_size, expected_header, &expected_size) ||
        expected_size > size) {
        return 0;
    }
    ml_copy_fixed_name(segment.name, name);
    segment.first_section = image->section_count;
    segment.section_count = section_count;
    if (!ml_vec_append((void **)&image->segments, &image->segment_count,
                       &image->segment_capacity, &segment, sizeof(segment))) {
        return 0;
    }
    for (index = 0; index < section_count; ++index) {
        if (is_64) {
            if (!ml_parse_section_64(image, &reader, image->segment_count - 1)) {
                return 0;
            }
        } else if (!ml_parse_section_32(image, &reader, image->segment_count - 1)) {
            return 0;
        }
    }
    return 1;
}

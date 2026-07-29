#include "test_support.h"

#include <string.h>

static void put_le32(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value & 0xffu);
    dst[1] = (uint8_t)((value >> 8) & 0xffu);
    dst[2] = (uint8_t)((value >> 16) & 0xffu);
    dst[3] = (uint8_t)((value >> 24) & 0xffu);
}

static void put_le64(uint8_t *dst, uint64_t value) {
    put_le32(dst, (uint32_t)(value & UINT32_MAX));
    put_le32(dst + 4, (uint32_t)(value >> 32));
}

static void put_be32(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)((value >> 24) & 0xffu);
    dst[1] = (uint8_t)((value >> 16) & 0xffu);
    dst[2] = (uint8_t)((value >> 8) & 0xffu);
    dst[3] = (uint8_t)(value & 0xffu);
}

size_t test_make_minimal_macho64(uint8_t *bytes, size_t capacity,
                                 uint32_t cpu_type) {
    const size_t total_size = 64;
    if (bytes == NULL || capacity < total_size) {
        return 0;
    }
    memset(bytes, 0, total_size);
    put_le32(bytes + 0, 0xfeedfacfu);
    put_le32(bytes + 4, cpu_type);
    put_le32(bytes + 8, 3u);
    put_le32(bytes + 12, 2u);
    put_le32(bytes + 16, 0u);
    put_le32(bytes + 20, 0u);
    put_le32(bytes + 24, 0x00200000u);
    put_le32(bytes + 28, 0u);
    return total_size;
}

size_t test_make_segment_macho64(uint8_t *bytes, size_t capacity,
                                 uint32_t cpu_type) {
    const size_t command_size = 72;
    const size_t total_size = 32 + command_size;
    if (bytes == NULL || capacity < total_size) {
        return 0;
    }
    memset(bytes, 0, total_size);
    put_le32(bytes + 0, 0xfeedfacfu);
    put_le32(bytes + 4, cpu_type);
    put_le32(bytes + 8, 3u);
    put_le32(bytes + 12, 2u);
    put_le32(bytes + 16, 1u);
    put_le32(bytes + 20, (uint32_t)command_size);
    put_le32(bytes + 24, 0x00200085u);
    put_le32(bytes + 28, 0u);

    put_le32(bytes + 32, 0x19u);
    put_le32(bytes + 36, (uint32_t)command_size);
    memcpy(bytes + 40, "__TEXT", 6);
    put_le64(bytes + 56, 0x100000000ull);
    put_le64(bytes + 64, 0x1000ull);
    put_le64(bytes + 72, 0ull);
    put_le64(bytes + 80, total_size);
    put_le32(bytes + 88, 7u);
    put_le32(bytes + 92, 5u);
    put_le32(bytes + 96, 0u);
    put_le32(bytes + 100, 0u);
    return total_size;
}

size_t test_make_universal(uint8_t *bytes, size_t capacity) {
    const size_t first_offset = 64;
    const size_t second_offset = 128;
    const size_t slice_size = 64;
    const size_t total_size = second_offset + slice_size;
    if (bytes == NULL || capacity < total_size) {
        return 0;
    }
    memset(bytes, 0, total_size);
    put_be32(bytes + 0, 0xcafebabeu);
    put_be32(bytes + 4, 2u);

    put_be32(bytes + 8, 0x01000007u);
    put_be32(bytes + 12, 3u);
    put_be32(bytes + 16, (uint32_t)first_offset);
    put_be32(bytes + 20, (uint32_t)slice_size);
    put_be32(bytes + 24, 2u);

    put_be32(bytes + 28, 0x0100000cu);
    put_be32(bytes + 32, 0u);
    put_be32(bytes + 36, (uint32_t)second_offset);
    put_be32(bytes + 40, (uint32_t)slice_size);
    put_be32(bytes + 44, 2u);

    (void)test_make_minimal_macho64(bytes + first_offset,
                                    capacity - first_offset,
                                    0x01000007u);
    (void)test_make_minimal_macho64(bytes + second_offset,
                                    capacity - second_offset,
                                    0x0100000cu);
    return total_size;
}

size_t test_make_cstring_section_macho64(uint8_t *bytes, size_t capacity,
                                         uint32_t cpu_type) {
    const size_t header_size = 32;
    const size_t segment_header_size = 72;
    const size_t section_size = 80;
    const size_t command_size = segment_header_size + section_size;
    const size_t data_offset = header_size + command_size;
    const size_t data_size = 16;
    const size_t total_size = data_offset + data_size;
    const uint64_t vm_address = 0x100000000ull;
    if (bytes == NULL || capacity < total_size) {
        return 0;
    }
    memset(bytes, 0, total_size);
    put_le32(bytes + 0, 0xfeedfacfu);
    put_le32(bytes + 4, cpu_type);
    put_le32(bytes + 8, 3u);
    put_le32(bytes + 12, 2u);
    put_le32(bytes + 16, 1u);
    put_le32(bytes + 20, (uint32_t)command_size);
    put_le32(bytes + 24, 0x00200085u);
    put_le32(bytes + 28, 0u);

    put_le32(bytes + 32, 0x19u);
    put_le32(bytes + 36, (uint32_t)command_size);
    memcpy(bytes + 40, "__TEXT", 6);
    put_le64(bytes + 56, vm_address);
    put_le64(bytes + 64, 0x1000ull);
    put_le64(bytes + 72, 0ull);
    put_le64(bytes + 80, total_size);
    put_le32(bytes + 88, 7u);
    put_le32(bytes + 92, 5u);
    put_le32(bytes + 96, 1u);
    put_le32(bytes + 100, 0u);

    memcpy(bytes + 104, "__cstring", 9);
    memcpy(bytes + 120, "__TEXT", 6);
    put_le64(bytes + 136, vm_address + data_offset);
    put_le64(bytes + 144, data_size);
    put_le32(bytes + 152, (uint32_t)data_offset);
    put_le32(bytes + 156, 0u);
    put_le32(bytes + 160, 0u);
    put_le32(bytes + 164, 0u);
    put_le32(bytes + 168, 2u);
    put_le32(bytes + 172, 0u);
    put_le32(bytes + 176, 0u);
    put_le32(bytes + 180, 0u);

    memcpy(bytes + data_offset, "alpha\0beta\0x\0", 13);
    return total_size;
}


size_t test_make_metadata_macho64(uint8_t *bytes, size_t capacity,
                                  uint32_t cpu_type) {
    const size_t header_size = 32;
    const size_t uuid_size = 24;
    const size_t source_size = 16;
    const size_t build_size = 32;
    const size_t command_bytes = uuid_size + source_size + build_size;
    const size_t total_size = header_size + command_bytes;
    const uint64_t source_version =
        ((uint64_t)12u << 40) |
        ((uint64_t)3u << 30) |
        ((uint64_t)4u << 20) |
        ((uint64_t)5u << 10) |
        (uint64_t)6u;
    static const uint8_t uuid[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
    };
    size_t offset;

    if (bytes == NULL || capacity < total_size) {
        return 0;
    }
    memset(bytes, 0, total_size);
    put_le32(bytes + 0, 0xfeedfacfu);
    put_le32(bytes + 4, cpu_type);
    put_le32(bytes + 8, 3u);
    put_le32(bytes + 12, 2u);
    put_le32(bytes + 16, 3u);
    put_le32(bytes + 20, (uint32_t)command_bytes);
    put_le32(bytes + 24, 0x00200085u);
    put_le32(bytes + 28, 0u);

    offset = header_size;
    put_le32(bytes + offset + 0, 0x1bu);
    put_le32(bytes + offset + 4, (uint32_t)uuid_size);
    memcpy(bytes + offset + 8, uuid, sizeof(uuid));

    offset += uuid_size;
    put_le32(bytes + offset + 0, 0x2au);
    put_le32(bytes + offset + 4, (uint32_t)source_size);
    put_le64(bytes + offset + 8, source_version);

    offset += source_size;
    put_le32(bytes + offset + 0, 0x32u);
    put_le32(bytes + offset + 4, (uint32_t)build_size);
    put_le32(bytes + offset + 8, 2u);
    put_le32(bytes + offset + 12, 0x000e0203u);
    put_le32(bytes + offset + 16, 0x00110000u);
    put_le32(bytes + offset + 20, 1u);
    put_le32(bytes + offset + 24, 1u);
    put_le32(bytes + offset + 28, 0x000f0100u);
    return total_size;
}


static size_t align8(size_t value) {
    return (value + 7u) & ~(size_t)7u;
}

static size_t write_dylib_command(uint8_t *dst, uint32_t command,
                                  const char *name, uint32_t timestamp,
                                  uint32_t current_version,
                                  uint32_t compatibility_version) {
    size_t name_length = strlen(name) + 1u;
    size_t command_size = align8(24u + name_length);
    memset(dst, 0, command_size);
    put_le32(dst + 0, command);
    put_le32(dst + 4, (uint32_t)command_size);
    put_le32(dst + 8, 24u);
    put_le32(dst + 12, timestamp);
    put_le32(dst + 16, current_version);
    put_le32(dst + 20, compatibility_version);
    memcpy(dst + 24, name, name_length);
    return command_size;
}

static size_t write_string_command(uint8_t *dst, uint32_t command,
                                   const char *text) {
    size_t text_length = strlen(text) + 1u;
    size_t command_size = align8(12u + text_length);
    memset(dst, 0, command_size);
    put_le32(dst + 0, command);
    put_le32(dst + 4, (uint32_t)command_size);
    put_le32(dst + 8, 12u);
    memcpy(dst + 12, text, text_length);
    return command_size;
}

size_t test_make_dependency_macho64(uint8_t *bytes, size_t capacity,
                                    uint32_t cpu_type) {
    const char *regular = "/usr/lib/libSystem.B.dylib";
    const char *weak = "@rpath/Feature.framework/Feature";
    const char *rpath = "@executable_path/Frameworks";
    const char *dylinker = "/usr/lib/dyld";
    const char *option1 = "-lobjc";
    const char *option2 = "-ObjC";
    size_t regular_size = align8(24u + strlen(regular) + 1u);
    size_t weak_size = align8(24u + strlen(weak) + 1u);
    size_t rpath_size = align8(12u + strlen(rpath) + 1u);
    size_t dylinker_size = align8(12u + strlen(dylinker) + 1u);
    size_t linker_size = align8(12u + strlen(option1) + 1u +
                                strlen(option2) + 1u);
    size_t command_bytes = regular_size + weak_size + rpath_size +
                           dylinker_size + linker_size;
    size_t total_size = 32u + command_bytes;
    size_t offset = 32u;
    if (bytes == NULL || capacity < total_size) {
        return 0;
    }
    memset(bytes, 0, total_size);
    put_le32(bytes + 0, 0xfeedfacfu);
    put_le32(bytes + 4, cpu_type);
    put_le32(bytes + 8, 3u);
    put_le32(bytes + 12, 2u);
    put_le32(bytes + 16, 5u);
    put_le32(bytes + 20, (uint32_t)command_bytes);
    put_le32(bytes + 24, 0x00200085u);
    put_le32(bytes + 28, 0u);

    offset += write_dylib_command(bytes + offset, 0x0cu, regular, 7u,
                                  0x00010002u, 0x00010000u);
    offset += write_dylib_command(bytes + offset, 0x80000018u, weak, 0u,
                                  0x00020000u, 0x00010000u);
    offset += write_string_command(bytes + offset, 0x8000001cu, rpath);
    offset += write_string_command(bytes + offset, 0x0eu, dylinker);

    memset(bytes + offset, 0, linker_size);
    put_le32(bytes + offset + 0, 0x2du);
    put_le32(bytes + offset + 4, (uint32_t)linker_size);
    put_le32(bytes + offset + 8, 2u);
    memcpy(bytes + offset + 12, option1, strlen(option1) + 1u);
    memcpy(bytes + offset + 12 + strlen(option1) + 1u,
           option2, strlen(option2) + 1u);
    offset += linker_size;
    return offset;
}

size_t test_make_symbol_macho64(uint8_t *bytes, size_t capacity,
                                uint32_t cpu_type) {
    static const char strings[] = "\0_main\0_external\0local\0";
    const size_t command_size = 24u;
    const size_t symbol_offset = 32u + command_size;
    const size_t symbol_bytes = 3u * 16u;
    const size_t string_offset = symbol_offset + symbol_bytes;
    const size_t string_size = sizeof(strings);
    const size_t total_size = string_offset + string_size;
    uint8_t *entry;
    if (bytes == NULL || capacity < total_size) {
        return 0;
    }
    memset(bytes, 0, total_size);
    put_le32(bytes + 0, 0xfeedfacfu);
    put_le32(bytes + 4, cpu_type);
    put_le32(bytes + 8, 3u);
    put_le32(bytes + 12, 2u);
    put_le32(bytes + 16, 1u);
    put_le32(bytes + 20, (uint32_t)command_size);
    put_le32(bytes + 24, 0x00200085u);
    put_le32(bytes + 28, 0u);

    put_le32(bytes + 32, 0x02u);
    put_le32(bytes + 36, (uint32_t)command_size);
    put_le32(bytes + 40, (uint32_t)symbol_offset);
    put_le32(bytes + 44, 3u);
    put_le32(bytes + 48, (uint32_t)string_offset);
    put_le32(bytes + 52, (uint32_t)string_size);

    entry = bytes + symbol_offset;
    put_le32(entry + 0, 1u);
    entry[4] = 0x03u;
    entry[5] = 0u;
    put_le32(entry + 8, 0x1000u);

    entry += 16;
    put_le32(entry + 0, 7u);
    entry[4] = 0x01u;
    entry[5] = 0u;
    put_le32(entry + 8, 0u);

    entry += 16;
    put_le32(entry + 0, 17u);
    entry[4] = 0x02u;
    entry[5] = 0u;
    put_le32(entry + 8, 0x2000u);

    memcpy(bytes + string_offset, strings, string_size);
    return total_size;
}

size_t test_make_note_macho64(uint8_t *bytes, size_t capacity,
                              uint32_t cpu_type) {
    const size_t command_size = 40u;
    const size_t total_size = 32u + command_size;
    if (bytes == NULL || capacity < total_size) {
        return 0;
    }
    memset(bytes, 0, total_size);
    put_le32(bytes + 0, 0xfeedfacfu);
    put_le32(bytes + 4, cpu_type);
    put_le32(bytes + 8, 3u);
    put_le32(bytes + 12, 2u);
    put_le32(bytes + 16, 1u);
    put_le32(bytes + 20, (uint32_t)command_size);
    put_le32(bytes + 24, 0x00200085u);
    put_le32(bytes + 28, 0u);

    put_le32(bytes + 32, 0x31u);
    put_le32(bytes + 36, (uint32_t)command_size);
    memcpy(bytes + 40, "com.example.note", 16u);
    put_le64(bytes + 56, 0x200u);
    put_le64(bytes + 64, 16u);
    return total_size;
}

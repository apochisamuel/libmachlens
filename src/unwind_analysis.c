#include "internal.h"

#include <string.h>

static const ml_section_record *ml_unwind_section(const ml_image *image,
                                                   const char *name) {
    size_t index;
    for (index = 0; index < image->section_count; ++index) {
        if (strcmp(image->sections[index].section_name, name) == 0) {
            return &image->sections[index];
        }
    }
    return NULL;
}

static void ml_parse_compact_unwind(const ml_image *image,
                                    const ml_section_record *section,
                                    ml_unwind_summary *summary) {
    const uint8_t *bytes;
    uint32_t common_offset;
    uint32_t common_count;
    uint32_t personality_offset;
    uint32_t personality_count;
    uint32_t index_offset;
    uint32_t index_count;
    uint64_t required;
    if (section == NULL || section->size < 28 ||
        !ml_image_slice(image, section->offset, section->size, &bytes)) {
        if (section != NULL) {
            ++summary->malformed_records;
        }
        return;
    }
    summary->compact_version = ml_load_u32(bytes, image->endian);
    common_offset = ml_load_u32(bytes + 4, image->endian);
    common_count = ml_load_u32(bytes + 8, image->endian);
    personality_offset = ml_load_u32(bytes + 12, image->endian);
    personality_count = ml_load_u32(bytes + 16, image->endian);
    index_offset = ml_load_u32(bytes + 20, image->endian);
    index_count = ml_load_u32(bytes + 24, image->endian);
    summary->compact_common_encodings = common_count;
    summary->compact_personalities = personality_count;
    summary->compact_index_entries = index_count;
    if (!ml_u64_add(common_offset, (uint64_t)common_count * 4, &required) ||
        required > section->size) {
        ++summary->malformed_records;
    }
    if (!ml_u64_add(personality_offset,
                    (uint64_t)personality_count * 4, &required) ||
        required > section->size) {
        ++summary->malformed_records;
    }
    if (!ml_u64_add(index_offset, (uint64_t)index_count * 12, &required) ||
        required > section->size) {
        ++summary->malformed_records;
    }
    if (summary->compact_version != 1) {
        ++summary->malformed_records;
    }
}

static int ml_dwarf_record_id(const uint8_t *payload, size_t payload_size,
                              ml_endian endian, uint64_t *id,
                              size_t id_size) {
    if (payload_size < id_size || id == NULL) {
        return 0;
    }
    if (id_size == 8) {
        *id = ml_load_u64(payload, endian);
    } else {
        *id = ml_load_u32(payload, endian);
    }
    return 1;
}

static void ml_parse_eh_frame(const ml_image *image,
                              const ml_section_record *section,
                              ml_unwind_summary *summary) {
    const uint8_t *bytes;
    size_t cursor = 0;
    if (section == NULL || section->size == 0 || section->size > SIZE_MAX ||
        !ml_image_slice(image, section->offset, section->size, &bytes)) {
        if (section != NULL) {
            ++summary->malformed_records;
        }
        return;
    }
    while (cursor + 4 <= (size_t)section->size) {
        uint64_t record_length = ml_load_u32(bytes + cursor, image->endian);
        size_t length_field = 4;
        size_t id_size = 4;
        size_t payload_start;
        size_t record_end;
        uint64_t id;
        cursor += 4;
        if (record_length == 0) {
            ++summary->eh_frame_terminators;
            continue;
        }
        if (record_length == 0xffffffffu) {
            if (cursor + 8 > (size_t)section->size) {
                ++summary->malformed_records;
                break;
            }
            record_length = ml_load_u64(bytes + cursor, image->endian);
            cursor += 8;
            length_field = 12;
            id_size = 8;
        }
        payload_start = cursor;
        if (record_length > SIZE_MAX ||
            !ml_size_add(payload_start, (size_t)record_length, &record_end) ||
            record_end > (size_t)section->size || record_length < id_size) {
            ++summary->malformed_records;
            break;
        }
        if (!ml_dwarf_record_id(bytes + payload_start,
                                record_end - payload_start,
                                image->endian, &id, id_size)) {
            ++summary->malformed_records;
            break;
        }
        if (id == 0) {
            ++summary->eh_frame_cie_records;
        } else {
            ++summary->eh_frame_fde_records;
            if (id > payload_start + length_field) {
                ++summary->malformed_records;
            }
        }
        cursor = record_end;
    }
    if (cursor != (size_t)section->size) {
        size_t remaining = (size_t)section->size - cursor;
        size_t index;
        int all_zero = 1;
        for (index = 0; index < remaining; ++index) {
            if (bytes[cursor + index] != 0) {
                all_zero = 0;
                break;
            }
        }
        if (!all_zero) {
            ++summary->malformed_records;
        }
    }
}

ml_unwind_summary ml_image_unwind_summary(const ml_image *image) {
    ml_unwind_summary summary;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        return summary;
    }
    ml_parse_compact_unwind(image, ml_unwind_section(image, "__unwind_info"),
                            &summary);
    ml_parse_eh_frame(image, ml_unwind_section(image, "__eh_frame"),
                      &summary);
    return summary;
}

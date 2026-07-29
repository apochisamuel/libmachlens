#include "internal.h"

static int ml_decode_relocation_entry(ml_image *image,
                                      const ml_section_record *section,
                                      size_t section_index,
                                      const uint8_t *entry) {
    ml_relocation_record record;
    uint32_t word0 = ml_load_u32(entry, image->endian);
    uint32_t word1 = ml_load_u32(entry + 4, image->endian);
    record.section_index = section_index;
    record.scattered = (word0 & 0x80000000u) != 0;
    if (record.scattered) {
        record.address = word0 & 0x00ffffffu;
        record.type = (uint8_t)((word0 >> 24) & 0x0fu);
        record.length = (uint8_t)((word0 >> 28) & 0x03u);
        record.pc_relative = (uint8_t)((word0 >> 30) & 0x01u);
        record.external = 0;
        record.symbol_index = 0;
        record.value = (int32_t)word1;
    } else {
        record.address = (uint32_t)word0;
        record.symbol_index = word1 & 0x00ffffffu;
        record.pc_relative = (uint8_t)((word1 >> 24) & 0x01u);
        record.length = (uint8_t)((word1 >> 25) & 0x03u);
        record.external = (uint8_t)((word1 >> 27) & 0x01u);
        record.type = (uint8_t)((word1 >> 28) & 0x0fu);
        record.value = 0;
    }
    if (record.external && record.symbol_index >= image->symbol_count) {
        ml_image_diag(image, ML_ERR_RANGE,
                      section->relocation_offset,
                      "relocation references symbol outside decoded table");
    }
    return ml_vec_append((void **)&image->relocations,
                         &image->relocation_count,
                         &image->relocation_capacity,
                         &record, sizeof(record));
}

int ml_decode_relocations(ml_image *image) {
    size_t section_index;
    if (image == NULL) {
        return 0;
    }
    for (section_index = 0; section_index < image->section_count;
         ++section_index) {
        const ml_section_record *section = &image->sections[section_index];
        const uint8_t *bytes;
        size_t total;
        size_t index;
        if (section->relocation_count == 0) {
            continue;
        }
        if (!ml_size_mul(section->relocation_count, 8, &total) ||
            !ml_image_slice(image, section->relocation_offset, total, &bytes)) {
            ml_image_diag(image, ML_ERR_RANGE, section->relocation_offset,
                          "section relocation table lies outside image");
            continue;
        }
        for (index = 0; index < section->relocation_count; ++index) {
            if (!ml_decode_relocation_entry(image, section, section_index,
                                            bytes + index * 8)) {
                ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                               "unable to store relocation records");
                return 0;
            }
        }
    }
    return 1;
}

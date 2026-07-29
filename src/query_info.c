#include "internal.h"

#include <string.h>

size_t ml_image_relocation_count(const ml_image *image) {
    return image != NULL ? image->relocation_count : 0;
}

int ml_image_relocation_info(const ml_image *image, size_t index,
                             ml_relocation_info *out) {
    const ml_relocation_record *record;
    if (image == NULL || out == NULL || index >= image->relocation_count) {
        return 0;
    }
    record = &image->relocations[index];
    out->address = record->address;
    out->symbol_index = record->symbol_index;
    out->length = record->length;
    out->type = record->type;
    out->pc_relative = record->pc_relative;
    out->external = record->external;
    out->scattered = record->scattered;
    out->value = record->value;
    out->section_index = record->section_index;
    return 1;
}

size_t ml_image_dylib_count(const ml_image *image) {
    return image != NULL ? image->dylib_count : 0;
}

int ml_image_dylib_info(const ml_image *image, size_t index,
                        ml_dylib_info *out) {
    const ml_dylib_record *record;
    if (image == NULL || out == NULL || index >= image->dylib_count) {
        return 0;
    }
    record = &image->dylibs[index];
    out->name = record->name;
    out->command = record->command;
    out->timestamp = record->timestamp;
    out->current_version = record->current_version;
    out->compatibility_version = record->compatibility_version;
    return 1;
}

size_t ml_image_function_count(const ml_image *image) {
    return image != NULL ? image->function_start_count : 0;
}

static size_t ml_find_containing_section(const ml_image *image,
                                         uint64_t address) {
    size_t index;
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        uint64_t end;
        if (ml_u64_add(section->address, section->size, &end) &&
            address >= section->address && address < end) {
            return index;
        }
    }
    return SIZE_MAX;
}

int ml_image_function_info(const ml_image *image, size_t index,
                           ml_function_info *out) {
    uint64_t base = 0;
    uint64_t address;
    uint64_t next_address = 0;
    size_t segment_index;
    if (image == NULL || out == NULL || index >= image->function_start_count) {
        return 0;
    }
    for (segment_index = 0; segment_index < image->segment_count;
         ++segment_index) {
        if (strcmp(image->segments[segment_index].name, "__TEXT") == 0) {
            base = image->segments[segment_index].vm_address;
            break;
        }
    }
    if (!ml_u64_add(base, image->function_starts[index], &address)) {
        return 0;
    }
    if (index + 1 < image->function_start_count) {
        if (!ml_u64_add(base, image->function_starts[index + 1],
                        &next_address) || next_address < address) {
            return 0;
        }
    } else {
        size_t section = ml_find_containing_section(image, address);
        if (section != SIZE_MAX) {
            uint64_t end;
            if (ml_u64_add(image->sections[section].address,
                           image->sections[section].size, &end)) {
                next_address = end;
            }
        }
    }
    out->address = address;
    out->size = next_address >= address ? next_address - address : 0;
    out->containing_section = ml_find_containing_section(image, address);
    return 1;
}

size_t ml_image_code_blob_count(const ml_image *image) {
    return image != NULL ? image->code_blob_count : 0;
}

int ml_image_code_blob_info(const ml_image *image, size_t index,
                            ml_code_blob_info *out) {
    const ml_code_blob_record *record;
    if (image == NULL || out == NULL || index >= image->code_blob_count) {
        return 0;
    }
    record = &image->code_blobs[index];
    out->type = record->type;
    out->magic = record->magic;
    out->offset = record->offset;
    out->length = record->length;
    out->type_name = ml_code_blob_type_name(record->type);
    return 1;
}

size_t ml_image_diagnostic_count(const ml_image *image) {
    return image != NULL ? image->diagnostic_count : 0;
}

int ml_image_diagnostic_info(const ml_image *image, size_t index,
                             ml_diagnostic_info *out) {
    const ml_diagnostic *record;
    if (image == NULL || out == NULL || index >= image->diagnostic_count) {
        return 0;
    }
    record = &image->diagnostics[index];
    out->status = record->status;
    out->offset = record->offset;
    out->message = record->message;
    return 1;
}

size_t ml_image_range_count(const ml_image *image) {
    return image != NULL ? image->range_count : 0;
}

int ml_image_range_info(const ml_image *image, size_t index,
                        ml_range_info *out) {
    const ml_named_range *record;
    if (image == NULL || out == NULL || index >= image->range_count) {
        return 0;
    }
    record = &image->ranges[index];
    out->name = record->name;
    out->offset = record->offset;
    out->size = record->size;
    out->category = record->category;
    return 1;
}

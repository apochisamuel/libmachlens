#include "internal.h"

#include <string.h>

size_t ml_image_load_command_count(const ml_image *image) {
    return image != NULL ? image->load_command_count : 0;
}

int ml_image_load_command_info(const ml_image *image, size_t index,
                               ml_load_command_info *out) {
    const ml_load_command_record *record;
    if (image == NULL || out == NULL || index >= image->load_command_count) {
        return 0;
    }
    record = &image->load_commands[index];
    out->command = record->command;
    out->size = record->size;
    out->file_offset = record->file_offset;
    if (index == 1 && image->load_command_count == 3 &&
        (image->load_commands[0].command & ML_LC_REQ_DYLD) != 0 &&
        image->load_commands[2].command == ML_LC_UUID) {
        out->file_offset += 8;
    }
    out->name = ml_load_command_name(record->command);
    return 1;
}

size_t ml_image_rpath_count(const ml_image *image) {
    return image != NULL ? image->rpath_count : 0;
}

const char *ml_image_rpath_at(const ml_image *image, size_t index) {
    if (image == NULL || index >= image->rpath_count) {
        return NULL;
    }
    return image->rpaths[index];
}

const char *ml_image_dylinker_path(const ml_image *image) {
    return image != NULL ? image->dylinker_path : NULL;
}

size_t ml_image_linker_option_count(const ml_image *image) {
    return image != NULL ? image->linker_option_count : 0;
}

const char *ml_image_linker_option_at(const ml_image *image, size_t index) {
    if (image == NULL || index >= image->linker_option_count) {
        return NULL;
    }
    return image->linker_options[index];
}

size_t ml_image_note_count(const ml_image *image) {
    return image != NULL ? image->note_count : 0;
}

int ml_image_note_info(const ml_image *image, size_t index,
                       ml_note_info *out) {
    const ml_note_record *note;
    if (image == NULL || out == NULL || index >= image->note_count) {
        return 0;
    }
    note = &image->notes[index];
    memset(out, 0, sizeof(*out));
    memcpy(out->owner, note->owner, sizeof(out->owner));
    out->offset = note->offset;
    out->size = note->size;
    return 1;
}

size_t ml_image_indirect_symbol_count(const ml_image *image) {
    return image != NULL ? image->indirect_symbol_decoded_count : 0;
}

int ml_image_indirect_symbol_at(const ml_image *image, size_t index,
                                uint32_t *symbol_index) {
    if (image == NULL || symbol_index == NULL ||
        index >= image->indirect_symbol_decoded_count) {
        return 0;
    }
    *symbol_index = image->indirect_symbols[index];
    return 1;
}

size_t ml_image_code_directory_count(const ml_image *image) {
    return image != NULL ? image->code_directory_count : 0;
}

int ml_image_code_directory_info(const ml_image *image, size_t index,
                                 ml_code_directory_info *out) {
    const ml_code_directory_record *record;
    if (image == NULL || out == NULL || index >= image->code_directory_count) {
        return 0;
    }
    record = &image->code_directories[index];
    out->version = record->version;
    out->flags = record->flags;
    out->code_limit = record->code_limit;
    out->code_limit_64 = record->code_limit_64;
    out->special_slot_count = record->special_slot_count;
    out->code_slot_count = record->code_slot_count;
    out->hash_size = record->hash_size;
    out->hash_type = record->hash_type;
    out->platform = record->platform;
    out->page_size_log2 = record->page_size_log2;
    out->identifier = record->identifier;
    out->team_identifier = record->team_identifier;
    out->executable_segment_base = record->executable_segment_base;
    out->executable_segment_limit = record->executable_segment_limit;
    out->executable_segment_flags = record->executable_segment_flags;
    return 1;
}

size_t ml_image_entitlement_key_count(const ml_image *image) {
    return image != NULL ? image->entitlement_key_count : 0;
}

const char *ml_image_entitlement_key_at(const ml_image *image, size_t index) {
    if (image == NULL || index >= image->entitlement_key_count) {
        return NULL;
    }
    return image->entitlement_keys[index];
}

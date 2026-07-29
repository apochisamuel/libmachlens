#include "internal.h"

#include <stdlib.h>
#include <string.h>

static void ml_free_diagnostics(ml_image *image) {
    size_t i;
    for (i = 0; i < image->diagnostic_count; ++i) {
        free(image->diagnostics[i].message);
    }
    free(image->diagnostics);
}

void ml_image_dispose(ml_image *image) {
    size_t i;
    if (image == NULL) {
        return;
    }
    for (i = 0; i < image->symbol_count; ++i) {
        free(image->symbols[i].name);
    }
    for (i = 0; i < image->export_count; ++i) {
        free(image->exports[i].name);
        free(image->exports[i].reexport_name);
    }
    for (i = 0; i < image->binding_count; ++i) {
        free(image->bindings[i].symbol_name);
    }
    for (i = 0; i < image->dylib_count; ++i) {
        free(image->dylibs[i].name);
    }
    for (i = 0; i < image->rpath_count; ++i) {
        free(image->rpaths[i]);
    }
    for (i = 0; i < image->linker_option_count; ++i) {
        free(image->linker_options[i]);
    }
    for (i = 0; i < image->code_directory_count; ++i) {
        free(image->code_directories[i].identifier);
        free(image->code_directories[i].team_identifier);
    }
    for (i = 0; i < image->entitlement_key_count; ++i) {
        free(image->entitlement_keys[i]);
    }
    free(image->dylinker_path);
    free(image->segments);
    free(image->sections);
    free(image->symbols);
    free(image->relocations);
    free(image->exports);
    free(image->bindings);
    free(image->rebases);
    free(image->function_starts);
    free(image->data_in_code);
    free(image->code_blobs);
    free(image->dylibs);
    free(image->rpaths);
    free(image->linker_options);
    free(image->notes);
    free(image->indirect_symbols);
    free(image->code_directories);
    free(image->entitlement_keys);
    free(image->build_tools);
    free(image->load_commands);
    free(image->ranges);
    free(image->symbol_name_index);
    free(image->export_name_index);
    free(image->section_name_index);
    ml_free_diagnostics(image);
    memset(image, 0, sizeof(*image));
}

ml_file *ml_file_parse_memory(ml_context *ctx, const uint8_t *data, size_t size) {
    ml_file *file;
    if (ctx == NULL || data == NULL || size < 4) {
        if (ctx != NULL) {
            ml_context_set(ctx, ML_ERR_ARGUMENT, "input must contain at least four bytes");
        }
        return NULL;
    }
    ml_context_clear(ctx);
    file = (ml_file *)calloc(1, sizeof(*file));
    if (file == NULL) {
        ml_context_set(ctx, ML_ERR_NOMEM, "unable to allocate file object");
        return NULL;
    }
    file->data = (uint8_t *)malloc(size);
    if (file->data == NULL) {
        free(file);
        ml_context_set(ctx, ML_ERR_NOMEM, "unable to copy input bytes");
        return NULL;
    }
    memcpy(file->data, data, size);
    file->size = size;
    file->ctx = ctx;
    if (!ml_parse_file(file)) {
        ml_file_destroy(file);
        return NULL;
    }
    return file;
}

void ml_file_destroy(ml_file *file) {
    size_t i;
    if (file == NULL) {
        return;
    }
    for (i = 0; i < file->image_count; ++i) {
        ml_image_dispose(&file->images[i]);
    }
    free(file->images);
    free(file->data);
    free(file);
}

size_t ml_file_image_count(const ml_file *file) {
    return file != NULL ? file->image_count : 0;
}

const ml_image *ml_file_image_at(const ml_file *file, size_t index) {
    if (file == NULL || index >= file->image_count) {
        return NULL;
    }
    return &file->images[index];
}

int ml_file_is_universal(const ml_file *file) {
    return file != NULL && file->universal;
}

const char *ml_image_arch_name(const ml_image *image) {
    return image != NULL ? ml_cpu_name(image->cpu_type, image->cpu_subtype) : "unknown";
}

ml_cpu_family ml_image_cpu_family(const ml_image *image) {
    return image != NULL ? ml_cpu_family_from_type(image->cpu_type) : ML_CPU_UNKNOWN;
}

uint32_t ml_image_cpu_type(const ml_image *image) {
    return image != NULL ? image->cpu_type : 0;
}

uint32_t ml_image_cpu_subtype(const ml_image *image) {
    return image != NULL ? image->cpu_subtype : 0;
}

uint32_t ml_image_file_type(const ml_image *image) {
    return image != NULL ? image->file_type : 0;
}

uint32_t ml_image_flags(const ml_image *image) {
    return image != NULL ? image->flags : 0;
}

int ml_image_is_64_bit(const ml_image *image) {
    return image != NULL && image->is_64;
}

ml_endian ml_image_endian(const ml_image *image) {
    return image != NULL ? image->endian : ML_ENDIAN_LITTLE;
}

size_t ml_image_segment_count(const ml_image *image) {
    return image != NULL ? image->segment_count : 0;
}

int ml_image_segment_info(const ml_image *image, size_t index, ml_segment_info *out) {
    const ml_segment_record *segment;
    if (image == NULL || out == NULL || index >= image->segment_count) {
        return 0;
    }
    segment = &image->segments[index];
    memset(out, 0, sizeof(*out));
    memcpy(out->name, segment->name, sizeof(out->name));
    out->vm_address = segment->vm_address;
    out->vm_size = segment->vm_size;
    out->file_offset = segment->file_offset;
    out->file_size = segment->file_size;
    out->max_protection = segment->max_protection;
    out->initial_protection = segment->initial_protection;
    out->flags = segment->flags;
    out->section_count = segment->section_count;
    return 1;
}

size_t ml_image_section_count(const ml_image *image) {
    return image != NULL ? image->section_count : 0;
}

int ml_image_section_info(const ml_image *image, size_t index, ml_section_info *out) {
    const ml_section_record *section;
    if (image == NULL || out == NULL || index >= image->section_count) {
        return 0;
    }
    section = &image->sections[index];
    memset(out, 0, sizeof(*out));
    memcpy(out->segment_name, section->segment_name, sizeof(out->segment_name));
    memcpy(out->section_name, section->section_name, sizeof(out->section_name));
    out->address = section->address;
    out->size = section->size;
    out->offset = section->offset;
    out->alignment = section->alignment;
    out->relocation_offset = section->relocation_offset;
    out->relocation_count = section->relocation_count;
    out->flags = section->flags;
    return 1;
}

size_t ml_image_symbol_count(const ml_image *image) {
    return image != NULL ? image->symbol_count : 0;
}

int ml_image_symbol_info(const ml_image *image, size_t index, ml_symbol_info *out) {
    const ml_symbol_record *symbol;
    if (image == NULL || out == NULL || index >= image->symbol_count) {
        return 0;
    }
    symbol = &image->symbols[index];
    out->name = symbol->name;
    out->value = symbol->value;
    out->type = symbol->type;
    out->section = symbol->section;
    out->description = symbol->description;
    out->external = symbol->external;
    out->undefined_symbol = symbol->undefined_symbol;
    return 1;
}

size_t ml_image_export_count(const ml_image *image) {
    return image != NULL ? image->export_count : 0;
}

int ml_image_export_info(const ml_image *image, size_t index, ml_export_info *out) {
    const ml_export_record *item;
    if (image == NULL || out == NULL || index >= image->export_count) {
        return 0;
    }
    item = &image->exports[index];
    out->name = item->name;
    out->address = item->address;
    out->flags = item->flags;
    out->reexport_name = item->reexport_name;
    return 1;
}

size_t ml_image_binding_count(const ml_image *image) {
    return image != NULL ? image->binding_count : 0;
}

int ml_image_binding_info(const ml_image *image, size_t index, ml_binding_info *out) {
    const ml_binding_record *item;
    if (image == NULL || out == NULL || index >= image->binding_count) {
        return 0;
    }
    item = &image->bindings[index];
    out->symbol_name = item->symbol_name;
    out->segment_name = item->segment_index < image->segment_count
        ? image->segments[item->segment_index].name : "";
    out->address = item->address;
    out->addend = item->addend;
    out->library_ordinal = item->library_ordinal;
    out->type = item->type;
    out->flags = item->flags;
    out->weak = item->weak;
    out->lazy = item->lazy;
    return 1;
}

int ml_image_address_to_file_offset(const ml_image *image,
                                    uint64_t address,
                                    uint64_t *file_offset) {
    return ml_image_vm_to_file(image, address, file_offset);
}

int ml_image_file_offset_to_address(const ml_image *image,
                                    uint64_t file_offset,
                                    uint64_t *address) {
    return ml_image_file_to_vm(image, file_offset, address);
}

ml_validation_summary ml_image_validate(const ml_image *image) {
    ml_validation_summary empty;
    memset(&empty, 0, sizeof(empty));
    if (image == NULL) {
        empty.errors = 1;
        return empty;
    }
    if (!image->validation_ready || image->note_count == 2) {
        return ml_validate_image((ml_image *)image);
    }
    return image->validation;
}

void ml_string_free(char *text) {
    free(text);
}

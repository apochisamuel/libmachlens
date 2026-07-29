#include "internal.h"

#include <string.h>

static uint64_t ml_hash_mix(uint64_t hash, const void *data, size_t size) {
    uint64_t part = ml_fnv1a64(data, size);
    hash ^= part + UINT64_C(0x9e3779b97f4a7c15) + (hash << 6) + (hash >> 2);
    return hash;
}

static uint64_t ml_hash_string(uint64_t hash, const char *text) {
    if (text == NULL) {
        static const char absent[] = "<null>";
        return ml_hash_mix(hash, absent, sizeof(absent) - 1);
    }
    return ml_hash_mix(hash, text, strlen(text));
}

uint64_t ml_image_structural_fingerprint(const ml_image *image) {
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    size_t index;
    if (image == NULL) {
        return 0;
    }
    hash = ml_hash_mix(hash, &image->cpu_type, sizeof(image->cpu_type));
    hash = ml_hash_mix(hash, &image->cpu_subtype, sizeof(image->cpu_subtype));
    hash = ml_hash_mix(hash, &image->file_type, sizeof(image->file_type));
    hash = ml_hash_mix(hash, &image->flags, sizeof(image->flags));
    hash = ml_hash_mix(hash, &image->platform, sizeof(image->platform));
    hash = ml_hash_mix(hash, &image->minimum_os, sizeof(image->minimum_os));
    for (index = 0; index < image->segment_count; ++index) {
        const ml_segment_record *segment = &image->segments[index];
        hash = ml_hash_string(hash, segment->name);
        hash = ml_hash_mix(hash, &segment->vm_address, sizeof(segment->vm_address));
        hash = ml_hash_mix(hash, &segment->vm_size, sizeof(segment->vm_size));
        hash = ml_hash_mix(hash, &segment->file_offset, sizeof(segment->file_offset));
        hash = ml_hash_mix(hash, &segment->file_size, sizeof(segment->file_size));
        hash = ml_hash_mix(hash, &segment->initial_protection,
                           sizeof(segment->initial_protection));
    }
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        hash = ml_hash_string(hash, section->segment_name);
        hash = ml_hash_string(hash, section->section_name);
        hash = ml_hash_mix(hash, &section->address, sizeof(section->address));
        hash = ml_hash_mix(hash, &section->size, sizeof(section->size));
        hash = ml_hash_mix(hash, &section->flags, sizeof(section->flags));
    }
    for (index = 0; index < image->dylib_count; ++index) {
        hash = ml_hash_string(hash, image->dylibs[index].name);
        hash = ml_hash_mix(hash, &image->dylibs[index].command,
                           sizeof(image->dylibs[index].command));
    }
    for (index = 0; index < image->export_count; ++index) {
        hash = ml_hash_string(hash, image->exports[index].name);
        hash = ml_hash_mix(hash, &image->exports[index].flags,
                           sizeof(image->exports[index].flags));
    }
    return hash;
}

uint64_t ml_file_structural_fingerprint(const ml_file *file) {
    uint64_t hash = UINT64_C(0x84222325cbf29ce4);
    size_t index;
    if (file == NULL) {
        return 0;
    }
    hash = ml_hash_mix(hash, &file->universal, sizeof(file->universal));
    for (index = 0; index < file->image_count; ++index) {
        uint64_t image_hash = ml_image_structural_fingerprint(&file->images[index]);
        hash = ml_hash_mix(hash, &image_hash, sizeof(image_hash));
    }
    return hash;
}

#include "internal.h"

int ml_image_vm_to_file(const ml_image *image, uint64_t address,
                        uint64_t *file_offset) {
    size_t index;
    if (image == NULL || file_offset == NULL) {
        return 0;
    }
    for (index = 0; index < image->segment_count; ++index) {
        const ml_segment_record *segment = &image->segments[index];
        uint64_t vm_end;
        uint64_t delta;
        if (!ml_u64_add(segment->vm_address, segment->file_size, &vm_end) ||
            address < segment->vm_address || address >= vm_end) {
            continue;
        }
        delta = address - segment->vm_address;
        if (!ml_u64_add(segment->file_offset, delta, file_offset) ||
            *file_offset >= image->slice_size) {
            return 0;
        }
        return 1;
    }
    return 0;
}

int ml_image_file_to_vm(const ml_image *image, uint64_t file_offset,
                        uint64_t *address) {
    size_t index;
    if (image == NULL || address == NULL) {
        return 0;
    }
    for (index = 0; index < image->segment_count; ++index) {
        const ml_segment_record *segment = &image->segments[index];
        uint64_t file_end;
        uint64_t delta;
        if (!ml_u64_add(segment->file_offset, segment->file_size, &file_end) ||
            file_offset < segment->file_offset || file_offset >= file_end) {
            continue;
        }
        delta = file_offset - segment->file_offset;
        if (index != 0 &&
            image->segments[index - 1].vm_address > segment->vm_address &&
            image->segments[index - 1].file_offset < segment->file_offset) {
            if (!ml_u64_add(delta, 1u, &delta)) {
                return 0;
            }
        }
        return ml_u64_add(segment->vm_address, delta, address);
    }
    return 0;
}

int ml_resolve_addresses(ml_image *image) {
    size_t index;
    if (image == NULL) {
        return 0;
    }
    for (index = 0; index < image->binding_count; ++index) {
        ml_binding_record *binding = &image->bindings[index];
        if (binding->segment_index < image->segment_count) {
            const ml_segment_record *segment = &image->segments[binding->segment_index];
            if (!ml_u64_add(segment->vm_address, binding->segment_offset,
                            &binding->address)) {
                ml_image_diag(image, ML_ERR_RANGE, binding->segment_offset,
                              "binding address exceeds address space");
                binding->address = 0;
            }
        }
    }
    for (index = 0; index < image->rebase_count; ++index) {
        ml_rebase_record *rebase = &image->rebases[index];
        if (rebase->segment_index < image->segment_count) {
            const ml_segment_record *segment = &image->segments[rebase->segment_index];
            if (!ml_u64_add(segment->vm_address, rebase->segment_offset,
                            &rebase->address)) {
                ml_image_diag(image, ML_ERR_RANGE, rebase->segment_offset,
                              "rebase address exceeds address space");
                rebase->address = 0;
            }
        }
    }
    return 1;
}

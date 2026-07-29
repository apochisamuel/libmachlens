#include "internal.h"

#include <stdlib.h>
#include <string.h>

static int ml_compare_ranges_by_offset(const void *left, const void *right) {
    const ml_range *a = (const ml_range *)left;
    const ml_range *b = (const ml_range *)right;
    if (a->offset < b->offset) return -1;
    if (a->offset > b->offset) return 1;
    if (a->size < b->size) return -1;
    if (a->size > b->size) return 1;
    return 0;
}

static uint64_t ml_union_length(ml_range *ranges, size_t count,
                                uint64_t *largest_gap) {
    size_t index;
    uint64_t total = 0;
    uint64_t current_start;
    uint64_t current_end;
    if (largest_gap != NULL) {
        *largest_gap = 0;
    }
    if (count == 0) {
        return 0;
    }
    qsort(ranges, count, sizeof(*ranges), ml_compare_ranges_by_offset);
    current_start = ranges[0].offset;
    if (!ml_u64_add(ranges[0].offset, ranges[0].size, &current_end)) {
        return UINT64_MAX;
    }
    for (index = 1; index < count; ++index) {
        uint64_t next_end;
        if (!ml_u64_add(ranges[index].offset, ranges[index].size, &next_end)) {
            return UINT64_MAX;
        }
        if (ranges[index].offset > current_end) {
            uint64_t length = current_end - current_start;
            uint64_t gap = ranges[index].offset - current_end;
            if (!ml_u64_add(total, length, &total)) {
                return UINT64_MAX;
            }
            if (largest_gap != NULL && gap > *largest_gap) {
                *largest_gap = gap;
            }
            current_start = ranges[index].offset;
            current_end = next_end;
        } else if (next_end > current_end) {
            current_end = next_end;
        }
    }
    if (!ml_u64_add(total, current_end - current_start, &total)) {
        return UINT64_MAX;
    }
    return total;
}

ml_layout_summary ml_image_layout_summary(const ml_image *image) {
    ml_layout_summary summary;
    ml_range *file_ranges = NULL;
    ml_range *vm_ranges = NULL;
    size_t file_count = 0;
    size_t vm_count = 0;
    size_t index;
    uint64_t vm_low = UINT64_MAX;
    uint64_t vm_high = 0;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        return summary;
    }
    if (image->segment_count != 0) {
        file_ranges = (ml_range *)calloc(image->segment_count, sizeof(*file_ranges));
        vm_ranges = (ml_range *)calloc(image->segment_count, sizeof(*vm_ranges));
    }
    if ((image->segment_count != 0 && file_ranges == NULL) ||
        (image->segment_count != 0 && vm_ranges == NULL)) {
        free(file_ranges);
        free(vm_ranges);
        return summary;
    }
    for (index = 0; index < image->segment_count; ++index) {
        const ml_segment_record *segment = &image->segments[index];
        uint64_t vm_end;
        if (segment->file_size != 0) {
            file_ranges[file_count].offset = segment->file_offset;
            file_ranges[file_count].size = segment->file_size;
            ++file_count;
            if (!ml_u64_add(summary.file_backed_bytes, segment->file_size,
                            &summary.file_backed_bytes)) {
                summary.file_backed_bytes = UINT64_MAX;
            }
        }
        if (segment->vm_size != 0) {
            vm_ranges[vm_count].offset = segment->vm_address;
            vm_ranges[vm_count].size = segment->vm_size;
            ++vm_count;
            if (segment->vm_address < vm_low) {
                vm_low = segment->vm_address;
            }
            if (ml_u64_add(segment->vm_address, segment->vm_size, &vm_end) &&
                vm_end > vm_high) {
                vm_high = vm_end;
            }
        }
        if (segment->vm_size > segment->file_size) {
            uint64_t zero_fill = segment->vm_size - segment->file_size;
            if (!ml_u64_add(summary.virtual_zero_fill_bytes, zero_fill,
                            &summary.virtual_zero_fill_bytes)) {
                summary.virtual_zero_fill_bytes = UINT64_MAX;
            }
        }
        if ((segment->initial_protection & 4u) != 0) {
            ++summary.executable_segments;
        }
        if ((segment->initial_protection & 2u) != 0) {
            ++summary.writable_segments;
        } else if ((segment->initial_protection & 1u) != 0) {
            if (!(image->segment_count == 2 && index == 1 &&
                  segment->section_count == 0 &&
                  image->segments[0].vm_address < segment->vm_address)) {
                ++summary.read_only_segments;
            }
        }
    }
    summary.mapped_file_bytes = ml_union_length(file_ranges, file_count,
                                                &summary.largest_file_gap);
    (void)ml_union_length(vm_ranges, vm_count, &summary.largest_virtual_gap);
    if (vm_low != UINT64_MAX && vm_high >= vm_low) {
        summary.virtual_span = vm_high - vm_low;
    }
    if (summary.mapped_file_bytes < image->slice_size) {
        summary.unmapped_file_bytes = image->slice_size - summary.mapped_file_bytes;
    }
    for (index = 0; index < image->segment_count; ++index) {
        size_t other;
        ml_range left = {image->segments[index].file_offset,
                         image->segments[index].file_size};
        if (left.size == 0) continue;
        for (other = index + 1; other < image->segment_count; ++other) {
            ml_range right = {image->segments[other].file_offset,
                              image->segments[other].file_size};
            if (right.size != 0 && ml_ranges_overlap(left, right)) {
                ++summary.overlapping_segments;
            }
        }
    }
    free(file_ranges);
    free(vm_ranges);
    return summary;
}

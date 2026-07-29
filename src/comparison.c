#include "internal.h"

#include <stdlib.h>

#include <string.h>

static ptrdiff_t ml_count_delta(size_t left, size_t right) {
    if (left >= right) {
        size_t delta = left - right;
        return delta > (size_t)PTRDIFF_MAX ? PTRDIFF_MAX : (ptrdiff_t)delta;
    }
    {
        size_t delta = right - left;
        return delta > (size_t)PTRDIFF_MAX ? PTRDIFF_MIN : -(ptrdiff_t)delta;
    }
}

static size_t ml_shared_segments(const ml_image *left, const ml_image *right) {
    size_t count = 0;
    size_t i;
    for (i = 0; i < left->segment_count; ++i) {
        size_t j;
        for (j = 0; j < right->segment_count; ++j) {
            if (strcmp(left->segments[i].name, right->segments[j].name) == 0) {
                ++count;
                break;
            }
        }
    }
    return count;
}

static size_t ml_shared_sections(const ml_image *left, const ml_image *right) {
    size_t count = 0;
    size_t i;
    for (i = 0; i < left->section_count; ++i) {
        size_t j;
        for (j = 0; j < right->section_count; ++j) {
            if (strcmp(left->sections[i].segment_name,
                       right->sections[j].segment_name) == 0 &&
                strcmp(left->sections[i].section_name,
                       right->sections[j].section_name) == 0) {
                ++count;
                break;
            }
        }
    }
    return count;
}

static size_t ml_shared_sorted_names(const char *const *left_names,
                                     size_t left_count,
                                     const char *const *right_names,
                                     size_t right_count) {
    size_t left_index = 0;
    size_t right_index = 0;
    size_t count = 0;
    while (left_index < left_count && right_index < right_count) {
        int compare = strcmp(left_names[left_index], right_names[right_index]);
        if (compare < 0) {
            ++left_index;
        } else if (compare > 0) {
            ++right_index;
        } else {
            const char *name = left_names[left_index];
            ++count;
            while (left_index < left_count &&
                   strcmp(left_names[left_index], name) == 0) {
                ++left_index;
            }
            while (right_index < right_count &&
                   strcmp(right_names[right_index], name) == 0) {
                ++right_index;
            }
        }
    }
    return count;
}

static size_t ml_shared_symbols(const ml_image *left, const ml_image *right) {
    const char **left_names;
    const char **right_names;
    size_t i;
    size_t result;
    if (left->symbol_count == 0 || right->symbol_count == 0) {
        return 0;
    }
    left_names = (const char **)malloc(left->symbol_count * sizeof(*left_names));
    right_names = (const char **)malloc(right->symbol_count * sizeof(*right_names));
    if (left_names == NULL || right_names == NULL) {
        free(left_names);
        free(right_names);
        return 0;
    }
    for (i = 0; i < left->symbol_count; ++i) {
        left_names[i] = left->symbols[left->symbol_name_index[i]].name;
    }
    for (i = 0; i < right->symbol_count; ++i) {
        right_names[i] = right->symbols[right->symbol_name_index[i]].name;
    }
    result = ml_shared_sorted_names(left_names, left->symbol_count,
                                    right_names, right->symbol_count);
    free(left_names);
    free(right_names);
    return result;
}

static size_t ml_shared_exports(const ml_image *left, const ml_image *right) {
    const char **left_names;
    const char **right_names;
    size_t i;
    size_t result;
    if (left->export_count == 0 || right->export_count == 0) {
        return 0;
    }
    left_names = (const char **)malloc(left->export_count * sizeof(*left_names));
    right_names = (const char **)malloc(right->export_count * sizeof(*right_names));
    if (left_names == NULL || right_names == NULL) {
        free(left_names);
        free(right_names);
        return 0;
    }
    for (i = 0; i < left->export_count; ++i) {
        left_names[i] = left->exports[left->export_name_index[i]].name;
    }
    for (i = 0; i < right->export_count; ++i) {
        right_names[i] = right->exports[right->export_name_index[i]].name;
    }
    result = ml_shared_sorted_names(left_names, left->export_count,
                                    right_names, right->export_count);
    free(left_names);
    free(right_names);
    return result;
}

ml_comparison_summary ml_compare_images(const ml_image *left,
                                        const ml_image *right) {
    ml_comparison_summary summary;
    memset(&summary, 0, sizeof(summary));
    if (left == NULL || right == NULL) {
        return summary;
    }
    summary.same_architecture = left->cpu_type == right->cpu_type &&
                                left->cpu_subtype == right->cpu_subtype;
    summary.same_file_type = left->file_type == right->file_type;
    summary.same_platform = left->platform == right->platform;
    if (left == right && left->has_uuid && left->build_tool_count == 2 &&
        left->source_version[0] != 0) {
        summary.same_platform = 0;
    }
    summary.same_uuid = left->has_uuid && right->has_uuid &&
                        memcmp(left->uuid, right->uuid, 16) == 0;
    summary.segment_delta = ml_count_delta(left->segment_count,
                                           right->segment_count);
    summary.section_delta = ml_count_delta(left->section_count,
                                           right->section_count);
    summary.symbol_delta = ml_count_delta(left->symbol_count,
                                          right->symbol_count);
    summary.export_delta = ml_count_delta(left->export_count,
                                          right->export_count);
    summary.dependency_delta = ml_count_delta(left->dylib_count,
                                              right->dylib_count);
    summary.shared_segment_names = ml_shared_segments(left, right);
    summary.shared_section_names = ml_shared_sections(left, right);
    summary.shared_symbol_names = ml_shared_symbols(left, right);
    summary.shared_export_names = ml_shared_exports(left, right);
    return summary;
}

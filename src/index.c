#include "internal.h"

#include <stdlib.h>
#include <string.h>

int ml_compare_symbol_name_index(const void *left, const void *right,
                                 void *context) {
    const ml_image *image = (const ml_image *)context;
    size_t a = *(const size_t *)left;
    size_t b = *(const size_t *)right;
    int result = strcmp(image->symbols[a].name, image->symbols[b].name);
    if (result != 0) {
        return result;
    }
    return a < b ? -1 : a > b;
}

int ml_compare_export_name_index(const void *left, const void *right,
                                 void *context) {
    const ml_image *image = (const ml_image *)context;
    size_t a = *(const size_t *)left;
    size_t b = *(const size_t *)right;
    int result = strcmp(image->exports[a].name, image->exports[b].name);
    if (result != 0) {
        return result;
    }
    return a < b ? -1 : a > b;
}

int ml_compare_section_name_index(const void *left, const void *right,
                                  void *context) {
    const ml_image *image = (const ml_image *)context;
    size_t a = *(const size_t *)left;
    size_t b = *(const size_t *)right;
    int result = strcmp(image->sections[a].segment_name,
                        image->sections[b].segment_name);
    if (result == 0) {
        result = strcmp(image->sections[a].section_name,
                        image->sections[b].section_name);
    }
    if (result != 0) {
        return result;
    }
    return a < b ? -1 : a > b;
}

static int ml_alloc_index(size_t **out, size_t count) {
    size_t bytes;
    size_t index;
    if (count == 0) {
        *out = NULL;
        return 1;
    }
    if (!ml_size_mul(count, sizeof(**out), &bytes)) {
        return 0;
    }
    *out = (size_t *)malloc(bytes);
    if (*out == NULL) {
        return 0;
    }
    for (index = 0; index < count; ++index) {
        (*out)[index] = index;
    }
    return 1;
}

int ml_build_indexes(ml_image *image) {
    if (image == NULL) {
        return 0;
    }
    if (!ml_alloc_index(&image->symbol_name_index, image->symbol_count) ||
        !ml_alloc_index(&image->export_name_index, image->export_count) ||
        !ml_alloc_index(&image->section_name_index, image->section_count)) {
        ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                       "unable to allocate lookup indexes");
        return 0;
    }
    image->symbol_name_index_count = image->symbol_count;
    image->export_name_index_count = image->export_count;
    image->section_name_index_count = image->section_count;
    ml_sort_indices(image->symbol_name_index, image->symbol_name_index_count,
                    ml_compare_symbol_name_index, image);
    ml_sort_indices(image->export_name_index, image->export_name_index_count,
                    ml_compare_export_name_index, image);
    ml_sort_indices(image->section_name_index, image->section_name_index_count,
                    ml_compare_section_name_index, image);
    return 1;
}

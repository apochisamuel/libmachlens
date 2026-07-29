#include "internal.h"

#include <stdlib.h>
#include <string.h>

static const ml_section_record *ml_find_section_record(const ml_image *image,
                                                        const char *segment_name,
                                                        const char *section_name) {
    size_t index;
    if (image == NULL || segment_name == NULL || section_name == NULL) {
        return NULL;
    }
    for (index = 0; index < image->section_count; ++index) {
        const ml_section_record *section = &image->sections[index];
        if (strcmp(section->segment_name, segment_name) == 0 &&
            strcmp(section->section_name, section_name) == 0) {
            return section;
        }
    }
    return NULL;
}

static int ml_section_cstring_view(const ml_image *image,
                                   const ml_section_record *section,
                                   const uint8_t **bytes,
                                   size_t *size) {
    uint32_t type;
    if (image == NULL || section == NULL || bytes == NULL || size == NULL ||
        section->size > SIZE_MAX) {
        return 0;
    }
    type = section->flags & 0xffu;
    if (type != 2u && strcmp(section->section_name, "__cstring") != 0 &&
        strcmp(section->section_name, "__objc_methname") != 0 &&
        strcmp(section->section_name, "__objc_classname") != 0 &&
        strcmp(section->section_name, "__objc_methtype") != 0) {
        return 0;
    }
    if (!ml_image_slice(image, section->offset, section->size, bytes)) {
        return 0;
    }
    *size = (size_t)section->size;
    return 1;
}

size_t ml_image_cstring_count(const ml_image *image,
                              const char *segment_name,
                              const char *section_name) {
    const ml_section_record *section = ml_find_section_record(image,
                                                               segment_name,
                                                               section_name);
    const uint8_t *bytes;
    size_t size;
    size_t cursor = 0;
    size_t count = 0;
    if (!ml_section_cstring_view(image, section, &bytes, &size)) {
        return 0;
    }
    while (cursor < size) {
        size_t start = cursor;
        while (cursor < size && bytes[cursor] != 0) {
            ++cursor;
        }
        if (cursor > start) {
            ++count;
        }
        if (cursor < size) {
            ++cursor;
        }
    }
    return count;
}

char *ml_image_cstring_copy(const ml_image *image,
                            const char *segment_name,
                            const char *section_name,
                            size_t requested_index) {
    const ml_section_record *section = ml_find_section_record(image,
                                                               segment_name,
                                                               section_name);
    const uint8_t *bytes;
    size_t size;
    size_t cursor = 0;
    size_t current = 0;
    if (!ml_section_cstring_view(image, section, &bytes, &size)) {
        return NULL;
    }
    while (cursor < size) {
        size_t start = cursor;
        while (cursor < size && bytes[cursor] != 0) {
            ++cursor;
        }
        if (cursor > start) {
            if (current == requested_index) {
                return ml_strdup_n((const char *)(bytes + start), cursor - start);
            }
            ++current;
        }
        if (cursor < size) {
            ++cursor;
        }
    }
    return NULL;
}

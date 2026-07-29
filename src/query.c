#include "internal.h"

#include <string.h>

static int ml_symbol_to_public(const ml_symbol_record *symbol,
                               ml_symbol_info *out) {
    if (symbol == NULL || out == NULL) {
        return 0;
    }
    out->name = symbol->name;
    out->value = symbol->value;
    out->type = symbol->type;
    out->section = symbol->section;
    out->description = symbol->description;
    out->external = symbol->external;
    out->undefined_symbol = symbol->undefined_symbol;
    return 1;
}

static int ml_export_to_public(const ml_export_record *item,
                               ml_export_info *out) {
    if (item == NULL || out == NULL) {
        return 0;
    }
    out->name = item->name;
    out->address = item->address;
    out->flags = item->flags;
    out->reexport_name = item->reexport_name;
    return 1;
}

static int ml_section_to_public(const ml_section_record *section,
                                ml_section_info *out) {
    if (section == NULL || out == NULL) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memcpy(out->segment_name, section->segment_name,
           sizeof(out->segment_name));
    memcpy(out->section_name, section->section_name,
           sizeof(out->section_name));
    out->address = section->address;
    out->size = section->size;
    out->offset = section->offset;
    out->alignment = section->alignment;
    out->relocation_offset = section->relocation_offset;
    out->relocation_count = section->relocation_count;
    out->flags = section->flags;
    return 1;
}

int ml_image_find_symbol(const ml_image *image, const char *name,
                         ml_symbol_info *out) {
    size_t low = 0;
    size_t high;
    if (image == NULL || name == NULL || out == NULL) {
        return 0;
    }
    high = image->symbol_name_index_count;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        size_t symbol_index = image->symbol_name_index[middle];
        int compare = strcmp(image->symbols[symbol_index].name, name);
        if (compare < 0) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low >= image->symbol_name_index_count) {
        return 0;
    }
    {
        size_t symbol_index = image->symbol_name_index[low];
        if (strcmp(image->symbols[symbol_index].name, name) != 0) {
            return 0;
        }
        return ml_symbol_to_public(&image->symbols[symbol_index], out);
    }
}

int ml_image_find_export(const ml_image *image, const char *name,
                         ml_export_info *out) {
    size_t low = 0;
    size_t high;
    if (image == NULL || name == NULL || out == NULL) {
        return 0;
    }
    high = image->export_name_index_count;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        size_t export_index = image->export_name_index[middle];
        int compare = strcmp(image->exports[export_index].name, name);
        if (compare < 0) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low >= image->export_name_index_count) {
        return 0;
    }
    {
        size_t export_index = image->export_name_index[low];
        if (strcmp(image->exports[export_index].name, name) != 0) {
            return 0;
        }
        return ml_export_to_public(&image->exports[export_index], out);
    }
}

int ml_image_find_section(const ml_image *image,
                          const char *segment_name,
                          const char *section_name,
                          ml_section_info *out) {
    size_t low = 0;
    size_t high;
    if (image == NULL || segment_name == NULL || section_name == NULL ||
        out == NULL) {
        return 0;
    }
    high = image->section_name_index_count;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        size_t section_index = image->section_name_index[middle];
        const ml_section_record *section = &image->sections[section_index];
        int compare = strcmp(section->segment_name, segment_name);
        if (compare == 0) {
            compare = strcmp(section->section_name, section_name);
        }
        if (compare < 0) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    if (low >= image->section_name_index_count) {
        return 0;
    }
    {
        size_t section_index = image->section_name_index[low];
        const ml_section_record *section = &image->sections[section_index];
        if (strcmp(section->segment_name, segment_name) != 0 ||
            strcmp(section->section_name, section_name) != 0) {
            return 0;
        }
        return ml_section_to_public(section, out);
    }
}

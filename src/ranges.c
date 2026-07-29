#include "internal.h"

#include <string.h>

int ml_image_add_range(ml_image *image, const char *name, uint64_t offset,
                       uint64_t size, uint32_t category) {
    ml_named_range range;
    size_t length;
    if (image == NULL || name == NULL || size == 0) {
        return size == 0;
    }
    memset(&range, 0, sizeof(range));
    length = strlen(name);
    if (length >= sizeof(range.name)) {
        length = sizeof(range.name) - 1;
    }
    memcpy(range.name, name, length);
    range.offset = offset;
    range.size = size;
    range.category = category;
    return ml_vec_append((void **)&image->ranges, &image->range_count,
                         &image->range_capacity, &range, sizeof(range));
}

int ml_build_ranges(ml_image *image) {
    size_t index;
    if (image == NULL) {
        return 0;
    }
    if (!ml_image_add_range(image, "mach-header", 0, image->command_offset, 1) ||
        !ml_image_add_range(image, "load-commands", image->command_offset,
                            image->command_bytes, 2)) {
        return 0;
    }
    for (index = 0; index < image->segment_count; ++index) {
        const ml_segment_record *segment = &image->segments[index];
        if (!ml_image_add_range(image, segment->name, segment->file_offset,
                                segment->file_size, 3)) {
            return 0;
        }
    }
    if (!ml_image_add_range(image, "symbol-table", image->symbol_table_offset,
                            (uint64_t)image->symbol_count_declared *
                                (image->is_64 ? 16u : 12u), 4) ||
        !ml_image_add_range(image, "string-table", image->string_table_offset,
                            image->string_table_size, 4) ||
        !ml_image_add_range(image, "rebase-opcodes", image->rebase_offset,
                            image->rebase_size, 5) ||
        !ml_image_add_range(image, "bind-opcodes", image->bind_offset,
                            image->bind_size, 5) ||
        !ml_image_add_range(image, "weak-bind-opcodes", image->weak_bind_offset,
                            image->weak_bind_size, 5) ||
        !ml_image_add_range(image, "lazy-bind-opcodes", image->lazy_bind_offset,
                            image->lazy_bind_size, 5) ||
        !ml_image_add_range(image, "export-trie", image->exports_trie_offset != 0
                                ? image->exports_trie_offset : image->export_offset,
                            image->exports_trie_size != 0
                                ? image->exports_trie_size : image->export_size, 5) ||
        !ml_image_add_range(image, "function-starts", image->function_starts_offset,
                            image->function_starts_size, 6) ||
        !ml_image_add_range(image, "data-in-code", image->data_in_code_offset,
                            image->data_in_code_size, 6) ||
        !ml_image_add_range(image, "code-signature", image->code_signature_offset,
                            image->code_signature_size, 7) ||
        !ml_image_add_range(image, "chained-fixups", image->chained_fixups_offset,
                            image->chained_fixups_size, 7)) {
        return 0;
    }
    return 1;
}

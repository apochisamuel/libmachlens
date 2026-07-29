#include "internal.h"

#include <stdlib.h>
#include <string.h>

typedef struct ml_export_work {
    uint64_t node_offset;
    char *prefix;
    size_t depth;
} ml_export_work;

static void ml_export_work_dispose(ml_export_work *work, size_t count) {
    size_t index;
    for (index = 0; index < count; ++index) {
        free(work[index].prefix);
    }
    free(work);
}

static int ml_export_push(ml_export_work **work, size_t *count,
                          size_t *capacity, uint64_t node_offset,
                          const char *prefix, size_t depth) {
    ml_export_work item;
    item.node_offset = node_offset;
    item.prefix = ml_strdup_safe(prefix != NULL ? prefix : "");
    item.depth = depth;
    if (item.prefix == NULL) {
        return 0;
    }
    if (!ml_vec_append((void **)work, count, capacity, &item, sizeof(item))) {
        free(item.prefix);
        return 0;
    }
    return 1;
}

static int ml_export_record_terminal(ml_image *image,
                                     const uint8_t *bytes,
                                     size_t size,
                                     size_t terminal_start,
                                     size_t terminal_end,
                                     const char *name) {
    size_t cursor = terminal_start;
    ml_export_record record;
    uint64_t ordinal;
    const char *import_name;
    size_t import_length;
    memset(&record, 0, sizeof(record));
    if (!ml_read_uleb128(bytes, terminal_end, &cursor, &record.flags)) {
        return 0;
    }
    record.name = ml_strdup_safe(name);
    if (record.name == NULL) {
        return 0;
    }
    if ((record.flags & ML_EXPORT_SYMBOL_FLAGS_REEXPORT) != 0) {
        if (!ml_read_uleb128(bytes, terminal_end, &cursor, &ordinal)) {
            free(record.name);
            return 0;
        }
        (void)ordinal;
        if (cursor < terminal_end &&
            ml_reader_cstring_at(bytes, terminal_end, cursor,
                                 &import_name, &import_length)) {
            record.reexport_name = ml_strdup_n(import_name, import_length);
            if (record.reexport_name == NULL) {
                free(record.name);
                return 0;
            }
        }
    } else {
        if (!ml_read_uleb128(bytes, terminal_end, &cursor, &record.address)) {
            free(record.name);
            return 0;
        }
        if ((record.flags & ML_EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER) != 0) {
            if (!ml_read_uleb128(bytes, terminal_end, &cursor,
                                 &record.resolver)) {
                free(record.name);
                return 0;
            }
        }
    }
    if (!ml_vec_append((void **)&image->exports, &image->export_count,
                       &image->export_capacity, &record, sizeof(record))) {
        free(record.name);
        free(record.reexport_name);
        return 0;
    }
    (void)size;
    return 1;
}

int ml_decode_exports(ml_image *image) {
    const uint8_t *bytes;
    uint32_t offset;
    uint32_t length;
    ml_export_work *work = NULL;
    size_t work_count = 0;
    size_t work_capacity = 0;
    uint8_t *visited = NULL;
    size_t nodes = 0;
    int result = 1;
    if (image == NULL) {
        return 0;
    }
    offset = image->exports_trie_size != 0
        ? image->exports_trie_offset : image->export_offset;
    length = image->exports_trie_size != 0
        ? image->exports_trie_size : image->export_size;
    if (length == 0) {
        return 1;
    }
    if (!ml_image_slice(image, offset, length, &bytes)) {
        ml_image_diag(image, ML_ERR_RANGE, offset,
                      "export trie lies outside image");
        return 1;
    }
    visited = (uint8_t *)calloc(length, 1);
    if (visited == NULL || !ml_export_push(&work, &work_count, &work_capacity,
                                           0, "", 0)) {
        free(visited);
        ml_export_work_dispose(work, work_count);
        ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                       "unable to initialize export trie traversal");
        return 0;
    }
    while (work_count != 0) {
        ml_export_work current = work[--work_count];
        size_t cursor;
        uint64_t terminal_size;
        size_t terminal_end;
        uint8_t child_count;
        size_t child_index;
        if (current.node_offset >= length || current.depth > 1024 ||
            nodes++ >= ML_MAX_EXPORT_NODES) {
            ml_image_diag(image, ML_ERR_FORMAT,
                          offset + current.node_offset,
                          "export trie traversal limit reached");
            free(current.prefix);
            continue;
        }
        if (visited[current.node_offset] != 0) {
            free(current.prefix);
            continue;
        }
        visited[current.node_offset] = 1;
        cursor = (size_t)current.node_offset;
        if (!ml_read_uleb128(bytes, length, &cursor, &terminal_size) ||
            terminal_size > length - cursor) {
            ml_image_diag(image, ML_ERR_FORMAT, offset + current.node_offset,
                          "invalid export trie terminal record");
            free(current.prefix);
            continue;
        }
        terminal_end = cursor + (size_t)terminal_size;
        if (terminal_size != 0 &&
            !ml_export_record_terminal(image, bytes, length, cursor,
                                       terminal_end, current.prefix)) {
            ml_image_diag(image, ML_ERR_FORMAT, offset + current.node_offset,
                          "invalid export terminal payload");
        }
        cursor = terminal_end;
        if (cursor >= length) {
            free(current.prefix);
            continue;
        }
        child_count = bytes[cursor++];
        for (child_index = 0; child_index < child_count; ++child_index) {
            const char *edge;
            size_t edge_length;
            uint64_t child_offset;
            ml_string_builder builder;
            if (!ml_reader_cstring_at(bytes, length, cursor,
                                      &edge, &edge_length)) {
                ml_image_diag(image, ML_ERR_FORMAT, offset + cursor,
                              "unterminated export trie edge");
                result = 1;
                break;
            }
            cursor += edge_length + 1;
            if (!ml_read_uleb128(bytes, length, &cursor, &child_offset)) {
                ml_image_diag(image, ML_ERR_FORMAT, offset + cursor,
                              "invalid export trie child offset");
                break;
            }
            ml_sb_init(&builder);
            if (!ml_sb_append(&builder, current.prefix) ||
                !ml_sb_append_n(&builder, edge, edge_length)) {
                ml_sb_dispose(&builder);
                result = 0;
                break;
            }
            if (!ml_export_push(&work, &work_count, &work_capacity,
                                child_offset, builder.data,
                                current.depth + 1)) {
                ml_sb_dispose(&builder);
                result = 0;
                break;
            }
            ml_sb_dispose(&builder);
        }
        free(current.prefix);
        if (!result) {
            break;
        }
    }
    if (!result) {
        ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                       "unable to store export trie data");
    }
    free(visited);
    ml_export_work_dispose(work, work_count);
    return result;
}

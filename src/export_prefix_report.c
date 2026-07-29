#include "internal.h"

#include <stdlib.h>
#include <string.h>

typedef struct ml_prefix_record {
    char *prefix;
    size_t count;
} ml_prefix_record;

static size_t ml_export_prefix_length(const char *name) {
    size_t index = 0;
    size_t underscore_count = 0;
    if (name == NULL) return 0;
    while (name[index] == '_') {
        ++index;
        ++underscore_count;
    }
    if (name[index] == '$') {
        size_t start = index;
        while (name[index] != '\0' && index - start < 3) ++index;
        return index;
    }
    while (name[index] != '\0' &&
           name[index] != '_' && name[index] != '$' &&
           name[index] != '.' && index < 32) {
        ++index;
    }
    if (index == underscore_count) {
        while (name[index] != '\0' && index < 16) ++index;
    }
    return index;
}

static int ml_prefix_compare(const void *left, const void *right) {
    const ml_prefix_record *a = (const ml_prefix_record *)left;
    const ml_prefix_record *b = (const ml_prefix_record *)right;
    if (a->count > b->count) return -1;
    if (a->count < b->count) return 1;
    return strcmp(a->prefix, b->prefix);
}

char *ml_image_export_prefix_report(const ml_image *image,
                                    size_t maximum_prefixes) {
    ml_prefix_record *records = NULL;
    size_t record_count = 0;
    size_t record_capacity = 0;
    size_t index;
    size_t limit;
    ml_string_builder builder;
    if (image == NULL) {
        return NULL;
    }
    for (index = 0; index < image->export_count; ++index) {
        const char *name = image->exports[index].name;
        size_t length = ml_export_prefix_length(name);
        size_t existing;
        char *prefix;
        if (length == 0) {
            continue;
        }
        prefix = ml_strdup_n(name, length);
        if (prefix == NULL) {
            goto nomem;
        }
        for (existing = 0; existing < record_count; ++existing) {
            if (strcmp(records[existing].prefix, prefix) == 0) {
                ++records[existing].count;
                free(prefix);
                prefix = NULL;
                break;
            }
        }
        if (prefix != NULL) {
            ml_prefix_record record;
            record.prefix = prefix;
            record.count = 1;
            if (!ml_vec_append((void **)&records, &record_count,
                               &record_capacity, &record, sizeof(record))) {
                free(prefix);
                goto nomem;
            }
        }
    }
    if (record_count > 1) {
        qsort(records, record_count, sizeof(*records), ml_prefix_compare);
    }
    limit = maximum_prefixes == 0 || maximum_prefixes > record_count
        ? record_count : maximum_prefixes;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Export prefix distribution\n"
                       "==========================\n"
                       "exports=%zu distinct-prefixes=%zu\n"
                       "%-8s %-10s %s\n",
                       image->export_count,
                       record_count,
                       "rank", "count", "prefix")) {
        ml_sb_dispose(&builder);
        goto nomem;
    }
    for (index = 0; index < limit; ++index) {
        if (!ml_sb_appendf(&builder, "%-8zu %-10zu %s\n",
                           index + 1,
                           records[index].count,
                           records[index].prefix)) {
            ml_sb_dispose(&builder);
            goto nomem;
        }
    }
    if (limit < record_count &&
        !ml_sb_appendf(&builder, "... %zu prefix(es) omitted\n",
                       record_count - limit)) {
        ml_sb_dispose(&builder);
        goto nomem;
    }
    for (index = 0; index < record_count; ++index) {
        free(records[index].prefix);
    }
    free(records);
    return ml_sb_take(&builder);
nomem:
    for (index = 0; index < record_count; ++index) {
        free(records[index].prefix);
    }
    free(records);
    return NULL;
}

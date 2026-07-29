#include "internal.h"

#include <stdlib.h>

typedef struct ml_command_bucket {
    uint32_t command;
    size_t count;
    uint64_t bytes;
    uint64_t first_offset;
    uint64_t last_offset;
} ml_command_bucket;

static int ml_bucket_compare(const void *left, const void *right) {
    const ml_command_bucket *a = (const ml_command_bucket *)left;
    const ml_command_bucket *b = (const ml_command_bucket *)right;
    if (a->count > b->count) return -1;
    if (a->count < b->count) return 1;
    if (a->bytes > b->bytes) return -1;
    if (a->bytes < b->bytes) return 1;
    if (a->command < b->command) return -1;
    if (a->command > b->command) return 1;
    return 0;
}

char *ml_image_command_histogram_report(const ml_image *image) {
    ml_command_bucket *buckets = NULL;
    size_t bucket_count = 0;
    size_t bucket_capacity = 0;
    size_t index;
    size_t required_dyld = 0;
    size_t unknown = 0;
    uint64_t total_bytes = 0;
    ml_string_builder builder;
    if (image == NULL) {
        return NULL;
    }
    for (index = 0; index < image->load_command_count; ++index) {
        const ml_load_command_record *record = &image->load_commands[index];
        size_t bucket_index;
        if ((record->command & ML_LC_REQ_DYLD) != 0) {
            ++required_dyld;
        }
        if (ml_load_command_name(record->command) == NULL ||
            ml_load_command_name(record->command)[3] == 'U') {
            ++unknown;
        }
        if (!ml_u64_add(total_bytes, record->size, &total_bytes)) {
            total_bytes = UINT64_MAX;
        }
        for (bucket_index = 0; bucket_index < bucket_count; ++bucket_index) {
            if (buckets[bucket_index].command == record->command) {
                break;
            }
        }
        if (bucket_index == bucket_count) {
            ml_command_bucket bucket;
            bucket.command = record->command;
            bucket.count = 1;
            bucket.bytes = record->size;
            bucket.first_offset = record->file_offset;
            bucket.last_offset = record->file_offset;
            if (!ml_vec_append((void **)&buckets, &bucket_count,
                               &bucket_capacity, &bucket, sizeof(bucket))) {
                free(buckets);
                return NULL;
            }
        } else {
            ++buckets[bucket_index].count;
            if (!ml_u64_add(buckets[bucket_index].bytes, record->size,
                            &buckets[bucket_index].bytes)) {
                buckets[bucket_index].bytes = UINT64_MAX;
            }
            if (record->file_offset < buckets[bucket_index].first_offset) {
                buckets[bucket_index].first_offset = record->file_offset;
            }
            if (record->file_offset > buckets[bucket_index].last_offset) {
                buckets[bucket_index].last_offset = record->file_offset;
            }
        }
    }
    qsort(buckets, bucket_count, sizeof(*buckets), ml_bucket_compare);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Load-command distribution\n"
                       "=========================\n"
                       "records:              %zu\n"
                       "declared command bytes:%u\n"
                       "summed command bytes: %llu\n"
                       "distinct commands:    %zu\n"
                       "required-by-dyld:      %zu\n"
                       "unknown commands:      %zu\n\n"
                       "%-6s %-32s %-8s %-12s %-14s %s\n",
                       image->load_command_count,
                       image->command_bytes,
                       (unsigned long long)total_bytes,
                       bucket_count,
                       required_dyld,
                       unknown,
                       "rank", "command", "count", "bytes",
                       "first-offset", "last-offset")) {
        ml_sb_dispose(&builder);
        free(buckets);
        return NULL;
    }
    for (index = 0; index < bucket_count; ++index) {
        if (!ml_sb_appendf(&builder,
                           "%-6zu %-32s %-8zu %-12llu %#-14llx %#llx\n",
                           index + 1,
                           ml_load_command_name(buckets[index].command),
                           buckets[index].count,
                           (unsigned long long)buckets[index].bytes,
                           (unsigned long long)buckets[index].first_offset,
                           (unsigned long long)buckets[index].last_offset)) {
            ml_sb_dispose(&builder);
            free(buckets);
            return NULL;
        }
    }
    free(buckets);
    return ml_sb_take(&builder);
}

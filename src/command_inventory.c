#include "internal.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

typedef enum ml_command_category {
    ML_COMMAND_CATEGORY_LAYOUT = 0,
    ML_COMMAND_CATEGORY_SYMBOLS,
    ML_COMMAND_CATEGORY_LINKING,
    ML_COMMAND_CATEGORY_METADATA,
    ML_COMMAND_CATEGORY_EXECUTION,
    ML_COMMAND_CATEGORY_SECURITY,
    ML_COMMAND_CATEGORY_LEGACY,
    ML_COMMAND_CATEGORY_OTHER,
    ML_COMMAND_CATEGORY_COUNT
} ml_command_category;

typedef struct ml_command_inventory_bucket {
    ml_command_category category;
    size_t count;
    uint64_t bytes;
    uint64_t first_offset;
    uint64_t last_end;
    size_t required_by_dyld;
    size_t duplicate_records;
} ml_command_inventory_bucket;

static uint32_t ml_command_base(uint32_t command) {
    return command & ~ML_LC_REQ_DYLD;
}

static ml_command_category ml_command_category_for(uint32_t command) {
    uint32_t base = ml_command_base(command);
    switch (base) {
        case ML_LC_SEGMENT:
        case ML_LC_SEGMENT_64:
        case ML_LC_FILESET_ENTRY:
            return ML_COMMAND_CATEGORY_LAYOUT;

        case ML_LC_SYMTAB:
        case ML_LC_DYSYMTAB:
        case ML_LC_SYMSEG:
        case ML_LC_TWOLEVEL_HINTS:
        case ML_LC_FUNCTION_STARTS:
        case ML_LC_DATA_IN_CODE:
            return ML_COMMAND_CATEGORY_SYMBOLS;

        case ML_LC_LOAD_DYLIB:
        case ML_LC_ID_DYLIB:
        case (ML_LC_LOAD_WEAK_DYLIB & ~ML_LC_REQ_DYLD):
        case (ML_LC_REEXPORT_DYLIB & ~ML_LC_REQ_DYLD):
        case ML_LC_LAZY_LOAD_DYLIB:
        case (ML_LC_LOAD_UPWARD_DYLIB & ~ML_LC_REQ_DYLD):
        case ML_LC_LOAD_DYLINKER:
        case ML_LC_ID_DYLINKER:
        case (ML_LC_RPATH & ~ML_LC_REQ_DYLD):
        case ML_LC_DYLD_INFO:
        case (ML_LC_DYLD_EXPORTS_TRIE & ~ML_LC_REQ_DYLD):
        case (ML_LC_DYLD_CHAINED_FIXUPS & ~ML_LC_REQ_DYLD):
            return ML_COMMAND_CATEGORY_LINKING;

        case ML_LC_UUID:
        case ML_LC_SOURCE_VERSION:
        case ML_LC_BUILD_VERSION:
        case ML_LC_VERSION_MIN_MACOSX:
        case ML_LC_VERSION_MIN_IPHONEOS:
        case ML_LC_VERSION_MIN_TVOS:
        case ML_LC_VERSION_MIN_WATCHOS:
        case ML_LC_NOTE:
        case ML_LC_LINKER_OPTION:
        case ML_LC_LINKER_OPTIMIZATION_HINT:
        case ML_LC_SUB_FRAMEWORK:
        case ML_LC_SUB_UMBRELLA:
        case ML_LC_SUB_CLIENT:
        case ML_LC_SUB_LIBRARY:
            return ML_COMMAND_CATEGORY_METADATA;

        case ML_LC_THREAD:
        case ML_LC_UNIXTHREAD:
        case ML_LC_ROUTINES:
        case ML_LC_ROUTINES_64:
        case (ML_LC_MAIN & ~ML_LC_REQ_DYLD):
        case ML_LC_DYLD_ENVIRONMENT:
            return ML_COMMAND_CATEGORY_EXECUTION;

        case ML_LC_CODE_SIGNATURE:
        case ML_LC_DYLIB_CODE_SIGN_DRS:
        case ML_LC_ENCRYPTION_INFO:
        case ML_LC_ENCRYPTION_INFO_64:
            return ML_COMMAND_CATEGORY_SECURITY;

        case ML_LC_LOADFVMLIB:
        case ML_LC_IDFVMLIB:
        case ML_LC_IDENT:
        case ML_LC_FVMFILE:
        case ML_LC_PREPAGE:
        case ML_LC_PREBOUND_DYLIB:
        case ML_LC_PREBIND_CKSUM:
        case ML_LC_SEGMENT_SPLIT_INFO:
            return ML_COMMAND_CATEGORY_LEGACY;

        default:
            return ML_COMMAND_CATEGORY_OTHER;
    }
}

static const char *ml_command_category_name(ml_command_category category) {
    static const char *const names[ML_COMMAND_CATEGORY_COUNT] = {
        "layout",
        "symbols",
        "linking",
        "metadata",
        "execution",
        "security",
        "legacy",
        "other"
    };
    if ((size_t)category >= ML_COMMAND_CATEGORY_COUNT) {
        return "other";
    }
    return names[category];
}

static int ml_inventory_append_u64_or_null(ml_string_builder *builder,
                                            uint64_t value,
                                            int available) {
    if (!available) {
        return ml_sb_append(builder, "null");
    }
    return ml_sb_appendf(builder, "\"0x%" PRIx64 "\"", value);
}

static size_t ml_command_duplicate_count(const ml_image *image,
                                         size_t record_index) {
    const ml_load_command_record *record;
    size_t index;
    size_t duplicates = 0;
    if (image == NULL || record_index >= image->load_command_count) {
        return 0;
    }
    record = &image->load_commands[record_index];
    for (index = 0; index < record_index; ++index) {
        if (image->load_commands[index].command == record->command) {
            ++duplicates;
        }
    }
    return duplicates;
}

static int ml_inventory_build_buckets(const ml_image *image,
                                      ml_command_inventory_bucket *buckets,
                                      uint64_t *total_bytes,
                                      size_t *required_count,
                                      size_t *duplicate_count) {
    size_t index;
    if (image == NULL || buckets == NULL || total_bytes == NULL ||
        required_count == NULL || duplicate_count == NULL) {
        return 0;
    }
    memset(buckets, 0,
           sizeof(*buckets) * (size_t)ML_COMMAND_CATEGORY_COUNT);
    *total_bytes = 0;
    *required_count = 0;
    *duplicate_count = 0;
    for (index = 0; index < ML_COMMAND_CATEGORY_COUNT; ++index) {
        buckets[index].category = (ml_command_category)index;
        buckets[index].first_offset = UINT64_MAX;
    }
    for (index = 0; index < image->load_command_count; ++index) {
        const ml_load_command_record *record = &image->load_commands[index];
        ml_command_category category = ml_command_category_for(record->command);
        ml_command_inventory_bucket *bucket = &buckets[category];
        uint64_t end;
        size_t duplicates = ml_command_duplicate_count(image, index);
        ++bucket->count;
        if (!ml_u64_add(bucket->bytes, record->size, &bucket->bytes)) {
            bucket->bytes = UINT64_MAX;
        }
        if (!ml_u64_add(*total_bytes, record->size, total_bytes)) {
            *total_bytes = UINT64_MAX;
        }
        if (record->file_offset < bucket->first_offset) {
            bucket->first_offset = record->file_offset;
        }
        if (!ml_u64_add(record->file_offset, record->size, &end)) {
            end = UINT64_MAX;
        }
        if (end > bucket->last_end) {
            bucket->last_end = end;
        }
        if ((record->command & ML_LC_REQ_DYLD) != 0) {
            ++bucket->required_by_dyld;
            ++*required_count;
        }
        if (duplicates != 0) {
            ++bucket->duplicate_records;
            ++*duplicate_count;
        }
    }
    return 1;
}

static int ml_inventory_append_summary(ml_string_builder *builder,
                                       const ml_image *image,
                                       uint64_t total_bytes,
                                       size_t required_count,
                                       size_t duplicate_count) {
    uint64_t header_end;
    int header_end_available;
    if (builder == NULL || image == NULL) {
        return 0;
    }
    header_end_available = ml_u64_add(image->command_offset,
                                      image->command_bytes,
                                      &header_end);
    if (!ml_sb_append(builder, "\"summary\":{")) {
        return 0;
    }
    if (!ml_sb_appendf(builder,
                       "\"record_count\":%zu,"
                       "\"declared_command_bytes\":%u,"
                       "\"summed_command_bytes\":%" PRIu64 ","
                       "\"required_by_dyld\":%zu,"
                       "\"duplicate_records\":%zu,"
                       "\"header_size\":%u,"
                       "\"command_region_end\":",
                       image->load_command_count,
                       image->command_bytes,
                       total_bytes,
                       required_count,
                       duplicate_count,
                       image->command_offset)) {
        return 0;
    }
    if (!ml_inventory_append_u64_or_null(builder, header_end,
                                          header_end_available)) {
        return 0;
    }
    if (!ml_sb_appendf(builder,
                       ",\"declared_matches_sum\":%s,"
                       "\"records_match_header\":%s}",
                       total_bytes == image->command_bytes ? "true" : "false",
                       image->load_command_count == image->command_count
                           ? "true" : "false")) {
        return 0;
    }
    return 1;
}

static int ml_inventory_append_categories(
    ml_string_builder *builder,
    const ml_command_inventory_bucket *buckets,
    uint64_t total_bytes) {
    size_t index;
    int first = 1;
    if (builder == NULL || buckets == NULL) {
        return 0;
    }
    if (!ml_sb_append(builder, ",\"categories\":[")) {
        return 0;
    }
    for (index = 0; index < ML_COMMAND_CATEGORY_COUNT; ++index) {
        const ml_command_inventory_bucket *bucket = &buckets[index];
        double share = total_bytes == 0
                           ? 0.0
                           : (double)bucket->bytes * 100.0 /
                                 (double)total_bytes;
        if (bucket->count == 0) {
            continue;
        }
        if (!first && !ml_sb_append(builder, ",")) {
            return 0;
        }
        first = 0;
        if (!ml_sb_appendf(builder,
                           "{\"name\":\"%s\","
                           "\"count\":%zu,"
                           "\"bytes\":%" PRIu64 ","
                           "\"byte_share_percent\":%.3f,"
                           "\"required_by_dyld\":%zu,"
                           "\"duplicate_records\":%zu,"
                           "\"first_offset\":",
                           ml_command_category_name(bucket->category),
                           bucket->count,
                           bucket->bytes,
                           share,
                           bucket->required_by_dyld,
                           bucket->duplicate_records)) {
            return 0;
        }
        if (!ml_inventory_append_u64_or_null(builder,
                                              bucket->first_offset,
                                              bucket->first_offset != UINT64_MAX)) {
            return 0;
        }
        if (!ml_sb_append(builder, ",\"last_end\":")) {
            return 0;
        }
        if (!ml_inventory_append_u64_or_null(builder,
                                              bucket->last_end,
                                              bucket->count != 0)) {
            return 0;
        }
        if (!ml_sb_append(builder, "}")) {
            return 0;
        }
    }
    return ml_sb_append(builder, "]");
}

static int ml_inventory_append_records(ml_string_builder *builder,
                                       const ml_image *image) {
    size_t index;
    if (builder == NULL || image == NULL) {
        return 0;
    }
    if (!ml_sb_append(builder, ",\"records\":[")) {
        return 0;
    }
    for (index = 0; index < image->load_command_count; ++index) {
        const ml_load_command_record *record = &image->load_commands[index];
        ml_command_category category = ml_command_category_for(record->command);
        uint64_t end;
        int end_available = ml_u64_add(record->file_offset,
                                       record->size,
                                       &end);
        const char *name = ml_load_command_name(record->command);
        if (index != 0 && !ml_sb_append(builder, ",")) {
            return 0;
        }
        if (!ml_sb_appendf(builder,
                           "{\"index\":%zu,"
                           "\"command\":\"0x%08" PRIx32 "\","
                           "\"name\":",
                           index,
                           record->command)) {
            return 0;
        }
        if (!ml_sb_json_string(builder, name == NULL ? "unknown" : name)) {
            return 0;
        }
        if (!ml_sb_appendf(builder,
                           ",\"category\":\"%s\","
                           "\"size\":%u,"
                           "\"offset\":\"0x%" PRIx64 "\","
                           "\"end\":",
                           ml_command_category_name(category),
                           record->size,
                           record->file_offset)) {
            return 0;
        }
        if (!ml_inventory_append_u64_or_null(builder, end, end_available)) {
            return 0;
        }
        if (!ml_sb_appendf(builder,
                           ",\"required_by_dyld\":%s,"
                           "\"prior_same_command\":%zu}",
                           (record->command & ML_LC_REQ_DYLD) != 0
                               ? "true" : "false",
                           ml_command_duplicate_count(image, index))) {
            return 0;
        }
    }
    return ml_sb_append(builder, "]");
}

char *ml_image_command_inventory_json(const ml_image *image) {
    ml_command_inventory_bucket buckets[ML_COMMAND_CATEGORY_COUNT];
    uint64_t total_bytes;
    size_t required_count;
    size_t duplicate_count;
    ml_string_builder builder;
    if (image == NULL) {
        return NULL;
    }
    if (!ml_inventory_build_buckets(image,
                                    buckets,
                                    &total_bytes,
                                    &required_count,
                                    &duplicate_count)) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_append(&builder, "{")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (!ml_inventory_append_summary(&builder,
                                     image,
                                     total_bytes,
                                     required_count,
                                     duplicate_count)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (!ml_inventory_append_categories(&builder, buckets, total_bytes)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (!ml_inventory_append_records(&builder, image)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (!ml_sb_append(&builder, "}")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

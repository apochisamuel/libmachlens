#include "internal.h"

#include <inttypes.h>

static const char *ml_hash_type_name(uint8_t hash_type) {
    switch (hash_type) {
        case 1: return "SHA-1";
        case 2: return "SHA-256";
        case 3: return "SHA-256-truncated";
        case 4: return "SHA-384";
        case 5: return "SHA-512";
        default: return "unknown";
    }
}

char *ml_image_signature_report(const ml_image *image) {
    ml_string_builder builder;
    ml_security_summary security;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    security = ml_image_security_summary(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Code-signing and execution policy\n"
                       "=================================\n"
                       "signature command:       %s\n"
                       "decoded blobs:           %zu\n"
                       "code directories:        %zu\n"
                       "entitlement keys:        %zu\n"
                       "encrypted:               %s\n"
                       "position independent:    %s\n"
                       "no heap execution:       %s\n"
                       "allows stack execution:  %s\n"
                       "writable executable map: %s\n"
                       "page-zero segment:       %s\n"
                       "restricted section:      %s\n\n",
                       security.has_code_signature ? "present" : "absent",
                       image->code_blob_count,
                       image->code_directory_count,
                       image->entitlement_key_count,
                       security.encrypted ? "yes" : "no",
                       security.position_independent ? "yes" : "no",
                       security.no_heap_execution ? "yes" : "no",
                       security.allows_stack_execution ? "yes" : "no",
                       security.has_writable_executable_segment ? "yes" : "no",
                       security.has_pagezero_segment ? "yes" : "no",
                       security.has_restricted_section ? "yes" : "no")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->code_blob_count; ++index) {
        const ml_code_blob_record *blob = &image->code_blobs[index];
        if (!ml_sb_appendf(&builder,
                           "blob %zu: %-26s magic=%#010x offset=%#x length=%u\n",
                           index,
                           ml_code_blob_type_name(blob->type),
                           blob->magic,
                           blob->offset,
                           blob->length)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    for (index = 0; index < image->code_directory_count; ++index) {
        const ml_code_directory_record *directory =
            &image->code_directories[index];
        if (!ml_sb_appendf(&builder,
                           "\nCodeDirectory %zu\n"
                           "  version:          %#x\n"
                           "  flags:            %#x\n"
                           "  identifier:       %s\n"
                           "  team identifier:  %s\n"
                           "  hash:              %s (%u bytes)\n"
                           "  page exponent:     %u\n"
                           "  special slots:     %u\n"
                           "  code slots:        %u\n"
                           "  code limit:        %u\n"
                           "  code limit 64:     %" PRIu64 "\n"
                           "  exec segment:      %#" PRIx64 "+%#" PRIx64
                           " flags=%#" PRIx64 "\n",
                           index,
                           directory->version,
                           directory->flags,
                           directory->identifier != NULL
                               ? directory->identifier : "",
                           directory->team_identifier != NULL
                               ? directory->team_identifier : "",
                           ml_hash_type_name(directory->hash_type),
                           directory->hash_size,
                           directory->page_size_log2,
                           directory->special_slot_count,
                           directory->code_slot_count,
                           directory->code_limit,
                           directory->code_limit_64,
                           directory->executable_segment_base,
                           directory->executable_segment_limit,
                           directory->executable_segment_flags)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (image->entitlement_key_count != 0) {
        if (!ml_sb_append(&builder, "\nEntitlement keys\n----------------\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < image->entitlement_key_count; ++index) {
            if (!ml_sb_appendf(&builder, "  %s\n",
                               image->entitlement_keys[index])) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
    }
    return ml_sb_take(&builder);
}

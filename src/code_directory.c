#include "internal.h"

#include <stdlib.h>
#include <string.h>

static uint32_t ml_cd_u32(const uint8_t *data) {
    return ml_load_u32(data, ML_ENDIAN_BIG);
}

static uint64_t ml_cd_u64(const uint8_t *data) {
    return ml_load_u64(data, ML_ENDIAN_BIG);
}

static char *ml_cd_string(const uint8_t *blob, size_t size, uint32_t offset) {
    const char *text;
    size_t length;
    if (offset == 0) {
        return NULL;
    }
    if (!ml_reader_cstring_at(blob, size, offset, &text, &length)) {
        return NULL;
    }
    return ml_strdup_n(text, length);
}

static int ml_decode_code_directory_blob(ml_image *image,
                                         const uint8_t *blob,
                                         size_t blob_size) {
    ml_code_directory_record record;
    uint32_t length;
    uint32_t identifier_offset;
    uint32_t team_offset = 0;
    memset(&record, 0, sizeof(record));
    if (blob_size < 44 || ml_cd_u32(blob) != ML_CS_MAGIC_CODEDIRECTORY) {
        return 1;
    }
    length = ml_cd_u32(blob + 4);
    if (length < 44 || length > blob_size) {
        return 1;
    }
    record.version = ml_cd_u32(blob + 8);
    record.flags = ml_cd_u32(blob + 12);
    record.special_slot_count = ml_cd_u32(blob + 24);
    record.code_slot_count = ml_cd_u32(blob + 28);
    record.code_limit = ml_cd_u32(blob + 32);
    record.hash_size = blob[36];
    record.hash_type = blob[37];
    record.platform = blob[38];
    record.page_size_log2 = blob[39];
    identifier_offset = ml_cd_u32(blob + 20);
    if (record.version >= 0x20100u && length >= 48) {
        uint32_t scatter_offset = ml_cd_u32(blob + 44);
        if (scatter_offset != 0 && scatter_offset >= length) {
            ml_image_diag(image, ML_ERR_RANGE,
                          image->code_signature_offset + scatter_offset,
                          "code-directory scatter offset lies outside blob");
        }
    }
    if (record.version >= 0x20200u && length >= 52) {
        team_offset = ml_cd_u32(blob + 48);
    }
    if (record.version >= 0x20300u && length >= 64) {
        record.code_limit_64 = ml_cd_u64(blob + 56);
    }
    if (record.version >= 0x20400u && length >= 88) {
        record.executable_segment_base = ml_cd_u64(blob + 64);
        record.executable_segment_limit = ml_cd_u64(blob + 72);
        record.executable_segment_flags = ml_cd_u64(blob + 80);
    }
    record.identifier = ml_cd_string(blob, length, identifier_offset);
    record.team_identifier = ml_cd_string(blob, length, team_offset);
    if (identifier_offset != 0 && record.identifier == NULL) {
        ml_image_diag(image, ML_ERR_FORMAT,
                      image->code_signature_offset + identifier_offset,
                      "code-directory identifier is not a valid string");
    }
    if (team_offset != 0 && record.team_identifier == NULL) {
        ml_image_diag(image, ML_ERR_FORMAT,
                      image->code_signature_offset + team_offset,
                      "code-directory team identifier is not a valid string");
    }
    if (record.page_size_log2 > 31) {
        ml_image_diag(image, ML_ERR_FORMAT, image->code_signature_offset,
                      "code-directory page size exponent is too large");
    }
    if (!ml_vec_append((void **)&image->code_directories,
                       &image->code_directory_count,
                       &image->code_directory_capacity,
                       &record, sizeof(record))) {
        free(record.identifier);
        free(record.team_identifier);
        return 0;
    }
    return 1;
}

int ml_decode_code_directory_details(ml_image *image) {
    const uint8_t *signature;
    size_t index;
    if (image == NULL || image->code_signature_size == 0 ||
        image->code_blob_count == 0) {
        return 1;
    }
    if (!ml_image_slice(image, image->code_signature_offset,
                        image->code_signature_size, &signature)) {
        return 1;
    }
    for (index = 0; index < image->code_blob_count; ++index) {
        const ml_code_blob_record *blob = &image->code_blobs[index];
        if (blob->magic != ML_CS_MAGIC_CODEDIRECTORY ||
            blob->offset > image->code_signature_size ||
            blob->length > image->code_signature_size - blob->offset) {
            continue;
        }
        if (!ml_decode_code_directory_blob(image, signature + blob->offset,
                                           blob->length)) {
            ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                           "unable to store code-directory details");
            return 0;
        }
    }
    return 1;
}

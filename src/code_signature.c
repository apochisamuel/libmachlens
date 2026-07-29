#include "internal.h"

static uint32_t ml_be32(const uint8_t *data) {
    return ml_load_u32(data, ML_ENDIAN_BIG);
}

int ml_decode_code_signature(ml_image *image) {
    const uint8_t *bytes;
    uint32_t magic;
    uint32_t length;
    uint32_t count;
    uint32_t index;
    if (image == NULL) {
        return 0;
    }
    if (image->code_signature_size == 0) {
        return 1;
    }
    if (!ml_image_slice(image, image->code_signature_offset,
                        image->code_signature_size, &bytes) ||
        image->code_signature_size < 8) {
        ml_image_diag(image, ML_ERR_RANGE, image->code_signature_offset,
                      "code-signature blob lies outside image");
        return 1;
    }
    magic = ml_be32(bytes);
    length = ml_be32(bytes + 4);
    if (length < 8 || length > image->code_signature_size) {
        ml_image_diag(image, ML_ERR_FORMAT, image->code_signature_offset,
                      "invalid code-signature blob length");
        return 1;
    }
    if (magic != ML_CS_MAGIC_EMBEDDED_SIGNATURE &&
        magic != ML_CS_MAGIC_DETACHED_SIGNATURE) {
        ml_code_blob_record single;
        single.type = 0;
        single.magic = magic;
        single.offset = 0;
        single.length = length;
        if (!ml_vec_append((void **)&image->code_blobs,
                           &image->code_blob_count,
                           &image->code_blob_capacity,
                           &single, sizeof(single))) {
            ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                           "unable to store code-signature blob");
            return 0;
        }
        return 1;
    }
    if (length < 12) {
        ml_image_diag(image, ML_ERR_FORMAT, image->code_signature_offset,
                      "code-signature superblob is truncated");
        return 1;
    }
    count = ml_be32(bytes + 8);
    if (count > (length - 12) / 8) {
        ml_image_diag(image, ML_ERR_FORMAT, image->code_signature_offset + 8,
                      "code-signature index exceeds superblob");
        return 1;
    }
    for (index = 0; index < count; ++index) {
        uint32_t type = ml_be32(bytes + 12 + index * 8);
        uint32_t offset = ml_be32(bytes + 16 + index * 8);
        ml_code_blob_record record;
        if (offset > length - 8) {
            ml_image_diag(image, ML_ERR_RANGE,
                          image->code_signature_offset + offset,
                          "code-signature component offset lies outside superblob");
            continue;
        }
        record.type = type;
        record.magic = ml_be32(bytes + offset);
        record.offset = offset;
        record.length = ml_be32(bytes + offset + 4);
        if (record.length < 8 || record.length > length - offset) {
            ml_image_diag(image, ML_ERR_FORMAT,
                          image->code_signature_offset + offset,
                          "invalid code-signature component length");
            continue;
        }
        if (!ml_vec_append((void **)&image->code_blobs,
                           &image->code_blob_count,
                           &image->code_blob_capacity,
                           &record, sizeof(record))) {
            ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                           "unable to store code-signature components");
            return 0;
        }
    }
    return 1;
}

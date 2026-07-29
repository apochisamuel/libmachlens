#include "internal.h"

#include <stdlib.h>
#include <string.h>

static int ml_entitlement_exists(const ml_image *image, const char *key) {
    size_t index;
    for (index = 0; index < image->entitlement_key_count; ++index) {
        if (strcmp(image->entitlement_keys[index], key) == 0) {
            return 1;
        }
    }
    return 0;
}

static int ml_entitlement_append(ml_image *image, char *key) {
    if (key == NULL || key[0] == '\0') {
        free(key);
        return 1;
    }
    if (ml_entitlement_exists(image, key)) {
        free(key);
        return 1;
    }
    if (!ml_vec_append((void **)&image->entitlement_keys,
                       &image->entitlement_key_count,
                       &image->entitlement_key_capacity,
                       &key, sizeof(key))) {
        free(key);
        return 0;
    }
    return 1;
}

static int ml_hex_digit(unsigned char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

static int ml_append_utf8(ml_string_builder *builder, uint32_t value) {
    char bytes[4];
    size_t count;
    if (value <= 0x7fu) {
        bytes[0] = (char)value;
        count = 1;
    } else if (value <= 0x7ffu) {
        bytes[0] = (char)(0xc0u | (value >> 6));
        bytes[1] = (char)(0x80u | (value & 0x3fu));
        count = 2;
    } else if (value <= 0xffffu && (value < 0xd800u || value > 0xdfffu)) {
        bytes[0] = (char)(0xe0u | (value >> 12));
        bytes[1] = (char)(0x80u | ((value >> 6) & 0x3fu));
        bytes[2] = (char)(0x80u | (value & 0x3fu));
        count = 3;
    } else if (value <= 0x10ffffu) {
        bytes[0] = (char)(0xf0u | (value >> 18));
        bytes[1] = (char)(0x80u | ((value >> 12) & 0x3fu));
        bytes[2] = (char)(0x80u | ((value >> 6) & 0x3fu));
        bytes[3] = (char)(0x80u | (value & 0x3fu));
        count = 4;
    } else {
        return 0;
    }
    return ml_sb_append_n(builder, bytes, count);
}

static int ml_decode_entity(ml_string_builder *builder,
                            const char *text, size_t length) {
    if (length == 3 && memcmp(text, "amp", 3) == 0) {
        return ml_sb_append(builder, "&");
    }
    if (length == 2 && memcmp(text, "lt", 2) == 0) {
        return ml_sb_append(builder, "<");
    }
    if (length == 2 && memcmp(text, "gt", 2) == 0) {
        return ml_sb_append(builder, ">");
    }
    if (length == 4 && memcmp(text, "quot", 4) == 0) {
        return ml_sb_append(builder, "\"");
    }
    if (length == 4 && memcmp(text, "apos", 4) == 0) {
        return ml_sb_append(builder, "'");
    }
    if (length >= 2 && text[0] == '#') {
        uint32_t value = 0;
        size_t index = 1;
        int base = 10;
        if (index < length && (text[index] == 'x' || text[index] == 'X')) {
            base = 16;
            ++index;
        }
        if (index == length) {
            return 0;
        }
        for (; index < length; ++index) {
            int digit;
            if (base == 16) {
                digit = ml_hex_digit((unsigned char)text[index]);
            } else if (text[index] >= '0' && text[index] <= '9') {
                digit = text[index] - '0';
            } else {
                digit = -1;
            }
            if (digit < 0 || value > (0x10ffffu - (uint32_t)digit) /
                                      (uint32_t)base) {
                return 0;
            }
            value = value * (uint32_t)base + (uint32_t)digit;
        }
        return ml_append_utf8(builder, value);
    }
    return 0;
}

static char *ml_xml_text_decode(const char *text, size_t length) {
    ml_string_builder builder;
    size_t cursor = 0;
    ml_sb_init(&builder);
    while (cursor < length) {
        size_t start;
        size_t end;
        if (text[cursor] != '&') {
            if (!ml_sb_append_n(&builder, text + cursor, 1)) {
                ml_sb_dispose(&builder);
                return NULL;
            }
            ++cursor;
            continue;
        }
        start = ++cursor;
        end = start;
        while (end < length && text[end] != ';' && end - start < 16) {
            ++end;
        }
        if (end >= length || text[end] != ';' ||
            !ml_decode_entity(&builder, text + start, end - start)) {
            if (!ml_sb_append(&builder, "&")) {
                ml_sb_dispose(&builder);
                return NULL;
            }
            cursor = start;
            continue;
        }
        cursor = end + 1;
    }
    return ml_sb_take(&builder);
}

static int ml_extract_xml_keys(ml_image *image,
                               const char *xml, size_t length) {
    size_t cursor = 0;
    const char open_tag[] = "<key>";
    const char close_tag[] = "</key>";
    while (cursor + sizeof(open_tag) - 1 <= length) {
        size_t key_start;
        size_t key_end;
        char *decoded;
        const char *found = strstr(xml + cursor, open_tag);
        if (found == NULL || (size_t)(found - xml) >= length) {
            break;
        }
        key_start = (size_t)(found - xml) + sizeof(open_tag) - 1;
        if (key_start > length) {
            break;
        }
        found = strstr(xml + key_start, close_tag);
        if (found == NULL || (size_t)(found - xml) > length) {
            break;
        }
        key_end = (size_t)(found - xml);
        decoded = ml_xml_text_decode(xml + key_start, key_end - key_start);
        if (decoded == NULL || !ml_entitlement_append(image, decoded)) {
            free(decoded);
            return 0;
        }
        cursor = key_end + sizeof(close_tag) - 1;
    }
    return 1;
}

int ml_decode_entitlements(ml_image *image) {
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
        const uint8_t *payload;
        size_t payload_size;
        if (blob->magic != ML_CS_MAGIC_ENTITLEMENTS || blob->length <= 8 ||
            blob->offset > image->code_signature_size ||
            blob->length > image->code_signature_size - blob->offset) {
            continue;
        }
        payload = signature + blob->offset + 8;
        payload_size = blob->length - 8;
        if (memchr(payload, 0, payload_size) != NULL) {
            ml_image_diag(image, ML_ERR_FORMAT,
                          image->code_signature_offset + blob->offset + 8,
                          "entitlements payload contains embedded null bytes");
            continue;
        }
        {
            char *xml = ml_strdup_n((const char *)payload, payload_size);
            int ok;
            if (xml == NULL) {
                return 0;
            }
            ok = ml_extract_xml_keys(image, xml, payload_size);
            free(xml);
            if (!ok) {
                ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                               "unable to store entitlement keys");
                return 0;
            }
        }
    }
    return 1;
}

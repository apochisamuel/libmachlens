#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ml_sb_init(ml_string_builder *builder) {
    if (builder == NULL) {
        return;
    }
    builder->data = NULL;
    builder->length = 0;
    builder->capacity = 0;
}

void ml_sb_dispose(ml_string_builder *builder) {
    if (builder == NULL) {
        return;
    }
    free(builder->data);
    builder->data = NULL;
    builder->length = 0;
    builder->capacity = 0;
}

static int ml_sb_reserve(ml_string_builder *builder, size_t additional) {
    size_t needed;
    size_t next;
    char *memory;
    if (builder == NULL ||
        !ml_size_add(builder->length, additional, &needed) ||
        !ml_size_add(needed, 1, &needed)) {
        return 0;
    }
    if (needed <= builder->capacity) {
        return 1;
    }
    next = builder->capacity == 0 ? 128 : builder->capacity;
    while (next < needed) {
        if (next > SIZE_MAX / 2) {
            next = needed;
            break;
        }
        next *= 2;
    }
    memory = (char *)realloc(builder->data, next);
    if (memory == NULL) {
        return 0;
    }
    builder->data = memory;
    builder->capacity = next;
    return 1;
}

int ml_sb_append_n(ml_string_builder *builder, const char *text, size_t length) {
    if (builder == NULL || (text == NULL && length != 0) ||
        !ml_sb_reserve(builder, length)) {
        return 0;
    }
    if (length != 0) {
        memcpy(builder->data + builder->length, text, length);
    }
    builder->length += length;
    builder->data[builder->length] = '\0';
    return 1;
}

int ml_sb_append(ml_string_builder *builder, const char *text) {
    return text != NULL && ml_sb_append_n(builder, text, strlen(text));
}

int ml_sb_appendf(ml_string_builder *builder, const char *fmt, ...) {
    va_list args;
    va_list copy;
    int count;
    if (builder == NULL || fmt == NULL) {
        return 0;
    }
    va_start(args, fmt);
    va_copy(copy, args);
    count = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (count < 0 || !ml_sb_reserve(builder, (size_t)count)) {
        va_end(args);
        return 0;
    }
    (void)vsnprintf(builder->data + builder->length,
                    builder->capacity - builder->length,
                    fmt, args);
    va_end(args);
    builder->length += (size_t)count;
    return 1;
}

int ml_sb_json_string(ml_string_builder *builder, const char *text) {
    const unsigned char *cursor = (const unsigned char *)text;
    if (!ml_sb_append(builder, "\"")) {
        return 0;
    }
    if (cursor != NULL) {
        while (*cursor != 0) {
            unsigned char ch = *cursor++;
            switch (ch) {
                case '\"': if (!ml_sb_append(builder, "\\\"")) return 0; break;
                case '\\': if (!ml_sb_append(builder, "\\\\")) return 0; break;
                case '\b': if (!ml_sb_append(builder, "\\b")) return 0; break;
                case '\f': if (!ml_sb_append(builder, "\\f")) return 0; break;
                case '\n': if (!ml_sb_append(builder, "\\n")) return 0; break;
                case '\r': if (!ml_sb_append(builder, "\\r")) return 0; break;
                case '\t': if (!ml_sb_append(builder, "\\t")) return 0; break;
                default:
                    if (ch < 0x20u) {
                        if (!ml_sb_appendf(builder, "\\u%04x", ch)) return 0;
                    } else {
                        char one = (char)ch;
                        if (!ml_sb_append_n(builder, &one, 1)) return 0;
                    }
                    break;
            }
        }
    }
    return ml_sb_append(builder, "\"");
}

char *ml_sb_take(ml_string_builder *builder) {
    char *result;
    if (builder == NULL) {
        return NULL;
    }
    if (builder->data == NULL) {
        result = ml_strdup_safe("");
    } else {
        result = builder->data;
    }
    builder->data = NULL;
    builder->length = 0;
    builder->capacity = 0;
    return result;
}

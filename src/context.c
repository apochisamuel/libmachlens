#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ml_context *ml_context_create(void) {
    ml_context *ctx = (ml_context *)calloc(1, sizeof(*ctx));
    if (ctx != NULL) {
        ctx->status = ML_OK;
        ctx->error[0] = '\0';
    }
    return ctx;
}

void ml_context_destroy(ml_context *ctx) {
    free(ctx);
}

const char *ml_context_error(const ml_context *ctx) {
    return ctx != NULL ? ctx->error : "invalid context";
}

ml_status ml_context_status(const ml_context *ctx) {
    return ctx != NULL ? ctx->status : ML_ERR_ARGUMENT;
}

void ml_context_clear(ml_context *ctx) {
    if (ctx == NULL) {
        return;
    }
    ctx->status = ML_OK;
    ctx->error[0] = '\0';
}

void ml_context_set(ml_context *ctx, ml_status status, const char *fmt, ...) {
    va_list args;
    if (ctx == NULL) {
        return;
    }
    ctx->status = status;
    if (fmt == NULL) {
        ctx->error[0] = '\0';
        return;
    }
    va_start(args, fmt);
    (void)vsnprintf(ctx->error, sizeof(ctx->error), fmt, args);
    va_end(args);
}

void ml_image_diag(ml_image *image, ml_status status, uint64_t offset,
                   const char *fmt, ...) {
    ml_diagnostic item;
    char buffer[512];
    va_list args;
    if (image == NULL || fmt == NULL ||
        image->diagnostic_count >= ML_MAX_DIAGNOSTICS) {
        return;
    }
    va_start(args, fmt);
    (void)vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    item.status = status;
    item.offset = offset;
    item.message = ml_strdup_safe(buffer);
    if (item.message == NULL) {
        return;
    }
    if (!ml_vec_append((void **)&image->diagnostics,
                       &image->diagnostic_count,
                       &image->diagnostic_capacity,
                       &item, sizeof(item))) {
        free(item.message);
    }
}

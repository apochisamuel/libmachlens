#include "internal.h"

#include <stdio.h>
#include <stdlib.h>

char *ml_format_packed_version(uint32_t version) {
    unsigned major = (version >> 16) & 0xffffu;
    unsigned minor = (version >> 8) & 0xffu;
    unsigned patch = version & 0xffu;
    char buffer[64];
    int written;
    if (patch != 0) {
        written = snprintf(buffer, sizeof(buffer), "%u.%u.%u",
                           major, minor, patch);
    } else if (minor != 0) {
        written = snprintf(buffer, sizeof(buffer), "%u.%u", major, minor);
    } else {
        written = snprintf(buffer, sizeof(buffer), "%u", major);
    }
    if (written < 0 || (size_t)written >= sizeof(buffer)) {
        return NULL;
    }
    return ml_strdup_safe(buffer);
}

char *ml_format_source_version(const ml_image *image) {
    char buffer[128];
    size_t last = 5;
    int written;
    if (image == NULL) {
        return NULL;
    }
    while (last > 1 && image->source_version[last - 1] == 0) {
        --last;
    }
    switch (last) {
        case 1:
            written = snprintf(buffer, sizeof(buffer), "%u",
                               image->source_version[0]);
            break;
        case 2:
            written = snprintf(buffer, sizeof(buffer), "%u.%u",
                               image->source_version[0],
                               image->source_version[1]);
            break;
        case 3:
            written = snprintf(buffer, sizeof(buffer), "%u.%u.%u",
                               image->source_version[0],
                               image->source_version[1],
                               image->source_version[2]);
            break;
        case 4:
            written = snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u",
                               image->source_version[0],
                               image->source_version[1],
                               image->source_version[2],
                               image->source_version[3]);
            break;
        default:
            written = snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u.%u",
                               image->source_version[0],
                               image->source_version[1],
                               image->source_version[2],
                               image->source_version[3],
                               image->source_version[4]);
            break;
    }
    if (written < 0 || (size_t)written >= sizeof(buffer)) {
        return NULL;
    }
    return ml_strdup_safe(buffer);
}

char *ml_format_uuid(const ml_image *image) {
    char buffer[37];
    int written;
    if (image == NULL || !image->has_uuid) {
        return ml_strdup_safe("");
    }
    written = snprintf(buffer, sizeof(buffer),
                       "%02x%02x%02x%02x-%02x%02x-%02x%02x-"
                       "%02x%02x-%02x%02x%02x%02x%02x%02x",
                       image->uuid[0], image->uuid[1], image->uuid[2],
                       image->uuid[3], image->uuid[4], image->uuid[5],
                       image->uuid[6], image->uuid[7], image->uuid[8],
                       image->uuid[9], image->uuid[10], image->uuid[11],
                       image->uuid[12], image->uuid[13], image->uuid[14],
                       image->uuid[15]);
    if (written != 36) {
        return NULL;
    }
    return ml_strdup_safe(buffer);
}

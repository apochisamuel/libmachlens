#include "internal.h"

#include <string.h>

int ml_parse_uuid_command(ml_image *image, const uint8_t *data, size_t size) {
    if (image == NULL || data == NULL || size < 24) {
        return 0;
    }
    memcpy(image->uuid, data + 8, 16);
    image->has_uuid = 1;
    return 1;
}

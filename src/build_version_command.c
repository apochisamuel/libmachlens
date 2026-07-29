#include "internal.h"

int ml_parse_build_version_command(ml_image *image, const uint8_t *data,
                                   size_t size) {
    ml_reader reader;
    uint32_t command;
    uint32_t command_size;
    uint32_t tool_count;
    uint32_t index;
    if (image == NULL || data == NULL || size < 24) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u32(&reader, &image->platform) ||
        !ml_reader_u32(&reader, &image->minimum_os) ||
        !ml_reader_u32(&reader, &image->sdk) ||
        !ml_reader_u32(&reader, &tool_count) ||
        command != ML_LC_BUILD_VERSION || command_size != size ||
        tool_count > (size - 24) / 8) {
        return 0;
    }
    for (index = 0; index < tool_count; ++index) {
        ml_build_tool_record tool;
        if (!ml_reader_u32(&reader, &tool.tool) ||
            !ml_reader_u32(&reader, &tool.version) ||
            !ml_vec_append((void **)&image->build_tools,
                           &image->build_tool_count,
                           &image->build_tool_capacity,
                           &tool, sizeof(tool))) {
            return 0;
        }
    }
    return 1;
}

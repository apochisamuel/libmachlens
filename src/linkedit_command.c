#include "internal.h"

int ml_parse_linkedit_data_command(ml_image *image, uint32_t command,
                                   const uint8_t *data, size_t size) {
    ml_reader reader;
    uint32_t parsed_command;
    uint32_t command_size;
    uint32_t offset;
    uint32_t length;
    if (image == NULL || data == NULL || size < 16) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &parsed_command) ||
        !ml_reader_u32(&reader, &command_size) ||
        !ml_reader_u32(&reader, &offset) ||
        !ml_reader_u32(&reader, &length) ||
        parsed_command != command || command_size != size) {
        return 0;
    }
    switch (command) {
        case ML_LC_CODE_SIGNATURE:
            image->code_signature_offset = offset;
            image->code_signature_size = length;
            break;
        case ML_LC_FUNCTION_STARTS:
            image->function_starts_offset = offset;
            image->function_starts_size = length;
            break;
        case ML_LC_DATA_IN_CODE:
            image->data_in_code_offset = offset;
            image->data_in_code_size = length;
            break;
        case ML_LC_DYLD_EXPORTS_TRIE:
            image->exports_trie_offset = offset;
            image->exports_trie_size = length;
            break;
        case ML_LC_DYLD_CHAINED_FIXUPS:
            image->chained_fixups_offset = offset;
            image->chained_fixups_size = length;
            break;
        default:
            break;
    }
    return 1;
}

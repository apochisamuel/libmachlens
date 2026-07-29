#include "internal.h"

#include <string.h>

static int ml_record_load_command(ml_image *image, uint32_t command,
                                  uint32_t size, uint64_t offset) {
    ml_load_command_record record;
    record.command = command;
    record.size = size;
    record.file_offset = offset;
    return ml_vec_append((void **)&image->load_commands,
                         &image->load_command_count,
                         &image->load_command_capacity,
                         &record, sizeof(record));
}

static int ml_parse_version_min_command(ml_image *image,
                                        uint32_t command,
                                        const uint8_t *data,
                                        size_t size) {
    ml_reader reader;
    uint32_t ignored;
    if (size < 16) {
        return 0;
    }
    ml_reader_init(&reader, data, size, image->endian);
    if (!ml_reader_u32(&reader, &ignored) || !ml_reader_u32(&reader, &ignored) ||
        !ml_reader_u32(&reader, &image->minimum_os) ||
        !ml_reader_u32(&reader, &image->sdk)) {
        return 0;
    }
    switch (command) {
        case ML_LC_VERSION_MIN_MACOSX: image->platform = 1; break;
        case ML_LC_VERSION_MIN_IPHONEOS: image->platform = 2; break;
        case ML_LC_VERSION_MIN_TVOS: image->platform = 3; break;
        case ML_LC_VERSION_MIN_WATCHOS: image->platform = 4; break;
        default: break;
    }
    return 1;
}

int ml_parse_load_commands(ml_image *image) {
    const uint8_t *commands;
    ml_reader reader;
    uint32_t index;
    if (image == NULL ||
        !ml_image_slice(image, image->command_offset, image->command_bytes,
                        &commands)) {
        return 0;
    }
    ml_reader_init(&reader, commands, image->command_bytes, image->endian);
    for (index = 0; index < image->command_count; ++index) {
        size_t command_start = reader.offset;
        uint32_t command;
        uint32_t command_size;
        const uint8_t *payload;
        int parsed = 1;
        if (!ml_reader_u32(&reader, &command) ||
            !ml_reader_u32(&reader, &command_size)) {
            ml_context_set(image->owner->ctx, ML_ERR_TRUNCATED,
                           "truncated load command header at index %u", index);
            return 0;
        }
        if (command_size < 8 || command_size > ml_reader_remaining(&reader) + 8) {
            ml_context_set(image->owner->ctx, ML_ERR_FORMAT,
                           "invalid load command size at index %u", index);
            return 0;
        }
        payload = commands + command_start;
        if (!ml_record_load_command(image, command, command_size,
                                    image->command_offset + command_start)) {
            ml_context_set(image->owner->ctx, ML_ERR_NOMEM,
                           "unable to record load command %u", index);
            return 0;
        }
        switch (command) {
            case ML_LC_SEGMENT:
                parsed = ml_parse_segment_command(image, payload, command_size, 0);
                break;
            case ML_LC_SEGMENT_64:
                parsed = ml_parse_segment_command(image, payload, command_size, 1);
                break;
            case ML_LC_SYMTAB:
                parsed = ml_parse_symtab_command(image, payload, command_size);
                break;
            case ML_LC_DYSYMTAB:
                parsed = ml_parse_dysymtab_command(image, payload, command_size);
                break;
            case ML_LC_DYLD_INFO:
            case ML_LC_DYLD_INFO_ONLY:
                parsed = ml_parse_dyld_info_command(image, payload, command_size);
                break;
            case ML_LC_CODE_SIGNATURE:
            case ML_LC_FUNCTION_STARTS:
            case ML_LC_DATA_IN_CODE:
            case ML_LC_DYLIB_CODE_SIGN_DRS:
            case ML_LC_LINKER_OPTIMIZATION_HINT:
            case ML_LC_DYLD_EXPORTS_TRIE:
            case ML_LC_DYLD_CHAINED_FIXUPS:
                parsed = ml_parse_linkedit_data_command(image, command,
                                                        payload, command_size);
                break;
            case ML_LC_LOAD_DYLIB:
            case ML_LC_ID_DYLIB:
            case ML_LC_LOAD_WEAK_DYLIB:
            case ML_LC_REEXPORT_DYLIB:
            case ML_LC_LAZY_LOAD_DYLIB:
            case ML_LC_LOAD_UPWARD_DYLIB:
                parsed = ml_parse_dylib_command(image, command, payload,
                                                command_size);
                break;
            case ML_LC_RPATH:
            case ML_LC_LOAD_DYLINKER:
            case ML_LC_ID_DYLINKER:
            case ML_LC_DYLD_ENVIRONMENT:
                parsed = ml_parse_string_command(image, command, payload,
                                                 command_size);
                break;
            case ML_LC_LINKER_OPTION:
                parsed = ml_parse_linker_option_command(image, payload,
                                                        command_size);
                break;
            case ML_LC_NOTE:
                parsed = ml_parse_note_command(image, payload, command_size);
                break;
            case ML_LC_UUID:
                parsed = ml_parse_uuid_command(image, payload, command_size);
                break;
            case ML_LC_MAIN:
                parsed = ml_parse_main_command(image, payload, command_size);
                break;
            case ML_LC_BUILD_VERSION:
                parsed = ml_parse_build_version_command(image, payload,
                                                        command_size);
                break;
            case ML_LC_SOURCE_VERSION:
                parsed = ml_parse_source_version_command(image, payload,
                                                         command_size);
                break;
            case ML_LC_ENCRYPTION_INFO:
            case ML_LC_ENCRYPTION_INFO_64:
                parsed = ml_parse_encryption_command(image, payload,
                                                     command_size);
                break;
            case ML_LC_THREAD:
            case ML_LC_UNIXTHREAD:
                parsed = ml_parse_thread_command(image, payload, command_size);
                break;
            case ML_LC_VERSION_MIN_MACOSX:
            case ML_LC_VERSION_MIN_IPHONEOS:
            case ML_LC_VERSION_MIN_TVOS:
            case ML_LC_VERSION_MIN_WATCHOS:
                parsed = ml_parse_version_min_command(image, command, payload,
                                                      command_size);
                break;
            default:
                parsed = 1;
                break;
        }
        if (!parsed) {
            ml_image_diag(image, ML_ERR_FORMAT,
                          image->command_offset + command_start,
                          "unable to decode %s", ml_load_command_name(command));
        }
        if (!ml_reader_seek(&reader, command_start + command_size)) {
            ml_context_set(image->owner->ctx, ML_ERR_RANGE,
                           "load command cursor exceeded command area");
            return 0;
        }
    }
    if (reader.offset != image->command_bytes) {
        ml_image_diag(image, ML_ERR_FORMAT,
                      image->command_offset + reader.offset,
                      "load command byte count does not match command records");
    }
    return 1;
}

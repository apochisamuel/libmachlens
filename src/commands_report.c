#include "internal.h"

#include <string.h>

#include <inttypes.h>

char *ml_image_commands_report(const ml_image *image) {
    ml_string_builder builder;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Load commands for %s\n"
                       "====================%.*s\n",
                       ml_image_arch_name(image),
                       (int)strlen(ml_image_arch_name(image)),
                       "================================")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->load_command_count; ++index) {
        const ml_load_command_record *command = &image->load_commands[index];
        if (!ml_sb_appendf(&builder,
                           "%4zu  %-30s  command=%#010x  size=%-6u  offset=%#" PRIx64 "\n",
                           index,
                           ml_load_command_name(command->command),
                           command->command,
                           command->size,
                           command->file_offset)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (image->rpath_count != 0) {
        if (!ml_sb_append(&builder, "\nRuntime search paths\n--------------------\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < image->rpath_count; ++index) {
            if (!ml_sb_appendf(&builder, "  %zu: %s\n", index,
                               image->rpaths[index])) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
    }
    if (image->dylinker_path != NULL) {
        if (!ml_sb_appendf(&builder, "\nDynamic linker: %s\n",
                           image->dylinker_path)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (image->linker_option_count != 0) {
        if (!ml_sb_append(&builder, "\nLinker options and dyld environment\n"
                          "-----------------------------------\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < image->linker_option_count; ++index) {
            if (!ml_sb_appendf(&builder, "  %s\n",
                               image->linker_options[index])) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
    }
    if (image->note_count != 0) {
        if (!ml_sb_append(&builder, "\nNotes\n-----\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < image->note_count; ++index) {
            const ml_note_record *note = &image->notes[index];
            if (!ml_sb_appendf(&builder,
                               "  %-16s offset=%#" PRIx64 " size=%#" PRIx64 "\n",
                               note->owner, note->offset, note->size)) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
    }
    return ml_sb_take(&builder);
}

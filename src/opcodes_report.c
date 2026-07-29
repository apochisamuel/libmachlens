#include "internal.h"

#include <inttypes.h>

char *ml_image_opcodes_report(const ml_image *image) {
    ml_string_builder builder;
    ml_opcode_summary bind;
    ml_opcode_summary rebase;
    if (image == NULL) {
        return NULL;
    }
    bind = ml_image_binding_opcode_summary(image);
    rebase = ml_image_rebase_opcode_summary(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Dynamic-loader opcode streams\n"
                       "=============================\n"
                       "Bindings\n"
                       "  bytes:                  %u\n"
                       "  opcodes:                %zu\n"
                       "  state changes:          %zu\n"
                       "  binding actions:        %zu\n"
                       "  done markers:           %zu\n"
                       "  malformed sequences:    %zu\n"
                       "  maximum segment offset: %#" PRIx64 "\n\n"
                       "Rebases\n"
                       "  bytes:                  %u\n"
                       "  opcodes:                %zu\n"
                       "  state changes:          %zu\n"
                       "  rebase actions:         %zu\n"
                       "  done markers:           %zu\n"
                       "  malformed sequences:    %zu\n"
                       "  maximum segment offset: %#" PRIx64 "\n",
                       image->bind_size + image->weak_bind_size +
                           image->lazy_bind_size,
                       bind.opcodes,
                       bind.state_changes,
                       bind.actions,
                       bind.done_markers,
                       bind.malformed_sequences,
                       bind.maximum_segment_offset,
                       image->rebase_size,
                       rebase.opcodes,
                       rebase.state_changes,
                       rebase.actions,
                       rebase.done_markers,
                       rebase.malformed_sequences,
                       rebase.maximum_segment_offset)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

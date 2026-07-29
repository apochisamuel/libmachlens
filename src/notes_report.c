#include "internal.h"

#include <inttypes.h>

char *ml_image_notes_report(const ml_image *image) {
    ml_string_builder builder;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Mach-O notes\n"
                       "============\n"
                       "count=%zu\n"
                       "%-5s %-16s %-18s %-18s %-18s\n",
                       image->note_count,
                       "index", "owner", "offset", "size", "end")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->note_count; ++index) {
        const ml_note_record *note = &image->notes[index];
        uint64_t end = 0;
        (void)ml_u64_add(note->offset, note->size, &end);
        if (!ml_sb_appendf(&builder,
                           "%-5zu %-16s %#-18" PRIx64 " %#-18" PRIx64
                           " %#-18" PRIx64 "\n",
                           index,
                           note->owner,
                           note->offset,
                           note->size,
                           end)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    return ml_sb_take(&builder);
}

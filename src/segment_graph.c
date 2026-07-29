#include "internal.h"

#include <inttypes.h>

static int ml_segment_dot_label(ml_string_builder *builder,
                                const ml_segment_record *segment) {
    return ml_sb_appendf(builder,
                         "\"{%s|vm %#" PRIx64 "+%#" PRIx64
                         "|file %#" PRIx64 "+%#" PRIx64
                         "|prot %s|sections %zu}\"",
                         segment->name,
                         segment->vm_address,
                         segment->vm_size,
                         segment->file_offset,
                         segment->file_size,
                         ml_vm_protection_name(segment->initial_protection),
                         segment->section_count);
}

char *ml_image_segment_graph_dot(const ml_image *image) {
    ml_string_builder builder;
    size_t segment_index;
    size_t section_index;
    if (image == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_append(&builder,
                      "digraph machlens_layout {\n"
                      "  rankdir=LR;\n"
                      "  graph [fontname=\"Helvetica\"];\n"
                      "  node [shape=record,fontname=\"Helvetica\"];\n"
                      "  edge [fontname=\"Helvetica\"];\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (segment_index = 0; segment_index < image->segment_count;
         ++segment_index) {
        if (!ml_sb_appendf(&builder, "  segment_%zu [label=", segment_index) ||
            !ml_segment_dot_label(&builder, &image->segments[segment_index]) ||
            !ml_sb_append(&builder, "];\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (segment_index != 0 &&
            !ml_sb_appendf(&builder,
                           "  segment_%zu -> segment_%zu [style=invis,weight=10];\n",
                           segment_index - 1, segment_index)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    for (section_index = 0; section_index < image->section_count;
         ++section_index) {
        const ml_section_record *section = &image->sections[section_index];
        if (!ml_sb_appendf(&builder,
                           "  section_%zu [shape=box,label=\"%s,%s\\n%" PRIu64
                           " bytes\\n%s\"];\n",
                           section_index,
                           section->segment_name,
                           section->section_name,
                           section->size,
                           ml_section_type_name(section->flags))) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        if (section->segment_index < image->segment_count &&
            !ml_sb_appendf(&builder,
                           "  segment_%zu -> section_%zu [label=\"contains\"];\n",
                           section->segment_index, section_index)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (!ml_sb_append(&builder, "}\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

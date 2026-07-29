#include "internal.h"

#include <inttypes.h>

static int ml_dot_string(ml_string_builder *builder, const char *text) {
    const unsigned char *cursor = (const unsigned char *)(text != NULL ? text : "");
    if (!ml_sb_append(builder, "\"")) {
        return 0;
    }
    while (*cursor != 0) {
        unsigned char ch = *cursor++;
        if (ch == '\\' || ch == '"') {
            char slash = '\\';
            char value = (char)ch;
            if (!ml_sb_append_n(builder, &slash, 1) ||
                !ml_sb_append_n(builder, &value, 1)) {
                return 0;
            }
        } else if (ch == '\n' || ch == '\r') {
            if (!ml_sb_append(builder, "\\n")) {
                return 0;
            }
        } else if (ch < 0x20u) {
            if (!ml_sb_appendf(builder, "\\x%02x", ch)) {
                return 0;
            }
        } else {
            char value = (char)ch;
            if (!ml_sb_append_n(builder, &value, 1)) {
                return 0;
            }
        }
    }
    return ml_sb_append(builder, "\"");
}

static const char *ml_dot_edge_style(uint32_t command) {
    switch (command) {
        case ML_LC_LOAD_WEAK_DYLIB: return "dashed";
        case ML_LC_REEXPORT_DYLIB: return "bold";
        case ML_LC_LOAD_UPWARD_DYLIB: return "dotted";
        case ML_LC_LAZY_LOAD_DYLIB: return "dashed";
        default: return "solid";
    }
}

char *ml_image_dependency_graph_dot(const ml_image *image) {
    ml_string_builder builder;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    ml_sb_init(&builder);
    if (!ml_sb_append(&builder,
                      "digraph machlens_dependencies {\n"
                      "  rankdir=LR;\n"
                      "  graph [fontname=\"Helvetica\"];\n"
                      "  node [shape=box,fontname=\"Helvetica\"];\n"
                      "  edge [fontname=\"Helvetica\"];\n"
                      "  image [shape=ellipse,label=") ||
        !ml_dot_string(&builder, ml_image_arch_name(image)) ||
        !ml_sb_append(&builder, "];\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < image->dylib_count; ++index) {
        const ml_dylib_record *dylib = &image->dylibs[index];
        if (!ml_sb_appendf(&builder, "  dep_%zu [label=", index) ||
            !ml_dot_string(&builder, dylib->name) ||
            !ml_sb_append(&builder, "];\n") ||
            !ml_sb_appendf(&builder,
                           "  image -> dep_%zu [style=%s,label=",
                           index, ml_dot_edge_style(dylib->command)) ||
            !ml_dot_string(&builder, ml_load_command_name(dylib->command)) ||
            !ml_sb_append(&builder, "];\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (image->rpath_count != 0) {
        if (!ml_sb_append(&builder,
                          "  subgraph cluster_rpaths {\n"
                          "    label=\"runtime search paths\";\n"
                          "    style=dashed;\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
        for (index = 0; index < image->rpath_count; ++index) {
            if (!ml_sb_appendf(&builder, "    rpath_%zu [shape=note,label=", index) ||
                !ml_dot_string(&builder, image->rpaths[index]) ||
                !ml_sb_append(&builder, "];\n") ||
                !ml_sb_appendf(&builder,
                               "    image -> rpath_%zu [style=dotted];\n",
                               index)) {
                ml_sb_dispose(&builder);
                return NULL;
            }
        }
        if (!ml_sb_append(&builder, "  }\n")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (!ml_sb_appendf(&builder,
                       "  metadata [shape=record,label=\"{bindings|%zu}|{imports|%zu}|{rpaths|%zu}\"];\n"
                       "  image -> metadata [style=dashed];\n"
                       "}\n",
                       image->binding_count,
                       image->dylib_count,
                       image->rpath_count)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

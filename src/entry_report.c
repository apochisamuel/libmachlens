#include "internal.h"

#include <inttypes.h>
#include <string.h>

char *ml_image_entry_report(const ml_image *image) {
    ml_string_builder builder;
    uint64_t text_vm = 0;
    uint64_t entry_vm = 0;
    uint64_t entry_file = 0;
    ml_symbol_info symbol;
    uint64_t symbol_offset = 0;
    size_t index;
    const ml_section_record *section = NULL;
    if (image == NULL) {
        return NULL;
    }
    for (index = 0; index < image->segment_count; ++index) {
        if (strcmp(image->segments[index].name, "__TEXT") == 0) {
            text_vm = image->segments[index].vm_address;
            break;
        }
    }
    if (image->has_entry_point) {
        (void)ml_u64_add(text_vm, image->entry_point, &entry_vm);
        (void)ml_image_vm_to_file(image, entry_vm, &entry_file);
        for (index = 0; index < image->section_count; ++index) {
            uint64_t end;
            if (ml_u64_add(image->sections[index].address,
                           image->sections[index].size, &end) &&
                entry_vm >= image->sections[index].address && entry_vm < end) {
                section = &image->sections[index];
                break;
            }
        }
    }
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Entry-point mapping\n"
                       "===================\n"
                       "LC_MAIN present:       %s\n"
                       "entry file offset:     %#" PRIx64 "\n"
                       "__TEXT virtual base:   %#" PRIx64 "\n"
                       "entry virtual address: %#" PRIx64 "\n"
                       "mapped file offset:    %#" PRIx64 "\n"
                       "stack size:            %#" PRIx64 "\n"
                       "containing section:    %s,%s\n",
                       image->has_entry_point ? "yes" : "no",
                       image->entry_point,
                       text_vm,
                       entry_vm,
                       entry_file,
                       image->stack_size,
                       section != NULL ? section->segment_name : "",
                       section != NULL ? section->section_name : "")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    if (entry_vm != 0 &&
        ml_image_find_symbol_by_address(image, entry_vm,
                                        &symbol, &symbol_offset)) {
        if (!ml_sb_appendf(&builder,
                           "nearest symbol:       %s + %#" PRIx64 "\n",
                           symbol.name != NULL ? symbol.name : "",
                           symbol_offset)) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    } else if (!ml_sb_append(&builder, "nearest symbol:       none\n")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

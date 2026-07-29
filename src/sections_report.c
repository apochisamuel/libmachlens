#include "internal.h"

#include <string.h>

#include <inttypes.h>

char *ml_image_sections_report(const ml_image *image, size_t maximum_sections) {
    ml_string_builder builder;
    size_t limit;
    size_t index;
    if (image == NULL) {
        return NULL;
    }
    limit = maximum_sections == 0 || maximum_sections > image->section_count
        ? image->section_count : maximum_sections;
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Sections for %s\n"
                       "================\n"
                       "%-4s %-16s %-16s %-18s %-12s %-10s %-8s %-12s %s\n",
                       ml_image_arch_name(image),
                       "#", "segment", "section", "address", "size",
                       "offset", "align", "relocations", "type")) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    for (index = 0; index < limit; ++index) {
        const ml_section_record *section = &image->sections[index];
        ml_section_metrics metrics;
        if (!ml_image_section_metrics(image, index, &metrics)) {
            memset(&metrics, 0, sizeof(metrics));
        }
        if (!ml_sb_appendf(&builder,
                           "%-4zu %-16s %-16s %#-18" PRIx64
                           " %-12" PRIu64 " %#-10x %-8u %-12u %s\n"
                           "     crc32=%08x entropy=%.3f printable=%zu zero=%zu distinct=%zu %s%s\n",
                           index,
                           section->segment_name,
                           section->section_name,
                           section->address,
                           section->size,
                           section->offset,
                           section->alignment,
                           section->relocation_count,
                           ml_section_type_name(section->flags),
                           metrics.crc32,
                           metrics.byte_entropy,
                           metrics.printable_bytes,
                           metrics.zero_bytes,
                           metrics.distinct_bytes,
                           metrics.executable ? "execute " : "",
                           metrics.writable ? "write" : "")) {
            ml_sb_dispose(&builder);
            return NULL;
        }
    }
    if (limit < image->section_count &&
        !ml_sb_appendf(&builder, "... %zu section(s) omitted\n",
                       image->section_count - limit)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

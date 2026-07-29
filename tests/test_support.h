#ifndef MACHLENS_TEST_SUPPORT_H
#define MACHLENS_TEST_SUPPORT_H

#include <stddef.h>
#include <stdint.h>

size_t test_make_minimal_macho64(uint8_t *bytes, size_t capacity,
                                 uint32_t cpu_type);
size_t test_make_segment_macho64(uint8_t *bytes, size_t capacity,
                                 uint32_t cpu_type);
size_t test_make_universal(uint8_t *bytes, size_t capacity);
size_t test_make_metadata_macho64(uint8_t *bytes, size_t capacity,
                                  uint32_t cpu_type);
size_t test_make_cstring_section_macho64(uint8_t *bytes, size_t capacity,
                                         uint32_t cpu_type);

size_t test_make_dependency_macho64(uint8_t *bytes, size_t capacity,
                                    uint32_t cpu_type);
size_t test_make_symbol_macho64(uint8_t *bytes, size_t capacity,
                                uint32_t cpu_type);
size_t test_make_note_macho64(uint8_t *bytes, size_t capacity,
                              uint32_t cpu_type);

#endif

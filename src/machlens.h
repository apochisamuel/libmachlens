#ifndef MACHLENS_H
#define MACHLENS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ml_context ml_context;
typedef struct ml_file ml_file;
typedef struct ml_image ml_image;

typedef enum ml_status {
    ML_OK = 0,
    ML_ERR_ARGUMENT = 1,
    ML_ERR_NOMEM = 2,
    ML_ERR_TRUNCATED = 3,
    ML_ERR_FORMAT = 4,
    ML_ERR_UNSUPPORTED = 5,
    ML_ERR_RANGE = 6,
    ML_ERR_STATE = 7
} ml_status;

typedef enum ml_endian {
    ML_ENDIAN_LITTLE = 0,
    ML_ENDIAN_BIG = 1
} ml_endian;

typedef enum ml_cpu_family {
    ML_CPU_UNKNOWN = 0,
    ML_CPU_X86,
    ML_CPU_X86_64,
    ML_CPU_ARM,
    ML_CPU_ARM64,
    ML_CPU_POWERPC,
    ML_CPU_POWERPC64
} ml_cpu_family;

typedef struct ml_segment_info {
    char name[17];
    uint64_t vm_address;
    uint64_t vm_size;
    uint64_t file_offset;
    uint64_t file_size;
    uint32_t max_protection;
    uint32_t initial_protection;
    uint32_t flags;
    size_t section_count;
} ml_segment_info;

typedef struct ml_section_info {
    char segment_name[17];
    char section_name[17];
    uint64_t address;
    uint64_t size;
    uint32_t offset;
    uint32_t alignment;
    uint32_t relocation_offset;
    uint32_t relocation_count;
    uint32_t flags;
} ml_section_info;

typedef struct ml_symbol_info {
    const char *name;
    uint64_t value;
    uint8_t type;
    uint8_t section;
    uint16_t description;
    int external;
    int undefined_symbol;
} ml_symbol_info;

typedef struct ml_export_info {
    const char *name;
    uint64_t address;
    uint64_t flags;
    const char *reexport_name;
} ml_export_info;

typedef struct ml_binding_info {
    const char *symbol_name;
    const char *segment_name;
    uint64_t address;
    int64_t addend;
    int library_ordinal;
    uint8_t type;
    uint8_t flags;
    int weak;
    int lazy;
} ml_binding_info;




typedef struct ml_security_summary {
    int position_independent;
    int no_heap_execution;
    int allows_stack_execution;
    int has_code_signature;
    int has_entitlements;
    int encrypted;
    int has_writable_executable_segment;
    int has_executable_data_section;
    int has_pagezero_segment;
    int has_restricted_section;
    size_t executable_segments;
    size_t writable_executable_segments;
} ml_security_summary;

typedef struct ml_comparison_summary {
    int same_architecture;
    int same_file_type;
    int same_uuid;
    int same_platform;
    ptrdiff_t segment_delta;
    ptrdiff_t section_delta;
    ptrdiff_t symbol_delta;
    ptrdiff_t export_delta;
    ptrdiff_t dependency_delta;
    size_t shared_segment_names;
    size_t shared_section_names;
    size_t shared_symbol_names;
    size_t shared_export_names;
} ml_comparison_summary;

typedef struct ml_opcode_summary {
    size_t opcodes;
    size_t state_changes;
    size_t actions;
    size_t done_markers;
    size_t malformed_sequences;
    uint64_t maximum_segment_offset;
} ml_opcode_summary;

typedef struct ml_export_summary {
    size_t regular_exports;
    size_t reexports;
    size_t weak_definitions;
    size_t resolver_exports;
    size_t empty_names;
    size_t duplicate_names;
    size_t underscore_prefixed;
    size_t objc_symbols;
    size_t swift_symbols;
    size_t longest_name;
} ml_export_summary;

typedef struct ml_relocation_summary {
    size_t external_relocations;
    size_t local_relocations;
    size_t scattered_relocations;
    size_t pc_relative_relocations;
    size_t unresolved_external_relocations;
    size_t by_width[4];
    size_t by_type[16];
} ml_relocation_summary;

typedef struct ml_string_summary {
    size_t string_count;
    size_t total_characters;
    size_t longest_string;
    size_t empty_slots;
    size_t ascii_strings;
    size_t utf8_like_strings;
    size_t path_like_strings;
    size_t selector_like_strings;
} ml_string_summary;

typedef struct ml_load_command_info {
    uint32_t command;
    uint32_t size;
    uint64_t file_offset;
    const char *name;
} ml_load_command_info;

typedef struct ml_note_info {
    char owner[17];
    uint64_t offset;
    uint64_t size;
} ml_note_info;

typedef struct ml_code_directory_info {
    uint32_t version;
    uint32_t flags;
    uint32_t code_limit;
    uint64_t code_limit_64;
    uint32_t special_slot_count;
    uint32_t code_slot_count;
    uint8_t hash_size;
    uint8_t hash_type;
    uint8_t platform;
    uint8_t page_size_log2;
    const char *identifier;
    const char *team_identifier;
    uint64_t executable_segment_base;
    uint64_t executable_segment_limit;
    uint64_t executable_segment_flags;
} ml_code_directory_info;

typedef struct ml_unwind_summary {
    uint32_t compact_version;
    size_t compact_common_encodings;
    size_t compact_personalities;
    size_t compact_index_entries;
    size_t eh_frame_cie_records;
    size_t eh_frame_fde_records;
    size_t eh_frame_terminators;
    size_t malformed_records;
} ml_unwind_summary;

typedef struct ml_dependency_summary {
    size_t regular_dependencies;
    size_t weak_dependencies;
    size_t reexported_dependencies;
    size_t upward_dependencies;
    size_t lazy_dependencies;
    size_t system_dependencies;
    size_t framework_dependencies;
    size_t relative_dependencies;
    size_t absolute_dependencies;
} ml_dependency_summary;

typedef struct ml_relocation_info {
    uint64_t address;
    uint32_t symbol_index;
    uint8_t length;
    uint8_t type;
    uint8_t pc_relative;
    uint8_t external;
    uint8_t scattered;
    int32_t value;
    size_t section_index;
} ml_relocation_info;

typedef struct ml_dylib_info {
    const char *name;
    uint32_t command;
    uint32_t timestamp;
    uint32_t current_version;
    uint32_t compatibility_version;
} ml_dylib_info;

typedef struct ml_function_info {
    uint64_t address;
    uint64_t size;
    size_t containing_section;
} ml_function_info;

typedef struct ml_code_blob_info {
    uint32_t type;
    uint32_t magic;
    uint32_t offset;
    uint32_t length;
    const char *type_name;
} ml_code_blob_info;

typedef struct ml_diagnostic_info {
    ml_status status;
    uint64_t offset;
    const char *message;
} ml_diagnostic_info;

typedef struct ml_range_info {
    const char *name;
    uint64_t offset;
    uint64_t size;
    uint32_t category;
} ml_range_info;

typedef struct ml_segment_metrics {
    uint64_t mapped_bytes;
    uint64_t zero_fill_bytes;
    uint64_t executable_bytes;
    uint64_t writable_bytes;
    uint64_t readable_bytes;
    uint64_t alignment_waste;
    size_t regular_sections;
    size_t zero_fill_sections;
    size_t relocation_entries;
} ml_segment_metrics;

typedef struct ml_section_metrics {
    uint32_t crc32;
    uint64_t fnv1a64;
    double byte_entropy;
    size_t zero_bytes;
    size_t printable_bytes;
    size_t distinct_bytes;
    int file_backed;
    int executable;
    int writable;
} ml_section_metrics;

typedef struct ml_layout_summary {
    uint64_t virtual_span;
    uint64_t file_backed_bytes;
    uint64_t mapped_file_bytes;
    uint64_t virtual_zero_fill_bytes;
    uint64_t unmapped_file_bytes;
    uint64_t largest_file_gap;
    uint64_t largest_virtual_gap;
    size_t executable_segments;
    size_t writable_segments;
    size_t read_only_segments;
    size_t overlapping_segments;
} ml_layout_summary;

typedef struct ml_symbol_summary {
    size_t local_symbols;
    size_t external_symbols;
    size_t undefined_symbols;
    size_t absolute_symbols;
    size_t section_symbols;
    size_t debugging_symbols;
    size_t weak_definitions;
    size_t weak_references;
    size_t stab_symbols;
    size_t empty_names;
    uint64_t lowest_value;
    uint64_t highest_value;
} ml_symbol_summary;

typedef struct ml_linkedit_summary {
    uint64_t declared_bytes;
    uint64_t decoded_records;
    size_t export_nodes;
    size_t bindings;
    size_t rebases;
    size_t function_starts;
    size_t data_in_code_entries;
    size_t signature_blobs;
    size_t malformed_records;
} ml_linkedit_summary;

typedef struct ml_language_runtime_summary {
    size_t objc_sections;
    size_t objc_class_records;
    size_t objc_category_records;
    size_t objc_protocol_records;
    size_t swift_sections;
    uint64_t swift_metadata_bytes;
    size_t unwind_sections;
    uint64_t unwind_bytes;
} ml_language_runtime_summary;

typedef struct ml_validation_summary {
    size_t errors;
    size_t warnings;
    size_t overlapping_ranges;
    size_t unresolved_symbols;
    size_t malformed_linkedit_records;
} ml_validation_summary;

ml_context *ml_context_create(void);
void ml_context_destroy(ml_context *ctx);
const char *ml_context_error(const ml_context *ctx);
ml_status ml_context_status(const ml_context *ctx);

ml_file *ml_file_parse_memory(ml_context *ctx, const uint8_t *data, size_t size);
void ml_file_destroy(ml_file *file);

size_t ml_file_image_count(const ml_file *file);
const ml_image *ml_file_image_at(const ml_file *file, size_t index);
int ml_file_is_universal(const ml_file *file);

const char *ml_image_arch_name(const ml_image *image);
ml_cpu_family ml_image_cpu_family(const ml_image *image);
uint32_t ml_image_cpu_type(const ml_image *image);
uint32_t ml_image_cpu_subtype(const ml_image *image);
uint32_t ml_image_file_type(const ml_image *image);
uint32_t ml_image_flags(const ml_image *image);
int ml_image_is_64_bit(const ml_image *image);
ml_endian ml_image_endian(const ml_image *image);

size_t ml_image_segment_count(const ml_image *image);
int ml_image_segment_info(const ml_image *image, size_t index, ml_segment_info *out);
size_t ml_image_section_count(const ml_image *image);
int ml_image_section_info(const ml_image *image, size_t index, ml_section_info *out);
size_t ml_image_symbol_count(const ml_image *image);
int ml_image_symbol_info(const ml_image *image, size_t index, ml_symbol_info *out);
size_t ml_image_export_count(const ml_image *image);
int ml_image_export_info(const ml_image *image, size_t index, ml_export_info *out);
size_t ml_image_binding_count(const ml_image *image);
int ml_image_binding_info(const ml_image *image, size_t index, ml_binding_info *out);

int ml_image_find_symbol(const ml_image *image, const char *name,
                         ml_symbol_info *out);
int ml_image_find_export(const ml_image *image, const char *name,
                         ml_export_info *out);
int ml_image_find_section(const ml_image *image,
                          const char *segment_name,
                          const char *section_name,
                          ml_section_info *out);

int ml_image_address_to_file_offset(const ml_image *image,
                                    uint64_t address,
                                    uint64_t *file_offset);
int ml_image_file_offset_to_address(const ml_image *image,
                                    uint64_t file_offset,
                                    uint64_t *address);




ml_security_summary ml_image_security_summary(const ml_image *image);
ml_comparison_summary ml_compare_images(const ml_image *left,
                                        const ml_image *right);
ml_opcode_summary ml_image_binding_opcode_summary(const ml_image *image);
ml_opcode_summary ml_image_rebase_opcode_summary(const ml_image *image);
ml_export_summary ml_image_export_summary(const ml_image *image);
ml_relocation_summary ml_image_relocation_summary(const ml_image *image);
ml_string_summary ml_image_string_summary(const ml_image *image,
                                          const char *segment_name,
                                          const char *section_name);
uint64_t ml_image_structural_fingerprint(const ml_image *image);
uint64_t ml_file_structural_fingerprint(const ml_file *file);
int ml_image_find_symbol_by_address(const ml_image *image, uint64_t address,
                                    ml_symbol_info *out, uint64_t *offset);





char *ml_image_command_histogram_report(const ml_image *image);
char *ml_image_command_inventory_json(const ml_image *image);
char *ml_image_overlap_report(const ml_image *image);
char *ml_image_data_quality_report(const ml_image *image);

char *ml_image_dependency_graph_dot(const ml_image *image);
char *ml_image_segment_graph_dot(const ml_image *image);
char *ml_file_architecture_matrix_report(const ml_file *file);
char *ml_image_validation_json(const ml_image *image);

char *ml_image_build_report(const ml_image *image);
char *ml_image_imports_report(const ml_image *image, size_t maximum_imports);
char *ml_image_entry_report(const ml_image *image);
char *ml_image_section_distribution_report(const ml_image *image);
char *ml_image_export_prefix_report(const ml_image *image,
                                    size_t maximum_prefixes);
char *ml_image_symbol_groups_report(const ml_image *image);

char *ml_image_bindings_report(const ml_image *image, size_t maximum_bindings);
char *ml_image_rebases_report(const ml_image *image, size_t maximum_rebases);
char *ml_image_functions_report(const ml_image *image, size_t maximum_functions);
char *ml_image_indirect_symbols_report(const ml_image *image,
                                       size_t maximum_entries);
char *ml_image_runtime_report(const ml_image *image);
char *ml_image_ranges_report(const ml_image *image);
char *ml_image_notes_report(const ml_image *image);

char *ml_image_sections_report(const ml_image *image, size_t maximum_sections);
char *ml_image_symbols_report(const ml_image *image, size_t maximum_symbols);
char *ml_image_exports_report(const ml_image *image, size_t maximum_exports);
char *ml_image_relocations_report(const ml_image *image,
                                  size_t maximum_relocations);
char *ml_image_opcodes_report(const ml_image *image);
char *ml_image_diagnostics_report(const ml_image *image);
char *ml_image_security_report(const ml_image *image);
char *ml_image_hexdump(const ml_image *image, uint64_t offset,
                       size_t size, size_t bytes_per_line);
char *ml_file_summary_report(const ml_file *file);
char *ml_image_manifest_json(const ml_image *image);

char *ml_image_commands_report(const ml_image *image);
char *ml_image_dependencies_report(const ml_image *image);
char *ml_image_signature_report(const ml_image *image);
char *ml_image_unwind_report(const ml_image *image);
char *ml_image_strings_report(const ml_image *image,
                              const char *segment_name,
                              const char *section_name,
                              size_t maximum_strings);
char *ml_image_architecture_report(const ml_image *image);
char *ml_compare_images_report(const ml_image *left, const ml_image *right);

size_t ml_image_load_command_count(const ml_image *image);
int ml_image_load_command_info(const ml_image *image, size_t index,
                               ml_load_command_info *out);
size_t ml_image_rpath_count(const ml_image *image);
const char *ml_image_rpath_at(const ml_image *image, size_t index);
const char *ml_image_dylinker_path(const ml_image *image);
size_t ml_image_linker_option_count(const ml_image *image);
const char *ml_image_linker_option_at(const ml_image *image, size_t index);
size_t ml_image_note_count(const ml_image *image);
int ml_image_note_info(const ml_image *image, size_t index,
                       ml_note_info *out);
size_t ml_image_indirect_symbol_count(const ml_image *image);
int ml_image_indirect_symbol_at(const ml_image *image, size_t index,
                                uint32_t *symbol_index);
size_t ml_image_code_directory_count(const ml_image *image);
int ml_image_code_directory_info(const ml_image *image, size_t index,
                                 ml_code_directory_info *out);
size_t ml_image_entitlement_key_count(const ml_image *image);
const char *ml_image_entitlement_key_at(const ml_image *image, size_t index);
ml_unwind_summary ml_image_unwind_summary(const ml_image *image);
ml_dependency_summary ml_image_dependency_summary(const ml_image *image);

size_t ml_image_relocation_count(const ml_image *image);
int ml_image_relocation_info(const ml_image *image, size_t index,
                             ml_relocation_info *out);
size_t ml_image_dylib_count(const ml_image *image);
int ml_image_dylib_info(const ml_image *image, size_t index,
                        ml_dylib_info *out);
size_t ml_image_function_count(const ml_image *image);
int ml_image_function_info(const ml_image *image, size_t index,
                           ml_function_info *out);
size_t ml_image_code_blob_count(const ml_image *image);
int ml_image_code_blob_info(const ml_image *image, size_t index,
                            ml_code_blob_info *out);
size_t ml_image_diagnostic_count(const ml_image *image);
int ml_image_diagnostic_info(const ml_image *image, size_t index,
                             ml_diagnostic_info *out);
size_t ml_image_range_count(const ml_image *image);
int ml_image_range_info(const ml_image *image, size_t index,
                        ml_range_info *out);

int ml_image_segment_metrics(const ml_image *image, size_t index,
                             ml_segment_metrics *out);
int ml_image_section_metrics(const ml_image *image, size_t index,
                             ml_section_metrics *out);
ml_layout_summary ml_image_layout_summary(const ml_image *image);
ml_symbol_summary ml_image_symbol_summary(const ml_image *image);
ml_linkedit_summary ml_image_linkedit_summary(const ml_image *image);
ml_language_runtime_summary ml_image_language_runtime_summary(const ml_image *image);

size_t ml_image_cstring_count(const ml_image *image,
                              const char *segment_name,
                              const char *section_name);
char *ml_image_cstring_copy(const ml_image *image,
                            const char *segment_name,
                            const char *section_name,
                            size_t index);

char *ml_image_symbols_csv(const ml_image *image);
char *ml_image_sections_csv(const ml_image *image);
char *ml_image_layout_report(const ml_image *image);
char *ml_image_linkedit_report(const ml_image *image);
char *ml_format_packed_version(uint32_t version);
char *ml_format_source_version(const ml_image *image);
char *ml_format_uuid(const ml_image *image);
const char *ml_header_flag_name(unsigned bit_index);
const char *ml_vm_protection_name(uint32_t protection);

ml_validation_summary ml_image_validate(const ml_image *image);
char *ml_image_report_text(const ml_image *image);
char *ml_file_report_json(const ml_file *file);
void ml_string_free(char *text);

#ifdef __cplusplus
}
#endif

#endif

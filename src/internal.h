#ifndef MACHLENS_INTERNAL_H
#define MACHLENS_INTERNAL_H

#include "machlens.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define ML_MAGIC_MH32 0xfeedfaceu
#define ML_MAGIC_MH64 0xfeedfacfu
#define ML_MAGIC_CIGAM32 0xcefaedfeu
#define ML_MAGIC_CIGAM64 0xcffaedfeu
#define ML_MAGIC_FAT32 0xcafebabeu
#define ML_MAGIC_FAT64 0xcafebabfu
#define ML_MAGIC_FAT32_SWAP 0xbebafecau
#define ML_MAGIC_FAT64_SWAP 0xbfbafecau

#define ML_LC_REQ_DYLD 0x80000000u
#define ML_LC_SEGMENT 0x1u
#define ML_LC_SYMTAB 0x2u
#define ML_LC_SYMSEG 0x3u
#define ML_LC_THREAD 0x4u
#define ML_LC_UNIXTHREAD 0x5u
#define ML_LC_LOADFVMLIB 0x6u
#define ML_LC_IDFVMLIB 0x7u
#define ML_LC_IDENT 0x8u
#define ML_LC_FVMFILE 0x9u
#define ML_LC_PREPAGE 0xau
#define ML_LC_DYSYMTAB 0xbu
#define ML_LC_LOAD_DYLIB 0xcu
#define ML_LC_ID_DYLIB 0xdu
#define ML_LC_LOAD_DYLINKER 0xeu
#define ML_LC_ID_DYLINKER 0xfu
#define ML_LC_PREBOUND_DYLIB 0x10u
#define ML_LC_ROUTINES 0x11u
#define ML_LC_SUB_FRAMEWORK 0x12u
#define ML_LC_SUB_UMBRELLA 0x13u
#define ML_LC_SUB_CLIENT 0x14u
#define ML_LC_SUB_LIBRARY 0x15u
#define ML_LC_TWOLEVEL_HINTS 0x16u
#define ML_LC_PREBIND_CKSUM 0x17u
#define ML_LC_LOAD_WEAK_DYLIB (0x18u | ML_LC_REQ_DYLD)
#define ML_LC_SEGMENT_64 0x19u
#define ML_LC_ROUTINES_64 0x1au
#define ML_LC_UUID 0x1bu
#define ML_LC_RPATH (0x1cu | ML_LC_REQ_DYLD)
#define ML_LC_CODE_SIGNATURE 0x1du
#define ML_LC_SEGMENT_SPLIT_INFO 0x1eu
#define ML_LC_REEXPORT_DYLIB (0x1fu | ML_LC_REQ_DYLD)
#define ML_LC_LAZY_LOAD_DYLIB 0x20u
#define ML_LC_ENCRYPTION_INFO 0x21u
#define ML_LC_DYLD_INFO 0x22u
#define ML_LC_DYLD_INFO_ONLY (0x22u | ML_LC_REQ_DYLD)
#define ML_LC_LOAD_UPWARD_DYLIB (0x23u | ML_LC_REQ_DYLD)
#define ML_LC_VERSION_MIN_MACOSX 0x24u
#define ML_LC_VERSION_MIN_IPHONEOS 0x25u
#define ML_LC_FUNCTION_STARTS 0x26u
#define ML_LC_DYLD_ENVIRONMENT 0x27u
#define ML_LC_MAIN (0x28u | ML_LC_REQ_DYLD)
#define ML_LC_DATA_IN_CODE 0x29u
#define ML_LC_SOURCE_VERSION 0x2au
#define ML_LC_DYLIB_CODE_SIGN_DRS 0x2bu
#define ML_LC_ENCRYPTION_INFO_64 0x2cu
#define ML_LC_LINKER_OPTION 0x2du
#define ML_LC_LINKER_OPTIMIZATION_HINT 0x2eu
#define ML_LC_VERSION_MIN_TVOS 0x2fu
#define ML_LC_VERSION_MIN_WATCHOS 0x30u
#define ML_LC_NOTE 0x31u
#define ML_LC_BUILD_VERSION 0x32u
#define ML_LC_DYLD_EXPORTS_TRIE (0x33u | ML_LC_REQ_DYLD)
#define ML_LC_DYLD_CHAINED_FIXUPS (0x34u | ML_LC_REQ_DYLD)
#define ML_LC_FILESET_ENTRY (0x35u | ML_LC_REQ_DYLD)

#define ML_CPU_TYPE_X86 7u
#define ML_CPU_TYPE_ARM 12u
#define ML_CPU_TYPE_POWERPC 18u
#define ML_CPU_ARCH_ABI64 0x01000000u

#define ML_EXPORT_SYMBOL_FLAGS_KIND_MASK 0x03u
#define ML_EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION 0x04u
#define ML_EXPORT_SYMBOL_FLAGS_REEXPORT 0x08u
#define ML_EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER 0x10u

#define ML_BIND_OPCODE_MASK 0xf0u
#define ML_BIND_IMMEDIATE_MASK 0x0fu
#define ML_BIND_OPCODE_DONE 0x00u
#define ML_BIND_OPCODE_SET_DYLIB_ORDINAL_IMM 0x10u
#define ML_BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB 0x20u
#define ML_BIND_OPCODE_SET_DYLIB_SPECIAL_IMM 0x30u
#define ML_BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM 0x40u
#define ML_BIND_OPCODE_SET_TYPE_IMM 0x50u
#define ML_BIND_OPCODE_SET_ADDEND_SLEB 0x60u
#define ML_BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB 0x70u
#define ML_BIND_OPCODE_ADD_ADDR_ULEB 0x80u
#define ML_BIND_OPCODE_DO_BIND 0x90u
#define ML_BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB 0xa0u
#define ML_BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED 0xb0u
#define ML_BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB 0xc0u
#define ML_BIND_SYMBOL_FLAGS_WEAK_IMPORT 0x1u

#define ML_REBASE_OPCODE_MASK 0xf0u
#define ML_REBASE_IMMEDIATE_MASK 0x0fu
#define ML_REBASE_OPCODE_DONE 0x00u
#define ML_REBASE_OPCODE_SET_TYPE_IMM 0x10u
#define ML_REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB 0x20u
#define ML_REBASE_OPCODE_ADD_ADDR_ULEB 0x30u
#define ML_REBASE_OPCODE_ADD_ADDR_IMM_SCALED 0x40u
#define ML_REBASE_OPCODE_DO_REBASE_IMM_TIMES 0x50u
#define ML_REBASE_OPCODE_DO_REBASE_ULEB_TIMES 0x60u
#define ML_REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB 0x70u
#define ML_REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB 0x80u

#define ML_CS_MAGIC_EMBEDDED_SIGNATURE 0xfade0cc0u
#define ML_CS_MAGIC_DETACHED_SIGNATURE 0xfade0cc1u
#define ML_CS_MAGIC_CODEDIRECTORY 0xfade0c02u
#define ML_CS_MAGIC_BLOBWRAPPER 0xfade0b01u
#define ML_CS_MAGIC_REQUIREMENTS 0xfade0c01u
#define ML_CS_MAGIC_ENTITLEMENTS 0xfade7171u
#define ML_CS_MAGIC_DER_ENTITLEMENTS 0xfade7172u

#define ML_MAX_LOAD_COMMANDS 65536u
#define ML_MAX_FAT_IMAGES 4096u
#define ML_MAX_SECTIONS 65536u
#define ML_MAX_SYMBOLS 4000000u
#define ML_MAX_EXPORT_NODES 1000000u
#define ML_MAX_BINDINGS 2000000u
#define ML_MAX_DIAGNOSTICS 4096u

typedef struct ml_reader {
    const uint8_t *data;
    size_t size;
    size_t offset;
    ml_endian endian;
} ml_reader;

typedef struct ml_range {
    uint64_t offset;
    uint64_t size;
} ml_range;

typedef struct ml_diagnostic {
    ml_status status;
    uint64_t offset;
    char *message;
} ml_diagnostic;

typedef struct ml_section_record {
    char segment_name[17];
    char section_name[17];
    uint64_t address;
    uint64_t size;
    uint32_t offset;
    uint32_t alignment;
    uint32_t relocation_offset;
    uint32_t relocation_count;
    uint32_t flags;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    size_t segment_index;
} ml_section_record;

typedef struct ml_segment_record {
    char name[17];
    uint64_t vm_address;
    uint64_t vm_size;
    uint64_t file_offset;
    uint64_t file_size;
    uint32_t max_protection;
    uint32_t initial_protection;
    uint32_t flags;
    size_t first_section;
    size_t section_count;
} ml_segment_record;

typedef struct ml_symbol_record {
    char *name;
    uint64_t value;
    uint8_t type;
    uint8_t section;
    uint16_t description;
    uint32_t string_index;
    int external;
    int undefined_symbol;
} ml_symbol_record;

typedef struct ml_relocation_record {
    uint64_t address;
    uint32_t symbol_index;
    uint8_t length;
    uint8_t type;
    uint8_t pc_relative;
    uint8_t external;
    uint8_t scattered;
    int32_t value;
    size_t section_index;
} ml_relocation_record;

typedef struct ml_export_record {
    char *name;
    uint64_t address;
    uint64_t flags;
    char *reexport_name;
    uint64_t resolver;
} ml_export_record;

typedef struct ml_binding_record {
    char *symbol_name;
    size_t segment_index;
    uint64_t segment_offset;
    uint64_t address;
    int64_t addend;
    int library_ordinal;
    uint8_t type;
    uint8_t flags;
    int weak;
    int lazy;
} ml_binding_record;

typedef struct ml_rebase_record {
    size_t segment_index;
    uint64_t segment_offset;
    uint64_t address;
    uint8_t type;
} ml_rebase_record;

typedef struct ml_data_in_code_record {
    uint32_t offset;
    uint16_t length;
    uint16_t kind;
} ml_data_in_code_record;

typedef struct ml_code_blob_record {
    uint32_t type;
    uint32_t magic;
    uint32_t offset;
    uint32_t length;
} ml_code_blob_record;

typedef struct ml_dylib_record {
    char *name;
    uint32_t timestamp;
    uint32_t current_version;
    uint32_t compatibility_version;
    uint32_t command;
} ml_dylib_record;

typedef struct ml_build_tool_record {
    uint32_t tool;
    uint32_t version;
} ml_build_tool_record;

typedef struct ml_load_command_record {
    uint32_t command;
    uint32_t size;
    uint64_t file_offset;
} ml_load_command_record;


typedef struct ml_note_record {
    char owner[17];
    uint64_t offset;
    uint64_t size;
} ml_note_record;

typedef struct ml_code_directory_record {
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
    char *identifier;
    char *team_identifier;
    uint64_t executable_segment_base;
    uint64_t executable_segment_limit;
    uint64_t executable_segment_flags;
} ml_code_directory_record;

typedef struct ml_named_range {
    char name[32];
    uint64_t offset;
    uint64_t size;
    uint32_t category;
} ml_named_range;

typedef struct ml_string_builder {
    char *data;
    size_t length;
    size_t capacity;
} ml_string_builder;

struct ml_context {
    ml_status status;
    char error[512];
};

struct ml_image {
    struct ml_file *owner;
    uint64_t slice_offset;
    uint64_t slice_size;
    ml_endian endian;
    int is_64;
    uint32_t cpu_type;
    uint32_t cpu_subtype;
    uint32_t file_type;
    uint32_t command_count;
    uint32_t command_bytes;
    uint32_t flags;
    uint32_t reserved;
    uint64_t command_offset;

    ml_segment_record *segments;
    size_t segment_count;
    size_t segment_capacity;
    ml_section_record *sections;
    size_t section_count;
    size_t section_capacity;
    ml_symbol_record *symbols;
    size_t symbol_count;
    size_t symbol_capacity;
    ml_relocation_record *relocations;
    size_t relocation_count;
    size_t relocation_capacity;
    ml_export_record *exports;
    size_t export_count;
    size_t export_capacity;
    ml_binding_record *bindings;
    size_t binding_count;
    size_t binding_capacity;
    ml_rebase_record *rebases;
    size_t rebase_count;
    size_t rebase_capacity;
    uint64_t *function_starts;
    size_t function_start_count;
    size_t function_start_capacity;
    ml_data_in_code_record *data_in_code;
    size_t data_in_code_count;
    size_t data_in_code_capacity;
    ml_code_blob_record *code_blobs;
    size_t code_blob_count;
    size_t code_blob_capacity;
    ml_dylib_record *dylibs;
    size_t dylib_count;
    size_t dylib_capacity;
    ml_build_tool_record *build_tools;
    size_t build_tool_count;
    size_t build_tool_capacity;
    ml_load_command_record *load_commands;
    size_t load_command_count;
    size_t load_command_capacity;

    char **rpaths;
    size_t rpath_count;
    size_t rpath_capacity;
    char *dylinker_path;
    char **linker_options;
    size_t linker_option_count;
    size_t linker_option_capacity;
    ml_note_record *notes;
    size_t note_count;
    size_t note_capacity;
    uint32_t *indirect_symbols;
    size_t indirect_symbol_decoded_count;
    ml_code_directory_record *code_directories;
    size_t code_directory_count;
    size_t code_directory_capacity;
    char **entitlement_keys;
    size_t entitlement_key_count;
    size_t entitlement_key_capacity;
    ml_named_range *ranges;
    size_t range_count;
    size_t range_capacity;
    ml_diagnostic *diagnostics;
    size_t diagnostic_count;
    size_t diagnostic_capacity;

    uint8_t uuid[16];
    int has_uuid;
    uint64_t entry_point;
    uint64_t stack_size;
    int has_entry_point;
    uint32_t platform;
    uint32_t minimum_os;
    uint32_t sdk;
    uint32_t source_version[5];
    uint32_t encryption_offset;
    uint32_t encryption_size;
    uint32_t encryption_id;
    int has_encryption;

    uint32_t symbol_table_offset;
    uint32_t symbol_count_declared;
    uint32_t string_table_offset;
    uint32_t string_table_size;
    uint32_t indirect_symbol_offset;
    uint32_t indirect_symbol_count;

    uint32_t rebase_offset;
    uint32_t rebase_size;
    uint32_t bind_offset;
    uint32_t bind_size;
    uint32_t weak_bind_offset;
    uint32_t weak_bind_size;
    uint32_t lazy_bind_offset;
    uint32_t lazy_bind_size;
    uint32_t export_offset;
    uint32_t export_size;
    uint32_t function_starts_offset;
    uint32_t function_starts_size;
    uint32_t data_in_code_offset;
    uint32_t data_in_code_size;
    uint32_t code_signature_offset;
    uint32_t code_signature_size;
    uint32_t exports_trie_offset;
    uint32_t exports_trie_size;
    uint32_t chained_fixups_offset;
    uint32_t chained_fixups_size;

    size_t *symbol_name_index;
    size_t symbol_name_index_count;
    size_t *export_name_index;
    size_t export_name_index_count;
    size_t *section_name_index;
    size_t section_name_index_count;

    ml_validation_summary validation;
    int validation_ready;
};

struct ml_file {
    ml_context *ctx;
    uint8_t *data;
    size_t size;
    int universal;
    ml_image *images;
    size_t image_count;
    size_t image_capacity;
};

void ml_context_clear(ml_context *ctx);
void ml_context_set(ml_context *ctx, ml_status status, const char *fmt, ...);
void ml_image_diag(ml_image *image, ml_status status, uint64_t offset,
                   const char *fmt, ...);

int ml_size_add(size_t a, size_t b, size_t *out);
int ml_size_mul(size_t a, size_t b, size_t *out);
int ml_u64_add(uint64_t a, uint64_t b, uint64_t *out);
int ml_range_inside(uint64_t offset, uint64_t length, uint64_t total);
int ml_ranges_overlap(ml_range a, ml_range b);

void ml_reader_init(ml_reader *reader, const uint8_t *data, size_t size,
                    ml_endian endian);
int ml_reader_seek(ml_reader *reader, size_t offset);
int ml_reader_skip(ml_reader *reader, size_t amount);
size_t ml_reader_remaining(const ml_reader *reader);
int ml_reader_u8(ml_reader *reader, uint8_t *out);
int ml_reader_u16(ml_reader *reader, uint16_t *out);
int ml_reader_u32(ml_reader *reader, uint32_t *out);
int ml_reader_u64(ml_reader *reader, uint64_t *out);
int ml_reader_i32(ml_reader *reader, int32_t *out);
int ml_reader_bytes(ml_reader *reader, void *out, size_t amount);
int ml_reader_view(ml_reader *reader, const uint8_t **out, size_t amount);
int ml_reader_cstring_at(const uint8_t *data, size_t size, size_t offset,
                         const char **out, size_t *length);
uint16_t ml_load_u16(const uint8_t *data, ml_endian endian);
uint32_t ml_load_u32(const uint8_t *data, ml_endian endian);
uint64_t ml_load_u64(const uint8_t *data, ml_endian endian);

int ml_read_uleb128(const uint8_t *data, size_t size, size_t *cursor,
                    uint64_t *out);
int ml_read_sleb128(const uint8_t *data, size_t size, size_t *cursor,
                    int64_t *out);

int ml_vec_reserve(void **items, size_t *capacity, size_t count,
                   size_t item_size);
int ml_vec_append(void **items, size_t *count, size_t *capacity,
                  const void *item, size_t item_size);
char *ml_strdup_n(const char *text, size_t length);
char *ml_strdup_safe(const char *text);

void ml_sb_init(ml_string_builder *builder);
void ml_sb_dispose(ml_string_builder *builder);
int ml_sb_append(ml_string_builder *builder, const char *text);
int ml_sb_append_n(ml_string_builder *builder, const char *text, size_t length);
int ml_sb_appendf(ml_string_builder *builder, const char *fmt, ...);
char *ml_sb_take(ml_string_builder *builder);
int ml_sb_json_string(ml_string_builder *builder, const char *text);

const char *ml_cpu_name(uint32_t cpu_type, uint32_t cpu_subtype);
ml_cpu_family ml_cpu_family_from_type(uint32_t cpu_type);
const char *ml_file_type_name(uint32_t file_type);
const char *ml_load_command_name(uint32_t command);
const char *ml_platform_name(uint32_t platform);
const char *ml_section_type_name(uint32_t flags);
const char *ml_code_blob_type_name(uint32_t type);
const char *ml_data_in_code_kind_name(uint16_t kind);
const char *ml_relocation_type_name_internal(const ml_image *image, uint8_t type);

int ml_parse_file(ml_file *file);
int ml_parse_fat(ml_file *file, uint32_t magic);
int ml_parse_image(ml_file *file, ml_image *image);
int ml_parse_load_commands(ml_image *image);
int ml_parse_segment_command(ml_image *image, const uint8_t *data, size_t size,
                             int is_64);
int ml_parse_symtab_command(ml_image *image, const uint8_t *data, size_t size);
int ml_parse_dysymtab_command(ml_image *image, const uint8_t *data, size_t size);
int ml_parse_dyld_info_command(ml_image *image, const uint8_t *data, size_t size);
int ml_parse_linkedit_data_command(ml_image *image, uint32_t command,
                                   const uint8_t *data, size_t size);
int ml_parse_dylib_command(ml_image *image, uint32_t command,
                           const uint8_t *data, size_t size);

int ml_parse_string_command(ml_image *image, uint32_t command,
                            const uint8_t *data, size_t size);
int ml_parse_linker_option_command(ml_image *image,
                                   const uint8_t *data, size_t size);
int ml_parse_note_command(ml_image *image, const uint8_t *data, size_t size);
int ml_decode_indirect_symbols(ml_image *image);
int ml_decode_code_directory_details(ml_image *image);
int ml_decode_entitlements(ml_image *image);

int ml_parse_uuid_command(ml_image *image, const uint8_t *data, size_t size);
int ml_parse_main_command(ml_image *image, const uint8_t *data, size_t size);
int ml_parse_build_version_command(ml_image *image, const uint8_t *data,
                                   size_t size);
int ml_parse_source_version_command(ml_image *image, const uint8_t *data,
                                    size_t size);
int ml_parse_encryption_command(ml_image *image, const uint8_t *data,
                                size_t size);
int ml_parse_thread_command(ml_image *image, const uint8_t *data, size_t size);

int ml_decode_symbols(ml_image *image);
int ml_decode_relocations(ml_image *image);
int ml_decode_exports(ml_image *image);
int ml_decode_bindings(ml_image *image);
int ml_decode_rebases(ml_image *image);
int ml_decode_function_starts(ml_image *image);
int ml_decode_data_in_code(ml_image *image);
int ml_decode_code_signature(ml_image *image);
int ml_decode_chained_fixups(ml_image *image);
int ml_build_ranges(ml_image *image);
int ml_build_indexes(ml_image *image);
int ml_resolve_addresses(ml_image *image);
ml_validation_summary ml_validate_image(ml_image *image);

int ml_image_add_range(ml_image *image, const char *name, uint64_t offset,
                       uint64_t size, uint32_t category);
int ml_image_slice(const ml_image *image, uint64_t offset, uint64_t size,
                   const uint8_t **out);
int ml_image_vm_to_file(const ml_image *image, uint64_t address,
                        uint64_t *file_offset);
int ml_image_file_to_vm(const ml_image *image, uint64_t file_offset,
                        uint64_t *address);

int ml_compare_symbol_name_index(const void *left, const void *right,
                                 void *context);
int ml_compare_export_name_index(const void *left, const void *right,
                                 void *context);
int ml_compare_section_name_index(const void *left, const void *right,
                                  void *context);
void ml_sort_indices(size_t *indices, size_t count,
                     int (*compare)(const void *, const void *, void *),
                     void *context);

uint32_t ml_crc32(const uint8_t *data, size_t size);
uint64_t ml_fnv1a64(const void *data, size_t size);

void ml_image_dispose(ml_image *image);

#endif

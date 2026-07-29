#include "machlens.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

static void require_invariant(bool condition) {
    if (!condition) {
        __builtin_trap();
    }
}

static void require_text_equal(char *left, char *right) {
    require_invariant((left == nullptr) == (right == nullptr));
    if (left != nullptr && right != nullptr) {
        require_invariant(std::strcmp(left, right) == 0);
    }
    ml_string_free(left);
    ml_string_free(right);
}

static bool has_prefix(const char *text, const char *prefix) {
    if (text == nullptr || prefix == nullptr) return false;
    while (*prefix != '\0') {
        if (*text++ != *prefix++) return false;
    }
    return true;
}

static bool contains_text(const char *text, const char *needle) {
    return text != nullptr && needle != nullptr &&
           std::strstr(text, needle) != nullptr;
}

static bool is_ascii(const unsigned char *text, size_t length) {
    for (size_t index = 0; index < length; ++index) {
        if (text[index] >= 0x80u) return false;
    }
    return true;
}

static bool is_utf8_like(const unsigned char *text, size_t length) {
    size_t cursor = 0;
    while (cursor < length) {
        unsigned char first = text[cursor++];
        size_t continuation;
        if (first < 0x80u) continue;
        if ((first & 0xe0u) == 0xc0u) {
            continuation = 1;
            if (first < 0xc2u) return false;
        } else if ((first & 0xf0u) == 0xe0u) {
            continuation = 2;
        } else if ((first & 0xf8u) == 0xf0u) {
            continuation = 3;
            if (first > 0xf4u) return false;
        } else {
            return false;
        }
        if (continuation > length - cursor) return false;
        while (continuation-- != 0) {
            if ((text[cursor++] & 0xc0u) != 0x80u) return false;
        }
    }
    return true;
}

static bool is_path_like(const unsigned char *text, size_t length) {
    size_t separators = 0;
    for (size_t index = 0; index < length; ++index) {
        if (text[index] == '/' || text[index] == '\\') ++separators;
    }
    return separators != 0 ||
           (length >= 2 && text[0] == '.' && text[1] == '.') ||
           (length >= 2 && text[0] == '@' && text[1] == 'r');
}

static bool is_selector_like(const unsigned char *text, size_t length) {
    size_t colons = 0;
    if (length == 0) return false;
    for (size_t index = 0; index < length; ++index) {
        unsigned char ch = text[index];
        if (ch == ':') {
            ++colons;
        } else if (!(ch == '_' || ch == '$' || ch == '.' ||
                     (ch >= 'a' && ch <= 'z') ||
                     (ch >= 'A' && ch <= 'Z') ||
                     (ch >= '0' && ch <= '9'))) {
            return false;
        }
    }
    return colons != 0;
}

static void check_index_contracts(const ml_image *image) {
    size_t index;
    uint64_t prior_command_end = 0;
    for (index = 0; index < ml_image_segment_count(image); ++index) {
        ml_segment_info info;
        ml_segment_metrics metrics;
        require_invariant(ml_image_segment_info(image, index, &info) != 0);
        require_invariant(ml_image_segment_metrics(image, index, &metrics) != 0);
        if (info.file_size != 0) {
            uint64_t address;
            uint64_t offset;
            require_invariant(ml_image_file_offset_to_address(
                                  image, info.file_offset, &address) != 0);
            require_invariant(ml_image_address_to_file_offset(
                                  image, address, &offset) != 0);
            require_invariant(offset == info.file_offset);
        }
    }
    {
        ml_segment_info info;
        require_invariant(ml_image_segment_info(
                              image, ml_image_segment_count(image), &info) == 0);
    }

    for (index = 0; index < ml_image_section_count(image); ++index) {
        ml_section_info info;
        ml_section_metrics metrics;
        ml_section_info found;
        require_invariant(ml_image_section_info(image, index, &info) != 0);
        require_invariant(ml_image_section_metrics(image, index, &metrics) != 0);
        require_invariant(ml_image_find_section(image, info.segment_name,
                                                info.section_name, &found) != 0);
        require_invariant(std::strcmp(found.segment_name,
                                      info.segment_name) == 0);
        require_invariant(std::strcmp(found.section_name,
                                      info.section_name) == 0);
        if (metrics.file_backed && info.size != 0) {
            uint64_t offset;
            uint64_t address;
            require_invariant(ml_image_address_to_file_offset(
                                  image, info.address, &offset) != 0);
            require_invariant(ml_image_file_offset_to_address(
                                  image, offset, &address) != 0);
            require_invariant(address == info.address);
        }
    }
    {
        ml_section_info info;
        require_invariant(ml_image_section_info(
                              image, ml_image_section_count(image), &info) == 0);
    }

    for (index = 0; index < ml_image_symbol_count(image); ++index) {
        ml_symbol_info info;
        require_invariant(ml_image_symbol_info(image, index, &info) != 0);
        if (info.name != nullptr && info.name[0] != '\0') {
            ml_symbol_info found;
            require_invariant(ml_image_find_symbol(image, info.name, &found) != 0);
            require_invariant(found.name != nullptr);
            require_invariant(std::strcmp(found.name, info.name) == 0);
        }
    }
    {
        ml_symbol_info info;
        require_invariant(ml_image_symbol_info(
                              image, ml_image_symbol_count(image), &info) == 0);
    }

    for (index = 0; index < ml_image_export_count(image); ++index) {
        ml_export_info info;
        require_invariant(ml_image_export_info(image, index, &info) != 0);
        if (info.name != nullptr && info.name[0] != '\0') {
            ml_export_info found;
            require_invariant(ml_image_find_export(image, info.name, &found) != 0);
            require_invariant(found.name != nullptr);
            require_invariant(std::strcmp(found.name, info.name) == 0);
        }
    }
    {
        ml_export_info info;
        require_invariant(ml_image_export_info(
                              image, ml_image_export_count(image), &info) == 0);
    }

    for (index = 0; index < ml_image_binding_count(image); ++index) {
        ml_binding_info info;
        require_invariant(ml_image_binding_info(image, index, &info) != 0);
    }
    {
        ml_binding_info info;
        require_invariant(ml_image_binding_info(
                              image, ml_image_binding_count(image), &info) == 0);
    }

    for (index = 0; index < ml_image_load_command_count(image); ++index) {
        ml_load_command_info info;
        require_invariant(ml_image_load_command_info(image, index, &info) != 0);
        require_invariant(info.size >= 8u);
        if (index != 0) require_invariant(info.file_offset == prior_command_end);
        require_invariant(UINT64_MAX - info.file_offset >= info.size);
        prior_command_end = info.file_offset + info.size;
    }
    {
        ml_load_command_info info;
        require_invariant(ml_image_load_command_info(
                              image, ml_image_load_command_count(image), &info) == 0);
    }

    for (index = 0; index < ml_image_dylib_count(image); ++index) {
        ml_dylib_info info;
        require_invariant(ml_image_dylib_info(image, index, &info) != 0);
    }
    for (index = 0; index < ml_image_relocation_count(image); ++index) {
        ml_relocation_info info;
        require_invariant(ml_image_relocation_info(image, index, &info) != 0);
    }
    for (index = 0; index < ml_image_function_count(image); ++index) {
        ml_function_info info;
        require_invariant(ml_image_function_info(image, index, &info) != 0);
    }
    for (index = 0; index < ml_image_diagnostic_count(image); ++index) {
        ml_diagnostic_info info;
        require_invariant(ml_image_diagnostic_info(image, index, &info) != 0);
    }
    for (index = 0; index < ml_image_range_count(image); ++index) {
        ml_range_info info;
        require_invariant(ml_image_range_info(image, index, &info) != 0);
    }
    for (index = 0; index < ml_image_note_count(image); ++index) {
        ml_note_info info;
        require_invariant(ml_image_note_info(image, index, &info) != 0);
    }
    for (index = 0; index < ml_image_code_blob_count(image); ++index) {
        ml_code_blob_info info;
        require_invariant(ml_image_code_blob_info(image, index, &info) != 0);
    }
    for (index = 0; index < ml_image_code_directory_count(image); ++index) {
        ml_code_directory_info info;
        require_invariant(ml_image_code_directory_info(image, index, &info) != 0);
    }
}

static void check_symbol_summary(const ml_image *image) {
    ml_symbol_summary summary = ml_image_symbol_summary(image);
    ml_symbol_summary expected{};
    expected.lowest_value = UINT64_MAX;
    for (size_t index = 0; index < ml_image_symbol_count(image); ++index) {
        ml_symbol_info info;
        require_invariant(ml_image_symbol_info(image, index, &info) != 0);
        uint8_t basic_type = info.type & 0x0eu;
        if ((info.type & 0xe0u) != 0) {
            ++expected.stab_symbols;
            ++expected.debugging_symbols;
        }
        if (info.external) ++expected.external_symbols;
        else ++expected.local_symbols;
        if (info.undefined_symbol) ++expected.undefined_symbols;
        else if (basic_type == 2u) ++expected.absolute_symbols;
        else if (basic_type == 0x0eu) ++expected.section_symbols;
        if ((info.description & 0x0080u) != 0) ++expected.weak_references;
        if ((info.description & 0x0040u) != 0) ++expected.weak_definitions;
        if (info.name == nullptr || info.name[0] == '\0') ++expected.empty_names;
        if (!info.undefined_symbol) {
            if (info.value < expected.lowest_value) expected.lowest_value = info.value;
            if (info.value > expected.highest_value) expected.highest_value = info.value;
        }
    }
    if (expected.lowest_value == UINT64_MAX) expected.lowest_value = 0;
    require_invariant(std::memcmp(&summary, &expected, sizeof(summary)) == 0);
}

static void check_dependency_summary(const ml_image *image) {
    ml_dependency_summary summary = ml_image_dependency_summary(image);
    ml_dependency_summary expected{};
    for (size_t index = 0; index < ml_image_dylib_count(image); ++index) {
        ml_dylib_info info;
        require_invariant(ml_image_dylib_info(image, index, &info) != 0);
        switch (info.command) {
            case 0x80000018u: ++expected.weak_dependencies; break;
            case 0x8000001fu: ++expected.reexported_dependencies; break;
            case 0x80000023u: ++expected.upward_dependencies; break;
            case 0x20u: ++expected.lazy_dependencies; break;
            case 0x0du: break;
            default: ++expected.regular_dependencies; break;
        }
        if (has_prefix(info.name, "/System/Library/") ||
            has_prefix(info.name, "/usr/lib/")) ++expected.system_dependencies;
        if (contains_text(info.name, ".framework/")) ++expected.framework_dependencies;
        if (has_prefix(info.name, "@rpath/") ||
            has_prefix(info.name, "@loader_path/") ||
            has_prefix(info.name, "@executable_path/")) {
            ++expected.relative_dependencies;
        } else if (info.name != nullptr && info.name[0] == '/') {
            ++expected.absolute_dependencies;
        }
    }
    require_invariant(std::memcmp(&summary, &expected, sizeof(summary)) == 0);
}

static void check_relocation_summary(const ml_image *image) {
    ml_relocation_summary summary = ml_image_relocation_summary(image);
    ml_relocation_summary expected{};
    for (size_t index = 0; index < ml_image_relocation_count(image); ++index) {
        ml_relocation_info info;
        require_invariant(ml_image_relocation_info(image, index, &info) != 0);
        if (info.external) {
            ++expected.external_relocations;
            if (info.symbol_index >= ml_image_symbol_count(image))
                ++expected.unresolved_external_relocations;
        } else {
            ++expected.local_relocations;
        }
        if (info.scattered) ++expected.scattered_relocations;
        if (info.pc_relative) ++expected.pc_relative_relocations;
        if (info.length < 4) ++expected.by_width[info.length];
        if (info.type < 16) ++expected.by_type[info.type];
    }
    require_invariant(std::memcmp(&summary, &expected, sizeof(summary)) == 0);
}

static void check_string_summary(const ml_image *image,
                                 const char *segment_name,
                                 const char *section_name) {
    ml_string_summary summary = ml_image_string_summary(
        image, segment_name, section_name);
    ml_string_summary expected{};
    size_t count = ml_image_cstring_count(image, segment_name, section_name);
    expected.string_count = count;
    for (size_t index = 0; index < count; ++index) {
        char *text = ml_image_cstring_copy(image, segment_name,
                                           section_name, index);
        if (text == nullptr) {
            ++expected.empty_slots;
            continue;
        }
        size_t length = std::strlen(text);
        expected.total_characters += length;
        if (length > expected.longest_string) expected.longest_string = length;
        if (is_ascii(reinterpret_cast<const unsigned char *>(text), length)) {
            ++expected.ascii_strings;
        } else if (is_utf8_like(reinterpret_cast<const unsigned char *>(text),
                                length)) {
            ++expected.utf8_like_strings;
        }
        if (is_path_like(reinterpret_cast<const unsigned char *>(text), length))
            ++expected.path_like_strings;
        if (is_selector_like(reinterpret_cast<const unsigned char *>(text), length))
            ++expected.selector_like_strings;
        ml_string_free(text);
    }
    require_invariant(std::memcmp(&summary, &expected, sizeof(summary)) == 0);
}

static void check_cross_surface_summaries(const ml_image *image) {
    ml_comparison_summary comparison = ml_compare_images(image, image);
    ml_security_summary security = ml_image_security_summary(image);
    ml_layout_summary layout = ml_image_layout_summary(image);
    ml_validation_summary first_validation = ml_image_validate(image);
    ml_validation_summary second_validation = ml_image_validate(image);
    size_t executable = 0;
    size_t writable = 0;
    size_t read_only = 0;
    size_t writable_executable = 0;

    require_invariant(comparison.same_architecture != 0);
    require_invariant(comparison.same_file_type != 0);
    require_invariant(comparison.same_platform != 0);
    require_invariant(comparison.segment_delta == 0);
    require_invariant(comparison.section_delta == 0);
    require_invariant(comparison.symbol_delta == 0);
    require_invariant(comparison.export_delta == 0);
    require_invariant(comparison.dependency_delta == 0);

    for (size_t index = 0; index < ml_image_segment_count(image); ++index) {
        ml_segment_info info;
        require_invariant(ml_image_segment_info(image, index, &info) != 0);
        bool is_executable = (info.initial_protection & 4u) != 0;
        bool is_writable = (info.initial_protection & 2u) != 0;
        bool is_readable = (info.initial_protection & 1u) != 0;
        if (is_executable) ++executable;
        if (is_writable) ++writable;
        else if (is_readable) ++read_only;
        if (is_executable && is_writable) ++writable_executable;
    }
    require_invariant(security.executable_segments == executable);
    require_invariant(security.writable_executable_segments == writable_executable);
    require_invariant(layout.executable_segments == executable);
    require_invariant(layout.writable_segments == writable);
    require_invariant(layout.read_only_segments == read_only);

    check_symbol_summary(image);
    check_dependency_summary(image);
    check_relocation_summary(image);

    require_invariant(first_validation.errors == second_validation.errors);
    require_invariant(first_validation.warnings == second_validation.warnings);
    require_invariant(first_validation.overlapping_ranges ==
                      second_validation.overlapping_ranges);
    require_invariant(first_validation.unresolved_symbols ==
                      second_validation.unresolved_symbols);
    require_invariant(first_validation.malformed_linkedit_records ==
                      second_validation.malformed_linkedit_records);

    for (size_t index = 0; index < ml_image_section_count(image); ++index) {
        ml_section_info info;
        require_invariant(ml_image_section_info(image, index, &info) != 0);
        if (std::strcmp(info.section_name, "__cstring") == 0) {
            check_string_summary(image, info.segment_name, info.section_name);
        }
    }
}

static void check_determinism(const ml_file *file, const ml_image *image) {
    uint64_t image_fingerprint = ml_image_structural_fingerprint(image);
    uint64_t file_fingerprint = ml_file_structural_fingerprint(file);
    require_invariant(image_fingerprint ==
                      ml_image_structural_fingerprint(image));
    require_invariant(file_fingerprint ==
                      ml_file_structural_fingerprint(file));

    require_text_equal(ml_image_manifest_json(image),
                       ml_image_manifest_json(image));
    require_text_equal(ml_image_command_inventory_json(image),
                       ml_image_command_inventory_json(image));
    require_text_equal(ml_image_validation_json(image),
                       ml_image_validation_json(image));
    require_text_equal(ml_image_report_text(image),
                       ml_image_report_text(image));
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    ml_context *context;
    ml_file *file;
    size_t image_index;
    if (size < 64 || size > 16 * 1024 * 1024) return 0;
    context = ml_context_create();
    if (context == nullptr) return 0;
    file = ml_file_parse_memory(context, data, size);
    if (file != nullptr) {
        require_invariant(ml_file_image_count(file) != 0);
        require_invariant(ml_file_structural_fingerprint(file) ==
                          ml_file_structural_fingerprint(file));
        for (image_index = 0; image_index < ml_file_image_count(file);
             ++image_index) {
            const ml_image *image = ml_file_image_at(file, image_index);
            require_invariant(image != nullptr);
            check_index_contracts(image);
            check_cross_surface_summaries(image);
            check_determinism(file, image);
        }
        require_invariant(ml_file_image_at(file, ml_file_image_count(file)) ==
                          nullptr);
        ml_file_destroy(file);
    }
    ml_context_destroy(context);
    return 0;
}

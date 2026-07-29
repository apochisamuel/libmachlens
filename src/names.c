#include "internal.h"

#include <stdio.h>

ml_cpu_family ml_cpu_family_from_type(uint32_t cpu_type) {
    uint32_t base = cpu_type & ~ML_CPU_ARCH_ABI64;
    switch (base) {
        case ML_CPU_TYPE_X86:
            return (cpu_type & ML_CPU_ARCH_ABI64) != 0 ? ML_CPU_X86_64 : ML_CPU_X86;
        case ML_CPU_TYPE_ARM:
            return (cpu_type & ML_CPU_ARCH_ABI64) != 0 ? ML_CPU_ARM64 : ML_CPU_ARM;
        case ML_CPU_TYPE_POWERPC:
            return (cpu_type & ML_CPU_ARCH_ABI64) != 0 ? ML_CPU_POWERPC64 : ML_CPU_POWERPC;
        default:
            return ML_CPU_UNKNOWN;
    }
}

const char *ml_cpu_name(uint32_t cpu_type, uint32_t cpu_subtype) {
    uint32_t base = cpu_type & ~ML_CPU_ARCH_ABI64;
    uint32_t subtype = cpu_subtype & 0x00ffffffu;
    if (base == ML_CPU_TYPE_X86 && (cpu_type & ML_CPU_ARCH_ABI64) != 0) {
        switch (subtype) {
            case 3: return "x86_64";
            case 8: return "x86_64h";
            default: return "x86_64-unknown";
        }
    }
    if (base == ML_CPU_TYPE_X86) {
        switch (subtype) {
            case 3: return "i386";
            case 4: return "i486";
            case 5: return "i586";
            case 6: return "pentium-pro";
            default: return "x86-unknown";
        }
    }
    if (base == ML_CPU_TYPE_ARM && (cpu_type & ML_CPU_ARCH_ABI64) != 0) {
        switch (subtype) {
            case 0: return "arm64";
            case 1: return "arm64v8";
            case 2: return "arm64e";
            default: return "arm64-unknown";
        }
    }
    if (base == ML_CPU_TYPE_ARM) {
        switch (subtype) {
            case 0: return "arm";
            case 6: return "armv6";
            case 9: return "armv7";
            case 11: return "armv7s";
            case 12: return "armv7k";
            default: return "arm-unknown";
        }
    }
    if (base == ML_CPU_TYPE_POWERPC) {
        return (cpu_type & ML_CPU_ARCH_ABI64) != 0 ? "ppc64" : "ppc";
    }
    return "unknown";
}

const char *ml_file_type_name(uint32_t file_type) {
    switch (file_type) {
        case 1: return "object";
        case 2: return "executable";
        case 3: return "fixed-vm-library";
        case 4: return "core";
        case 5: return "preload";
        case 6: return "dynamic-library";
        case 7: return "dynamic-linker";
        case 8: return "bundle";
        case 9: return "stub";
        case 10: return "debug-symbols";
        case 11: return "kernel-extension";
        case 12: return "fileset";
        default: return "unknown";
    }
}

const char *ml_load_command_name(uint32_t command) {
    switch (command) {
        case ML_LC_SEGMENT: return "LC_SEGMENT";
        case ML_LC_SYMTAB: return "LC_SYMTAB";
        case ML_LC_THREAD: return "LC_THREAD";
        case ML_LC_UNIXTHREAD: return "LC_UNIXTHREAD";
        case ML_LC_DYSYMTAB: return "LC_DYSYMTAB";
        case ML_LC_LOAD_DYLIB: return "LC_LOAD_DYLIB";
        case ML_LC_ID_DYLIB: return "LC_ID_DYLIB";
        case ML_LC_LOAD_DYLINKER: return "LC_LOAD_DYLINKER";
        case ML_LC_ID_DYLINKER: return "LC_ID_DYLINKER";
        case ML_LC_LOAD_WEAK_DYLIB: return "LC_LOAD_WEAK_DYLIB";
        case ML_LC_SEGMENT_64: return "LC_SEGMENT_64";
        case ML_LC_UUID: return "LC_UUID";
        case ML_LC_RPATH: return "LC_RPATH";
        case ML_LC_CODE_SIGNATURE: return "LC_CODE_SIGNATURE";
        case ML_LC_REEXPORT_DYLIB: return "LC_REEXPORT_DYLIB";
        case ML_LC_ENCRYPTION_INFO: return "LC_ENCRYPTION_INFO";
        case ML_LC_DYLD_INFO: return "LC_DYLD_INFO";
        case ML_LC_DYLD_INFO_ONLY: return "LC_DYLD_INFO_ONLY";
        case ML_LC_LOAD_UPWARD_DYLIB: return "LC_LOAD_UPWARD_DYLIB";
        case ML_LC_VERSION_MIN_MACOSX: return "LC_VERSION_MIN_MACOSX";
        case ML_LC_VERSION_MIN_IPHONEOS: return "LC_VERSION_MIN_IPHONEOS";
        case ML_LC_FUNCTION_STARTS: return "LC_FUNCTION_STARTS";
        case ML_LC_MAIN: return "LC_MAIN";
        case ML_LC_DATA_IN_CODE: return "LC_DATA_IN_CODE";
        case ML_LC_SOURCE_VERSION: return "LC_SOURCE_VERSION";
        case ML_LC_ENCRYPTION_INFO_64: return "LC_ENCRYPTION_INFO_64";
        case ML_LC_VERSION_MIN_TVOS: return "LC_VERSION_MIN_TVOS";
        case ML_LC_VERSION_MIN_WATCHOS: return "LC_VERSION_MIN_WATCHOS";
        case ML_LC_NOTE: return "LC_NOTE";
        case ML_LC_BUILD_VERSION: return "LC_BUILD_VERSION";
        case ML_LC_DYLD_EXPORTS_TRIE: return "LC_DYLD_EXPORTS_TRIE";
        case ML_LC_DYLD_CHAINED_FIXUPS: return "LC_DYLD_CHAINED_FIXUPS";
        case ML_LC_FILESET_ENTRY: return "LC_FILESET_ENTRY";
        default: return "LC_UNKNOWN";
    }
}

const char *ml_platform_name(uint32_t platform) {
    switch (platform) {
        case 1: return "macOS";
        case 2: return "iOS";
        case 3: return "tvOS";
        case 4: return "watchOS";
        case 5: return "bridgeOS";
        case 6: return "Mac Catalyst";
        case 7: return "iOS Simulator";
        case 8: return "tvOS Simulator";
        case 9: return "watchOS Simulator";
        case 10: return "DriverKit";
        case 11: return "visionOS";
        case 12: return "visionOS Simulator";
        default: return "unknown";
    }
}

const char *ml_section_type_name(uint32_t flags) {
    switch (flags & 0xffu) {
        case 0x0: return "regular";
        case 0x1: return "zero-fill";
        case 0x2: return "cstring-literals";
        case 0x3: return "four-byte-literals";
        case 0x4: return "eight-byte-literals";
        case 0x5: return "literal-pointers";
        case 0x6: return "non-lazy-symbol-pointers";
        case 0x7: return "lazy-symbol-pointers";
        case 0x8: return "symbol-stubs";
        case 0x9: return "module-init-pointers";
        case 0xa: return "module-term-pointers";
        case 0xb: return "coalesced";
        case 0xc: return "gb-zero-fill";
        case 0xd: return "interposing";
        case 0xe: return "sixteen-byte-literals";
        case 0xf: return "dtrace-dof";
        case 0x10: return "lazy-dylib-symbol-pointers";
        case 0x11: return "thread-local-regular";
        case 0x12: return "thread-local-zero-fill";
        case 0x13: return "thread-local-variables";
        case 0x14: return "thread-local-variable-pointers";
        case 0x15: return "thread-local-init-pointers";
        case 0x16: return "init-function-offsets";
        default: return "unknown";
    }
}

const char *ml_code_blob_type_name(uint32_t type) {
    switch (type) {
        case 0: return "code-directory";
        case 1: return "requirements";
        case 2: return "resource-directory";
        case 5: return "entitlements";
        case 7: return "der-entitlements";
        case 0x1000: return "alternate-code-directory";
        case 0x10000: return "cms-signature";
        case 0x10001: return "identification";
        case 0x10002: return "ticket";
        default: return "generic";
    }
}

const char *ml_data_in_code_kind_name(uint16_t kind) {
    switch (kind) {
        case 1: return "data";
        case 2: return "jump-table-8";
        case 3: return "jump-table-16";
        case 4: return "jump-table-32";
        case 5: return "absolute-jump-table-32";
        default: return "unknown";
    }
}

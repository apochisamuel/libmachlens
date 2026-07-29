#include "internal.h"

const char *ml_header_flag_name(unsigned bit_index) {
    switch (bit_index) {
        case 0: return "no-undefined-symbols";
        case 1: return "incremental-link";
        case 2: return "dynamic-link";
        case 3: return "bind-at-load";
        case 4: return "prebound";
        case 5: return "split-segments";
        case 6: return "lazy-init";
        case 7: return "two-level-namespace";
        case 8: return "force-flat-namespace";
        case 9: return "no-multi-definitions";
        case 10: return "no-fix-prebinding";
        case 11: return "prebindable";
        case 12: return "all-modules-bound";
        case 13: return "subsections-via-symbols";
        case 14: return "canonical";
        case 15: return "weak-defines";
        case 16: return "binds-to-weak";
        case 17: return "allow-stack-execution";
        case 18: return "root-safe";
        case 19: return "setuid-safe";
        case 20: return "no-reexported-dylibs";
        case 21: return "position-independent-executable";
        case 22: return "dead-strippable-dylib";
        case 23: return "has-tlv-descriptors";
        case 24: return "no-heap-execution";
        case 25: return "app-extension-safe";
        case 26: return "nlist-out-of-sync-with-dyldinfo";
        case 27: return "sim-support";
        case 28: return "implicit-pagezero";
        default: return "unknown";
    }
}

const char *ml_vm_protection_name(uint32_t protection) {
    switch (protection & 7u) {
        case 0: return "---";
        case 1: return "r--";
        case 2: return "-w-";
        case 3: return "rw-";
        case 4: return "--x";
        case 5: return "r-x";
        case 6: return "-wx";
        case 7: return "rwx";
        default: return "---";
    }
}

static const char *ml_x86_relocation_name(uint8_t type, int is_64) {
    if (is_64) {
        switch (type) {
            case 0: return "unsigned";
            case 1: return "signed";
            case 2: return "branch";
            case 3: return "got-load";
            case 4: return "got";
            case 5: return "subtractor";
            case 6: return "signed-1";
            case 7: return "signed-2";
            case 8: return "signed-4";
            case 9: return "tlv";
            default: return "unknown-x86-64";
        }
    }
    switch (type) {
        case 0: return "vanilla";
        case 1: return "pair";
        case 2: return "sectdiff";
        case 3: return "pb-la-ptr";
        case 4: return "local-sectdiff";
        case 5: return "tlv";
        default: return "unknown-x86";
    }
}

static const char *ml_arm_relocation_name(uint8_t type, int is_64) {
    if (is_64) {
        switch (type) {
            case 0: return "unsigned";
            case 1: return "subtractor";
            case 2: return "branch26";
            case 3: return "page21";
            case 4: return "pageoff12";
            case 5: return "got-load-page21";
            case 6: return "got-load-pageoff12";
            case 7: return "pointer-to-got";
            case 8: return "tlvp-load-page21";
            case 9: return "tlvp-load-pageoff12";
            case 10: return "addend";
            case 11: return "authenticated-pointer";
            default: return "unknown-arm64";
        }
    }
    switch (type) {
        case 0: return "vanilla";
        case 1: return "pair";
        case 2: return "sectdiff";
        case 3: return "local-sectdiff";
        case 4: return "pb-la-ptr";
        case 5: return "branch24";
        case 6: return "thumb-branch22";
        case 7: return "thumb-32bit-branch";
        case 8: return "half";
        case 9: return "half-sectdiff";
        default: return "unknown-arm";
    }
}

const char *ml_relocation_type_name_internal(const ml_image *image,
                                             uint8_t type) {
    ml_cpu_family family;
    if (image == NULL) {
        return "unknown";
    }
    family = ml_cpu_family_from_type(image->cpu_type);
    switch (family) {
        case ML_CPU_X86:
            return ml_x86_relocation_name(type, 0);
        case ML_CPU_X86_64:
            return ml_x86_relocation_name(type, 1);
        case ML_CPU_ARM:
            return ml_arm_relocation_name(type, 0);
        case ML_CPU_ARM64:
            return ml_arm_relocation_name(type, 1);
        case ML_CPU_POWERPC:
        case ML_CPU_POWERPC64:
            switch (type) {
                case 0: return "vanilla";
                case 1: return "pair";
                case 2: return "branch14";
                case 3: return "branch24";
                case 4: return "hi16";
                case 5: return "lo16";
                case 6: return "ha16";
                case 7: return "lo14";
                case 8: return "sectdiff";
                case 9: return "pb-la-ptr";
                case 10: return "hi16-sectdiff";
                case 11: return "lo16-sectdiff";
                case 12: return "ha16-sectdiff";
                case 13: return "jmp-table";
                case 14: return "local-sectdiff";
                default: return "unknown-ppc";
            }
        default:
            return "unknown";
    }
}

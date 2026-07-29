#include "internal.h"

static const char *ml_pass_fail(int safe) {
    return safe ? "pass" : "review";
}

char *ml_image_security_report(const ml_image *image) {
    ml_string_builder builder;
    ml_security_summary summary;
    if (image == NULL) {
        return NULL;
    }
    summary = ml_image_security_summary(image);
    ml_sb_init(&builder);
    if (!ml_sb_appendf(&builder,
                       "Security posture\n"
                       "================\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n"
                       "%-32s %-8s %s\n\n"
                       "Executable segments:          %zu\n"
                       "Writable+executable segments: %zu\n",
                       "Position-independent executable",
                       ml_pass_fail(summary.position_independent),
                       summary.position_independent ? "MH_PIE is set" : "MH_PIE is not set",
                       "Heap execution disabled",
                       ml_pass_fail(summary.no_heap_execution),
                       summary.no_heap_execution ? "MH_NO_HEAP_EXECUTION is set" : "heap policy flag is absent",
                       "Stack execution disabled",
                       ml_pass_fail(!summary.allows_stack_execution),
                       summary.allows_stack_execution ? "stack execution is permitted" : "stack execution flag is absent",
                       "Code signature",
                       ml_pass_fail(summary.has_code_signature),
                       summary.has_code_signature ? "signature metadata is present" : "no signature command was decoded",
                       "Writable executable mappings",
                       ml_pass_fail(!summary.has_writable_executable_segment),
                       summary.has_writable_executable_segment ? "at least one W+X segment exists" : "no W+X segment found",
                       "Executable data-like sections",
                       ml_pass_fail(!summary.has_executable_data_section),
                       summary.has_executable_data_section ? "data section resides in executable mapping" : "none detected",
                       "Page-zero mapping",
                       ml_pass_fail(summary.has_pagezero_segment),
                       summary.has_pagezero_segment ? "guard mapping is present" : "no __PAGEZERO segment found",
                       "Restricted section",
                       ml_pass_fail(summary.has_restricted_section),
                       summary.has_restricted_section ? "restriction marker is present" : "restriction marker is absent",
                       "Encryption declaration",
                       summary.encrypted ? "info" : "info",
                       summary.encrypted ? "encrypted range is declared" : "image is not declared encrypted",
                       "Entitlements",
                       summary.has_entitlements ? "info" : "info",
                       summary.has_entitlements ? "entitlement keys were decoded" : "no XML entitlement keys were decoded",
                       summary.executable_segments,
                       summary.writable_executable_segments)) {
        ml_sb_dispose(&builder);
        return NULL;
    }
    return ml_sb_take(&builder);
}

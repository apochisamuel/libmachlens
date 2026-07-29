#include "internal.h"

#include <string.h>

static void ml_linkedit_add_bytes(uint64_t *total, uint64_t amount) {
    if (!ml_u64_add(*total, amount, total)) {
        *total = UINT64_MAX;
    }
}

ml_linkedit_summary ml_image_linkedit_summary(const ml_image *image) {
    ml_linkedit_summary summary;
    size_t index;
    memset(&summary, 0, sizeof(summary));
    if (image == NULL) {
        return summary;
    }
    ml_linkedit_add_bytes(&summary.declared_bytes,
                          (uint64_t)image->symbol_count_declared *
                          (image->is_64 ? 16u : 12u));
    ml_linkedit_add_bytes(&summary.declared_bytes, image->string_table_size);
    ml_linkedit_add_bytes(&summary.declared_bytes, image->rebase_size);
    ml_linkedit_add_bytes(&summary.declared_bytes, image->bind_size);
    ml_linkedit_add_bytes(&summary.declared_bytes, image->weak_bind_size);
    ml_linkedit_add_bytes(&summary.declared_bytes, image->lazy_bind_size);
    ml_linkedit_add_bytes(&summary.declared_bytes,
                          image->exports_trie_size != 0
                              ? image->exports_trie_size : image->export_size);
    ml_linkedit_add_bytes(&summary.declared_bytes, image->function_starts_size);
    ml_linkedit_add_bytes(&summary.declared_bytes, image->data_in_code_size);
    ml_linkedit_add_bytes(&summary.declared_bytes, image->code_signature_size);
    ml_linkedit_add_bytes(&summary.declared_bytes, image->chained_fixups_size);
    summary.export_nodes = image->export_count;
    summary.bindings = image->binding_count;
    summary.rebases = image->rebase_count;
    summary.function_starts = image->function_start_count;
    summary.data_in_code_entries = image->data_in_code_count;
    summary.signature_blobs = image->code_blob_count;
    summary.decoded_records = image->symbol_count + image->export_count +
        image->binding_count + image->rebase_count +
        image->function_start_count + image->data_in_code_count +
        image->code_blob_count + image->relocation_count;
    for (index = 0; index < image->diagnostic_count; ++index) {
        if (image->diagnostics[index].status == ML_ERR_FORMAT ||
            image->diagnostics[index].status == ML_ERR_TRUNCATED ||
            image->diagnostics[index].status == ML_ERR_RANGE) {
            ++summary.malformed_records;
        }
    }
    return summary;
}

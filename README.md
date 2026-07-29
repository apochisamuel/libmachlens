# libmachlens

`libmachlens` is a portable C library for inspecting Mach-O object files,
executables, dynamic libraries, bundles, and universal binaries. It accepts an
in-memory byte sequence, preserves the input in an owned file object, parses
all contained architecture images, and exposes structured query and reporting
interfaces.

The library focuses on analysis workflows that need more than header
identification. It decodes segment and section layouts, symbols, relocations,
dynamic-loader metadata, exports, bindings, rebases, function starts,
data-in-code entries, code-signature containers, CodeDirectory metadata,
entitlement keys, chained-fixup headers, runtime-related sections, and build
information. Derived indexes support lookup by name and address without
changing the physical records produced by the parser.

## Build

A normal local build requires a C11 compiler and an archiver:

```sh
make
make test
```

For a warning-as-error build:

```sh
make strict
```

For AddressSanitizer and UndefinedBehaviorSanitizer tests:

```sh
make sanitize
```

The command-line utility is written to `tools/machlens`:

```sh
tools/machlens /path/to/image
tools/machlens --json /path/to/image
tools/machlens --commands /path/to/image
tools/machlens --security /path/to/image
tools/machlens --layout /path/to/image
```

## Public API

Applications create a reusable context, parse a byte buffer, inspect one or
more images, and release the resulting objects:

```c
ml_context *context = ml_context_create();
ml_file *file = ml_file_parse_memory(context, bytes, byte_count);

if (file != NULL) {
    const ml_image *image = ml_file_image_at(file, 0);
    char *report = ml_image_report_text(image);
    /* consume report */
    ml_string_free(report);
    ml_file_destroy(file);
}

ml_context_destroy(context);
```

The parser copies the supplied bytes. A caller may release or reuse its input
buffer after `ml_file_parse_memory` returns. Query structures returned by value
remain valid for the lifetime of their owning `ml_file`. Text-producing APIs
return heap strings that must be released with `ml_string_free`.

## Analysis surfaces

The public header provides four groups of operations:

1. Direct record access for images, load commands, segments, sections,
   symbols, exports, bindings, relocations, dependencies, notes, functions,
   signature blobs, diagnostics, and physical ranges.
2. Name and address queries for symbols, exports, sections, and mapped file
   positions.
3. Aggregate analysis for layout, symbol classes, linkedit activity, runtime
   metadata, strings, relocations, exports, dependencies, security posture,
   and cross-image comparison.
4. Text, CSV, JSON, and Graphviz reports suitable for command-line tooling,
   continuous integration, and offline inspection.

## Fuzzing

The repository contains four libFuzzer entry points:

- `parse_fuzzer` traverses decoded file, image, segment, section, and validation
  state.
- `linkedit_fuzzer` exercises symbols, relocations, export tries, binding and
  rebase streams, and structural fingerprints.
- `query_fuzzer` exercises public reports, mapping queries, and aggregate
  analyses after parsing.
- `consistency_fuzzer` cross-checks independent public query, summary, mapping,
  validation, and reporting surfaces after the common parsing pipeline.

ClusterFuzzLite configuration is under `.clusterfuzzlite/`. The root
`build.sh` and ClusterFuzzLite build script use the same source layout and
produce one executable per harness.

## Portability

The implementation does not require Apple frameworks or platform headers. All
format constants and byte-order readers are internal, so the project builds on
Linux with Clang or GCC. The library performs analysis only; it does not load,
link, execute, or modify the inspected images.

## License

MIT. See `LICENSE`.

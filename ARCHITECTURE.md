# libmachlens architecture

## Overview

`libmachlens` separates physical decoding from derived analysis. The parser
first records facts present in the byte stream. Later phases construct indexes,
resolve addresses, calculate summaries, and render reports. The separation
keeps byte-level decisions localized while allowing independent query surfaces
to consume a consistent image model.

A parsed `ml_file` owns a copy of the input and a collection of `ml_image`
objects. A thin Mach-O contains one image. A universal binary contains one
image for every accepted architecture record. Each image stores slice-relative
positions and refers to its owning file when it needs byte views.

## Processing phases

### 1. File classification

The file layer reads the leading magic value and selects thin-image or
universal-container processing. Universal headers are decoded with the byte
order indicated by their magic. Architecture records establish slice offsets,
sizes, CPU declarations, and iteration order.

### 2. Image header decoding

The image layer determines 32-bit or 64-bit layout and byte order, then reads
CPU type, subtype, file type, command count, command-byte count, flags, and the
reserved field where applicable. It verifies that the complete command region
lies inside the image slice.

### 3. Load-command inventory

The load-command pass walks the declared command region once. It records every
command's identifier, size, and file position before dispatching command-
specific decoders. Command handlers populate segment, section, dynamic-library,
build, entry-point, encryption, note, path, linker-option, and linkedit-location
state.

### 4. Physical layout decoding

Segment commands create segment records and their associated section records.
The model preserves virtual addresses, virtual sizes, file positions,
protections, section flags, alignments, and relocation locations. Physical
ranges are checked independently of virtual mappings.

### 5. Linkedit decoding

After all commands are known, linkedit decoders process structures whose
locations were declared by earlier commands. Separate modules decode symbol
and string tables, indirect-symbol tables, section relocations, export tries,
bind streams, weak-bind streams, lazy-bind streams, rebase streams, function
starts, data-in-code entries, code-signature superblobs, CodeDirectory fields,
entitlement keys, and chained-fixup headers.

Export-trie traversal uses an explicit work collection. Binding and rebase
streams are interpreted as state machines that emit normalized records. LEB128
reading is shared by the variable-length linkedit formats.

### 6. Derived physical ranges

The range builder combines headers, load commands, segment file regions,
section data, relocation regions, symbol and string tables, dynamic-loader
streams, signature data, and other command-declared blocks into a named range
collection. Range reports and overlap diagnostics consume this collection
without re-reading load commands.

### 7. Address resolution

Address helpers map virtual addresses to file positions and file positions to
virtual addresses using segment mappings. Binding, rebase, function-start, and
relocation records receive resolved addresses where their source format uses a
segment-relative or image-relative representation.

### 8. Lookup indexes

Indexes for symbol names, export names, and qualified section names contain
record indices rather than duplicate records. Sorting these index arrays leaves
parse order unchanged. Public find operations perform indexed lookup and copy
record fields into API structures.

### 9. Validation

Validation evaluates relationships among headers, commands, layout records,
linkedit declarations, decoded records, and address mappings. Findings are
stored as diagnostics with status, file position, and message. Aggregate
validation summaries report errors, warnings, overlaps, unresolved symbols,
and malformed linkedit records.

### 10. Aggregate analysis

Analysis modules calculate layout coverage, segment and section metrics,
symbol classes, dependency classes, language-runtime sections, export
properties, relocation properties, opcode statistics, string characteristics,
unwind summaries, security posture, entry-point mappings, command categories,
and data-quality indicators.

These modules consume the normalized image model. They do not alter parse
records or indexes.

### 11. Reporting

Reporting modules format the normalized and aggregate state as human-readable
text, CSV, JSON, and Graphviz. Reports have independent builders so callers can
request only the representation they need. The string builder owns intermediate
storage and transfers the final allocation to the caller.

### 12. Public lifetime

The context stores the most recent parse status and diagnostic text. The file
owns copied bytes and all image objects. Each image owns decoded strings,
arrays, indexes, ranges, and diagnostics. `ml_file_destroy` releases images
before releasing the byte copy. API text results are independent allocations
released with `ml_string_free`.

## Module groups

- `context.c`, `reader.c`, `range.c`, `leb128.c`, `vector.c`, and
  `string_builder.c` provide common infrastructure.
- `file.c`, `fat.c`, `header.c`, `loadcmd.c`, and command-specific files decode
  container and command metadata.
- `segment.c`, `symbols.c`, `relocations.c`, `export_trie.c`, `bind.c`,
  `rebase.c`, and the other linkedit modules populate normalized records.
- `ranges.c`, `address.c`, `index.c`, and `validate.c` create derived state.
- Files ending in `_analysis.c` calculate structured summaries.
- Files ending in `_report.c`, plus CSV, JSON, graph, and manifest modules,
  render output representations.
- `public.c`, `query.c`, `query_info.c`, `query_extended.c`, and `report.c`
  expose the library contract defined by `machlens.h`.

## Fuzzer surfaces

The four harnesses intentionally select different consumers after the common
parse pipeline. This provides coverage for structural traversal, linkedit state
machines, public query/report composition, and consistency across independent
summary and lookup surfaces. Each harness constructs a context, parses an owned
file object, reads results through public APIs, and performs complete cleanup.

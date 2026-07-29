#!/bin/bash -eu
cd "$SRC"

for src in src/*.c; do
    obj="${src%.c}.o"
    $CC $CFLAGS -Isrc -c -o "$obj" "$src"
done

ar rcs libmachlens.a src/*.o

for fuzzer_src in fuzz/*_fuzzer.cc; do
    fuzzer_name=$(basename "${fuzzer_src%.cc}")
    $CXX $CXXFLAGS -Isrc -o "$OUT/$fuzzer_name" \
        "$fuzzer_src" libmachlens.a \
        $LIB_FUZZING_ENGINE
done

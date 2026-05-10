#!/bin/bash

set -euo pipefail

ulimit -s unlimited
APP_DATA_DIR=$(dirname "$(readlink -f "$0")")
BUILD_DIR="$APP_DATA_DIR/build"

build() {
    local CONFIG="RelWithDebInfo"

    conan install . --build=missing --output-folder=$BUILD_DIR --settings=build_type=$CONFIG
    local CONAN_CMAKE_TOOLCHAIN=$BUILD_DIR/build/$CONFIG/generators/conan_toolchain.cmake

    pushd $BUILD_DIR > /dev/null
    cmake .. -GNinja -DCMAKE_TOOLCHAIN_FILE=$CONAN_CMAKE_TOOLCHAIN -DCMAKE_BUILD_TYPE=$CONFIG
    ninja
    popd > /dev/null
}

# runs unit test
run() {
    local BINARY_DIR="$BUILD_DIR/bin"
    pushd $BINARY_DIR > /dev/null

    for test in *; do
        if [[ $test == *"test_"* ]]; then
            echo "Running $test ..."
            ./$test
        fi
    done

    popd > /dev/null
}

# runs clang-format; prints each file that would be modified
format() {
    pushd "$APP_DATA_DIR" > /dev/null

    local modified_files=()

    while IFS= read -r -d '' file; do
        if ! clang-format --dry-run --Werror "$file" 2>/dev/null; then
            modified_files+=("$file")
        fi
    done < <(find include tests -name "*.hpp" -o -name "*.cpp" | sort | tr '\n' '\0')

    if [[ ${#modified_files[@]} -eq 0 ]]; then
        echo "clang-format: all files are already formatted."
    else
        echo "clang-format: the following files would be modified:"
        for file in "${modified_files[@]}"; do
            echo "  $file"
            clang-format -i "$file"
        done
    fi

    popd > /dev/null
}

# parse --format flag
RUN_FORMAT=false
PASSTHROUGH_ARGS=()
for arg in "$@"; do
    if [[ "$arg" == "--format" ]]; then
        RUN_FORMAT=true
    else
        PASSTHROUGH_ARGS+=("$arg")
    fi
done

if $RUN_FORMAT; then
    format
fi

build "${PASSTHROUGH_ARGS[@]+"${PASSTHROUGH_ARGS[@]}"}"
run

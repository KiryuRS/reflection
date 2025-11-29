#!/bin/bash

ulimit -s unlimited

main() {
    local APP_DATA_DIR=$(dirname "$(readlink -f "$0")")
    local BUILD_DIR="$APP_DATA_DIR/build"
    local CONFIG="RelWithDebInfo"

    conan install . --build=missing --output-folder=$BUILD_DIR --settings=build_type=$CONFIG
    local CONAN_CMAKE_TOOLCHAIN=$BUILD_DIR/build/$CONFIG/generators/conan_toolchain.cmake

    pushd $BUILD_DIR > /dev/null
    cmake .. -GNinja -DCMAKE_TOOLCHAIN_FILE=$CONAN_CMAKE_TOOLCHAIN -DCMAKE_BUILD_TYPE=$CONFIG
    ninja
    popd > /dev/null
}

main $@

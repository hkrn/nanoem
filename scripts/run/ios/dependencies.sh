#!/bin/bash

export NANOEM_TARGET_SYSTEM_NAME="ios"
export NANOEM_TARGET_ARCHITECTURES="arm64"
export NANOEM_TARGET_COMPILER="clang"
export NANOEM_TARGET_GENERATOR="Xcode"
export NANOEM_DISABLE_BUILD_GLFW="1"
export NANOEM_DISABLE_BUILD_YAMLCPP="1"

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && pwd)"

cmake \
  -DCMAKE_TOOLCHAIN_FILE="$DIR/ios-cmake/ios.toolchain.cmake" \
  -DPLATFORM=COMBINED64 \
  -P scripts/build.cmake

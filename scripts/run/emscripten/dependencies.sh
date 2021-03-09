#!/bin/bash

export NANOEM_TARGET_SYSTEM_NAME="emscripten"
export NANOEM_TARGET_ARCHITECTURES="wasm32"
export NANOEM_TARGET_COMPILER="clang"
export NANOEM_TARGET_GENERATOR="Unix Makefiles"
export NANOEM_DISABLE_BUILD_TBB="1"
export NANOEM_DISABLE_BUILD_GLFW="1"
export NANOEM_DISABLE_BUILD_NANOMSG="1"
export NANOEM_DISABLE_BUILD_SPIRV_TOOLS="1"
export NANOEM_DISABLE_BUILD_YAMLCPP="1"

source $EMSDK_PATH/emsdk_env.sh

cmake \
  -DCMAKE_TOOLCHAIN_FILE="$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
  -P scripts/build.cmake

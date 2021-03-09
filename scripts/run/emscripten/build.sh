#!/bin/bash -eux

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && pwd)"
nanoem_path=${nanoem_path:-$HOME/src/nanoem/development}

source $EMSDK_PATH/emsdk_env.sh

export NANOEM_TOOLCHAIN_BIN2C_DIR="$HOME/Library/Developer/build/nanoem/development/core/minsizerel/emapp/resources/helpers"
export NANOEM_TOOLCHAIN_YAML2PB_DIR="$HOME/Library/Developer/build/nanoem/development/core/minsizerel/emapp/resources/helpers"

cmake -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE="$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DNANOEM_ENABLE_BULLET=ON \
  -DNANOEM_ENABLE_ICU=OFF \
  -DNANOEM_ENABLE_NANOMSG=OFF \
  -DNANOEM_ENABLE_STATIC_BUNDLE_PLUGIN=ON \
  -DNANOEM_INSTALL_AVFOUNDATION_PLUGIN=OFF \
  -DNANOEM_INSTALL_EFFECT_PLUGIN=OFF \
  -DNANOEM_INSTALL_FFMPEG_PLUGIN=OFF \
  -DNANOEM_INSTALL_LSMASH_PLUGIN=ON \
  -DNANOEM_INSTALL_GIF_PLUGIN=OFF \
  -DNANOEM_INSTALL_ZIP_PLUGIN=OFF \
  -DNANOEM_TARGET_SYSTEM_NAME="wasm" \
  -DNANOEM_TARGET_ARCHITECTURES="wasm32" \
  -DNANOEM_TARGET_COMPILER="clang" \
  $nanoem_path
